#pragma once
#include "transceiver.h"

// CC1101 SPI command strobes
#define CC1101_SRES         0x30  // Reset chip
#define CC1101_SFSTXON      0x31  // Enable and calibrate frequency synthesizer
#define CC1101_SXOFF        0x32  // Turn off crystal oscillator
#define CC1101_SCAL         0x33  // Calibrate frequency synthesizer
#define CC1101_SRX          0x34  // Enable RX
#define CC1101_STX          0x35  // Enable TX
#define CC1101_SIDLE        0x36  // Exit RX/TX, turn off frequency synthesizer
#define CC1101_SWOR         0x38  // Start automatic RX polling sequence
#define CC1101_SPWD         0x39  // Enter power down mode
#define CC1101_SFRX         0x3A  // Flush the RX FIFO
#define CC1101_SFTX         0x3B  // Flush the TX FIFO
#define CC1101_SWORRST      0x3C  // Reset real time clock to Event1 value
#define CC1101_SNOP         0x3D  // No operation

// CC1101 configuration registers
#define CC1101_IOCFG2       0x00
#define CC1101_IOCFG1       0x01
#define CC1101_IOCFG0       0x02
#define CC1101_FIFOTHR      0x03
#define CC1101_SYNC1        0x04
#define CC1101_SYNC0        0x05
#define CC1101_PKTLEN       0x06
#define CC1101_PKTCTRL1     0x07
#define CC1101_PKTCTRL0     0x08
#define CC1101_ADDR         0x09
#define CC1101_CHANNR       0x0A
#define CC1101_FSCTRL1      0x0B
#define CC1101_FSCTRL0      0x0C
#define CC1101_FREQ2        0x0D
#define CC1101_FREQ1        0x0E
#define CC1101_FREQ0        0x0F
#define CC1101_MDMCFG4      0x10
#define CC1101_MDMCFG3      0x11
#define CC1101_MDMCFG2      0x12
#define CC1101_MDMCFG1      0x13
#define CC1101_MDMCFG0      0x14
#define CC1101_DEVIATN      0x15
#define CC1101_MCSM2        0x16
#define CC1101_MCSM1        0x17
#define CC1101_MCSM0        0x18
#define CC1101_FOCCFG       0x19
#define CC1101_BSCFG        0x1A
#define CC1101_AGCCTRL2     0x1B
#define CC1101_AGCCTRL1     0x1C
#define CC1101_AGCCTRL0     0x1D
#define CC1101_WOREVT1      0x1E
#define CC1101_WOREVT0      0x1F
#define CC1101_WORCTRL      0x20
#define CC1101_FREND1       0x21
#define CC1101_FREND0       0x22
#define CC1101_FSCAL3       0x23
#define CC1101_FSCAL2       0x24
#define CC1101_FSCAL1       0x25
#define CC1101_FSCAL0       0x26
#define CC1101_RCCTRL1      0x27
#define CC1101_RCCTRL0      0x28
#define CC1101_FSTEST       0x29
#define CC1101_PTEST        0x2A
#define CC1101_AGCTEST      0x2B
#define CC1101_TEST2        0x2C
#define CC1101_TEST1        0x2D
#define CC1101_TEST0        0x2E

// CC1101 status registers (read with burst bit set)
#define CC1101_PARTNUM      0x30
#define CC1101_VERSION      0x31
#define CC1101_FREQEST      0x32
#define CC1101_LQI          0x33
#define CC1101_RSSI         0x34
#define CC1101_MARCSTATE    0x35
#define CC1101_WORTIME1     0x36
#define CC1101_WORTIME0     0x37
#define CC1101_PKTSTATUS    0x38
#define CC1101_VCO_VC_DAC   0x39
#define CC1101_TXBYTES      0x3A
#define CC1101_RXBYTES      0x3B
#define CC1101_RCCTRL1_STATUS 0x3C
#define CC1101_RCCTRL0_STATUS 0x3D

// CC1101 FIFO access
#define CC1101_TXFIFO       0x3F
#define CC1101_RXFIFO       0x3F

// CC1101 SPI access masks
#define CC1101_WRITE_SINGLE 0x00
#define CC1101_WRITE_BURST  0x40
#define CC1101_READ_SINGLE  0x80
#define CC1101_READ_BURST   0xC0

// MARCSTATE values
#define CC1101_MARCSTATE_SLEEP          0x00
#define CC1101_MARCSTATE_IDLE           0x01
#define CC1101_MARCSTATE_XOFF           0x02
#define CC1101_MARCSTATE_VCOON_MC       0x03
#define CC1101_MARCSTATE_REGON_MC       0x04
#define CC1101_MARCSTATE_MANCAL         0x05
#define CC1101_MARCSTATE_VCOON          0x06
#define CC1101_MARCSTATE_REGON          0x07
#define CC1101_MARCSTATE_STARTCAL       0x08
#define CC1101_MARCSTATE_BWBOOST        0x09
#define CC1101_MARCSTATE_FS_LOCK        0x0A
#define CC1101_MARCSTATE_IFADCON        0x0B
#define CC1101_MARCSTATE_ENDCAL         0x0C
#define CC1101_MARCSTATE_RX             0x0D
#define CC1101_MARCSTATE_RX_END         0x0E
#define CC1101_MARCSTATE_RX_RST         0x0F
#define CC1101_MARCSTATE_TXRX_SWITCH    0x10
#define CC1101_MARCSTATE_RXFIFO_OVERFLOW 0x11
#define CC1101_MARCSTATE_FSTXON         0x12
#define CC1101_MARCSTATE_TX             0x13
#define CC1101_MARCSTATE_TX_END         0x14
#define CC1101_MARCSTATE_RXTX_SWITCH    0x15
#define CC1101_MARCSTATE_TXFIFO_UNDERFLOW 0x16

// GDOx configuration values (for IOCFG0/1/2)
#define CC1101_GDO_RXFIFO_THR           0x00
#define CC1101_GDO_RXFIFO_FULL          0x01
#define CC1101_GDO_TXFIFO_THR           0x02
#define CC1101_GDO_TXFIFO_FULL          0x03
#define CC1101_GDO_RXFIFO_OVERFLOW      0x04
#define CC1101_GDO_TXFIFO_UNDERFLOW     0x05
#define CC1101_GDO_SYNC_WORD            0x06
#define CC1101_GDO_PKT_RECEIVED         0x07
#define CC1101_GDO_PREAMBLE_QUALITY     0x08
#define CC1101_GDO_CCA                  0x09
#define CC1101_GDO_PLL_LOCK             0x0A
#define CC1101_GDO_SERIAL_CLK           0x0B
#define CC1101_GDO_SERIAL_SYNC_DATA     0x0C
#define CC1101_GDO_SERIAL_ASYNC_DATA    0x0D
#define CC1101_GDO_CARRIER_SENSE        0x0E
#define CC1101_GDO_CRC_OK               0x0F
#define CC1101_GDO_HI_Z                 0x2E
#define CC1101_GDO_CLK_XOSC_1           0x30
#define CC1101_GDO_CLK_XOSC_1_5         0x31
#define CC1101_GDO_CLK_XOSC_2           0x32
#define CC1101_GDO_CLK_XOSC_3           0x33
#define CC1101_GDO_CLK_XOSC_4           0x34
#define CC1101_GDO_CLK_XOSC_6           0x35
#define CC1101_GDO_CLK_XOSC_8           0x36
#define CC1101_GDO_CLK_XOSC_12          0x37
#define CC1101_GDO_CLK_XOSC_16          0x38
#define CC1101_GDO_CLK_XOSC_24          0x39
#define CC1101_GDO_CLK_XOSC_32          0x3A
#define CC1101_GDO_CLK_XOSC_48          0x3B
#define CC1101_GDO_CLK_XOSC_64          0x3C
#define CC1101_GDO_CLK_XOSC_96          0x3D
#define CC1101_GDO_CLK_XOSC_128         0x3E
#define CC1101_GDO_CLK_XOSC_192         0x3F

// wM-Bus specific defines
#define WMBUS_SYNC_WORD_HIGH            0x54
#define WMBUS_SYNC_WORD_LOW             0x3D
#define WMBUS_PREAMBLE                  0x54

namespace esphome {
namespace wmbus_radio {

class CC1101 : public RadioTransceiver {
public:
  void setup() override;
  size_t get_frame(uint8_t *buffer, size_t length, uint32_t offset) override;
  bool read_in_task(uint8_t *buffer, size_t length, uint32_t offset) override;
  gpio::InterruptType get_interrupt_type() override { return gpio::INTERRUPT_FALLING_EDGE; }
  void restart_rx() override;
  int8_t get_rssi() override;
  const char *get_name() override;

protected:
  optional<uint8_t> read() override;

  // CC1101-specific SPI methods
  uint8_t strobe(uint8_t cmd);
  uint8_t read_register(uint8_t address);
  // SWRZ020-safe status register read (reads twice and accepts stable value).
  uint8_t read_status_register(uint8_t address);
  // Single-shot status register read, no errata workaround. Used internally.
  uint8_t read_status_register_raw_(uint8_t address);
  void write_register(uint8_t address, uint8_t value);
  void write_burst(uint8_t address, const uint8_t *data, size_t length);
  void read_burst(uint8_t address, uint8_t *data, size_t length);
  uint8_t get_rx_bytes();

  int8_t last_rssi_{0};
};

} // namespace wmbus_radio
} // namespace esphome
