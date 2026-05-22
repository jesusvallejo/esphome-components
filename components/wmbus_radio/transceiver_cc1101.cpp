#include "transceiver_cc1101.h"

#include "esphome/core/log.h"

namespace esphome {
namespace wmbus_radio {
static const char *TAG = "CC1101";

// CC1101 crystal frequency (standard 26 MHz)
#define F_XTAL 26000000

// Maximum number of attempts when applying the TI SWRZ020 status-register
// read-twice workaround. Two consecutive identical reads are accepted as
// stable; if the chip keeps changing the value we give up and return the
// last sample (still better than a single unverified read).
#define CC1101_STATUS_READ_MAX_TRIES 5

void CC1101::setup() {
  this->common_setup();

  ESP_LOGV(TAG, "Setup");

  // CC1101 has no hardware reset pin — skip hardware reset, use software reset only
  ESP_LOGVV(TAG, "software reset (SRES)");
  this->strobe(CC1101_SRES);
  delay(10);

  ESP_LOGVV(TAG, "checking part number");
  uint8_t partnum = this->read_status_register(CC1101_PARTNUM);
  uint8_t version = this->read_status_register(CC1101_VERSION);
  ESP_LOGVV(TAG, "part: %02X, version: %02X", partnum, version);
  if (partnum != 0x00) {
    ESP_LOGE(TAG, "Invalid part number: %02X (expected 0x00)", partnum);
    return;
  }

  // Configure for wM-Bus Mode C/T at 868.95 MHz, 100 kbps, 2-FSK
  // RF settings based on TI Design Note DN022 and wmbusmeters reference

  ESP_LOGVV(TAG, "configuring GDO pins");
  // GDO2: Sync word sent/received (active low, inverted)
  this->write_register(CC1101_IOCFG2, CC1101_GDO_SYNC_WORD | 0x40);
  // GDO1: High impedance (unused, default)
  this->write_register(CC1101_IOCFG1, CC1101_GDO_HI_Z);
  // GDO0: RXFIFO threshold reached (IRQ pin), active low (inverted)
  this->write_register(CC1101_IOCFG0, CC1101_GDO_RXFIFO_THR | 0x40);

  ESP_LOGVV(TAG, "configuring FIFO threshold");
  // FIFO threshold: 7 = RX FIFO >= 32 bytes, TX FIFO <= 31 bytes
  this->write_register(CC1101_FIFOTHR, 0x07);

  ESP_LOGVV(TAG, "configuring sync word");
  // Sync word for wM-Bus: 0x543D (with Manchester encoding: preamble 0x54)
  this->write_register(CC1101_SYNC1, WMBUS_SYNC_WORD_HIGH);
  this->write_register(CC1101_SYNC0, WMBUS_SYNC_WORD_LOW);

  ESP_LOGVV(TAG, "configuring packet control");
  // Packet length: 0xFF for infinite/variable
  this->write_register(CC1101_PKTLEN, 0xFF);
  // Packet control 1: No address check, no append status, no CRC autoflush
  this->write_register(CC1101_PKTCTRL1, 0x00);
  // Packet control 0: Infinite packet length, no whitening, no CRC.
  //
  // The previous value (0x00) selected FIXED packet length mode. With
  // PKTLEN=0xFF this told the chip to receive exactly 255 bytes after every
  // sync word detection, then go IDLE. A real wM-Bus T1/C1 telegram is
  // typically 30–80 bytes on air, so the chip would continue demodulating
  // ~200 bytes of noise into the 64-byte RX FIFO after the actual packet
  // ended — overrunning the FIFO and producing the "RX FIFO overflow" /
  // "RX timeout after 0 bytes (need 3)" pattern reported by users on
  // single-core ESP32-C3 / S3 / C6 targets sharing the core with WiFi.
  //
  // Infinite length mode (0x02) matches the receive loop in component.cpp,
  // which determines the real telegram length from the decoded L-field and
  // then issues SIDLE/SFRX/SRX via restart_rx() to stop the chip exactly
  // when expected_size bytes have been read. See SzczepanLeon PR #382 and
  // CC1101 datasheet §15.2.
  this->write_register(CC1101_PKTCTRL0, 0x02);

  // No device address filtering
  this->write_register(CC1101_ADDR, 0x00);
  // Channel number 0
  this->write_register(CC1101_CHANNR, 0x00);

  ESP_LOGVV(TAG, "configuring frequency synthesizer");
  // IF frequency: ~152 kHz (typical for 100 kbps)
  this->write_register(CC1101_FSCTRL1, 0x08);
  this->write_register(CC1101_FSCTRL0, 0x00);

  ESP_LOGVV(TAG, "setting radio frequency");
  // FREQ = (f_carrier * 2^16) / f_xtal
  uint32_t freq_reg = ((uint64_t)this->frequency_hz_ << 16) / F_XTAL;
  this->write_register(CC1101_FREQ2, BYTE(freq_reg, 2));
  this->write_register(CC1101_FREQ1, BYTE(freq_reg, 1));
  this->write_register(CC1101_FREQ0, BYTE(freq_reg, 0));

  ESP_LOGVV(TAG, "configuring modem");
  // Modem configuration for 100 kbps, 2-FSK, 50 kHz deviation
  this->write_register(CC1101_MDMCFG4, 0x5C);
  this->write_register(CC1101_MDMCFG3, 0x04);
  this->write_register(CC1101_MDMCFG2, 0x06);
  this->write_register(CC1101_MDMCFG1, 0x22);
  this->write_register(CC1101_MDMCFG0, 0xF8);

  ESP_LOGVV(TAG, "configuring deviation");
  // Deviation: ~50 kHz
  this->write_register(CC1101_DEVIATN, 0x44);

  ESP_LOGVV(TAG, "configuring state machine");
  this->write_register(CC1101_MCSM2, 0x07);
  this->write_register(CC1101_MCSM1, 0x00);
  this->write_register(CC1101_MCSM0, 0x18);

  ESP_LOGVV(TAG, "configuring AFC/AGC");
  this->write_register(CC1101_FOCCFG, 0x2E);
  this->write_register(CC1101_BSCFG, 0xBF);
  this->write_register(CC1101_AGCCTRL2, 0x43);
  this->write_register(CC1101_AGCCTRL1, 0x09);
  this->write_register(CC1101_AGCCTRL0, 0xB5);

  ESP_LOGVV(TAG, "configuring WOR");
  this->write_register(CC1101_WOREVT1, 0x87);
  this->write_register(CC1101_WOREVT0, 0x6B);
  this->write_register(CC1101_WORCTRL, 0xFB);

  ESP_LOGVV(TAG, "configuring front end");
  this->write_register(CC1101_FREND1, 0xB6);
  this->write_register(CC1101_FREND0, 0x10);

  ESP_LOGVV(TAG, "configuring frequency calibration");
  this->write_register(CC1101_FSCAL3, 0xEA);
  this->write_register(CC1101_FSCAL2, 0x2A);
  this->write_register(CC1101_FSCAL1, 0x00);
  this->write_register(CC1101_FSCAL0, 0x1F);

  this->write_register(CC1101_RCCTRL1, 0x41);
  this->write_register(CC1101_RCCTRL0, 0x00);

  ESP_LOGVV(TAG, "configuring test registers");
  this->write_register(CC1101_FSTEST, 0x59);
  this->write_register(CC1101_PTEST, 0x7F);
  this->write_register(CC1101_AGCTEST, 0x3F);
  this->write_register(CC1101_TEST2, 0x81);
  this->write_register(CC1101_TEST1, 0x35);
  this->write_register(CC1101_TEST0, 0x09);

  ESP_LOGVV(TAG, "calibrating");
  this->strobe(CC1101_SCAL);
  delay(1);

  ESP_LOGVV(TAG, "entering RX mode");
  this->strobe(CC1101_SFRX);
  this->strobe(CC1101_SRX);

  ESP_LOGV(TAG, "CC1101 setup done");
}

uint8_t CC1101::strobe(uint8_t cmd) {
  this->delegate_->begin_transaction();
  uint8_t status = this->delegate_->transfer(cmd);
  this->delegate_->end_transaction();
  return status;
}

uint8_t CC1101::read_register(uint8_t address) {
  this->delegate_->begin_transaction();
  this->delegate_->transfer(address | CC1101_READ_SINGLE);
  uint8_t value = this->delegate_->transfer(0x00);
  this->delegate_->end_transaction();
  return value;
}

// Single (raw) status-register read. CC1101 status registers must be
// addressed with the burst bit set (datasheet section 10.3).
uint8_t CC1101::read_status_register_raw_(uint8_t address) {
  this->delegate_->begin_transaction();
  this->delegate_->transfer(address | CC1101_READ_BURST);
  uint8_t value = this->delegate_->transfer(0x00);
  this->delegate_->end_transaction();
  return value;
}

// Status register read with the SWRZ020 errata workaround.
// "Radio status register read access" — the value of a status register may
// be wrong if it is read while the chip is updating it. The recommended
// workaround is to read the register twice and only accept the value when
// two consecutive reads return the same data. We retry up to
// CC1101_STATUS_READ_MAX_TRIES times before giving up.
uint8_t CC1101::read_status_register(uint8_t address) {
  uint8_t prev = this->read_status_register_raw_(address);
  for (uint8_t i = 0; i < CC1101_STATUS_READ_MAX_TRIES; i++) {
    uint8_t curr = this->read_status_register_raw_(address);
    if (curr == prev)
      return curr;
    prev = curr;
  }
  return prev;
}

void CC1101::write_register(uint8_t address, uint8_t value) {
  this->delegate_->begin_transaction();
  this->delegate_->transfer(address | CC1101_WRITE_SINGLE);
  this->delegate_->transfer(value);
  this->delegate_->end_transaction();
}

void CC1101::write_burst(uint8_t address, const uint8_t *data, size_t length) {
  this->delegate_->begin_transaction();
  this->delegate_->transfer(address | CC1101_WRITE_BURST);
  for (size_t i = 0; i < length; i++) {
    this->delegate_->transfer(data[i]);
  }
  this->delegate_->end_transaction();
}

void CC1101::read_burst(uint8_t address, uint8_t *data, size_t length) {
  this->delegate_->begin_transaction();
  this->delegate_->transfer(address | CC1101_READ_BURST);
  for (size_t i = 0; i < length; i++) {
    data[i] = this->delegate_->transfer(0x00);
  }
  this->delegate_->end_transaction();
}

uint8_t CC1101::get_rx_bytes() {
  return this->read_status_register(CC1101_RXBYTES) & 0x7F;
}

optional<uint8_t> CC1101::read() {
  // Active low IRQ = FIFO threshold reached
  if (this->irq_pin_->digital_read() == false) {
    uint8_t rx_bytes = this->get_rx_bytes();
    if (rx_bytes > 0) {
      this->last_rssi_ = (int8_t)this->read_status_register(CC1101_RSSI);
      uint8_t data;
      this->read_burst(CC1101_RXFIFO, &data, 1);
      return data;
    }
  }
  return {};
}

size_t CC1101::get_frame(uint8_t *buffer, size_t length, uint32_t offset) {
  auto byte = this->read();
  if (!byte.has_value())
    return 0;

  *buffer = *byte;
  return 1;
}

void CC1101::restart_rx() {
  ESP_LOGVV(TAG, "Restarting RX");
  this->strobe(CC1101_SIDLE);
  delay(1);
  this->strobe(CC1101_SFRX);
  this->strobe(CC1101_SRX);
  delay(1);
}

int8_t CC1101::get_rssi() {
  // Convert RSSI_dec to dBm: RSSI_dBm = (RSSI_dec / 2) - RSSI_offset
  // RSSI offset per TI DN505: 74 dBm @ 868 MHz, 76 dBm @ 433 MHz
  int8_t rssi_offset = (this->frequency_hz_ < 600000000u) ? 76 : 74;
  int8_t rssi_dec = this->last_rssi_;
  int16_t rssi_dbm;
  if (rssi_dec >= 128) {
    rssi_dbm = ((int16_t)(rssi_dec - 256) / 2) - rssi_offset;
  } else {
    rssi_dbm = (rssi_dec / 2) - rssi_offset;
  }
  return (int8_t)rssi_dbm;
}

const char *CC1101::get_name() { return TAG; }

bool CC1101::read_in_task(uint8_t *buffer, size_t length, uint32_t offset) {
  size_t total = 0;
  uint32_t last_progress = millis();

  while (total < length) {
    uint8_t rxbytes_raw = this->read_status_register(CC1101_RXBYTES);

    // FIFO overflow — bit 0x80 set
    if (rxbytes_raw & 0x80) {
      ESP_LOGW(TAG, "RX FIFO overflow");
      this->restart_rx();
      return false;
    }

    uint8_t available = rxbytes_raw & 0x7F;
    size_t remaining = length - total;

    if (available > 0) {
      // CC1101 RX FIFO errata: when more data is still expected, never empty
      // the FIFO completely — leave at least one byte behind.
      size_t to_read = (remaining > 1 && available > 1)
                           ? std::min((size_t)(available - 1), remaining)
                           : std::min((size_t)available, remaining);

      if (to_read > 0) {
        if (total == 0 && offset == 0) {
          this->last_rssi_ = (int8_t)this->read_status_register(CC1101_RSSI);
        }
        this->read_burst(CC1101_RXFIFO, buffer + total, to_read);
        total += to_read;
        last_progress = millis();
        continue;  // re-check the FIFO immediately
      }
    }

    // End-of-packet check via MARCSTATE (with errata workaround).
    if (total > 0 && total < length) {
      uint8_t marcstate = this->read_status_register(CC1101_MARCSTATE) & 0x1F;
      if (marcstate == CC1101_MARCSTATE_IDLE || marcstate == CC1101_MARCSTATE_RX_END) {
        uint8_t final_bytes = this->read_status_register(CC1101_RXBYTES) & 0x7F;
        if (final_bytes > 0 && total + final_bytes <= length) {
          this->read_burst(CC1101_RXFIFO, buffer + total, final_bytes);
          total += final_bytes;
        }
        break;
      }
    }

    // No progress for 500 ms — give up and let the receiver task re-arm.
    if (millis() - last_progress > 500) {
      ESP_LOGW(TAG, "RX timeout after %zu bytes (need %zu)", total + offset, length + offset);
      return false;
    }

    // Sleep ~1 ms instead of busy-polling at 200 µs. Drops SPI traffic ~5x
    // during quiet periods and yields the CPU to other FreeRTOS tasks.
    delay(1);
  }

  return (total == length);
}

} // namespace wmbus_radio
} // namespace esphome
