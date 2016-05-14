
#ifndef BM019_H
#define BM019_H
    
#ifndef ARDUINO
    #include "mbed.h"
    //TODO #define SPICLass SPI
#else
    #include <arduino.h>
    //TODO #include <SPI.h>
    //TODO #define DigitalOut int
    #define wait_ms delayMicroseconds
#endif

#define BM019_MAX_RX 512
#define BM019_READY_TIMEOUT 50

enum BM019_PROTOCOL_ISO_IEC_15693_BYTE_0 {
    BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_0_NO_CRC = 0x00,
    BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_0_CRC = 0x01,

    BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_1_SINGLE_SUBCARRIER = 0x00,
    BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_1_DUAL_SUBCARRIER = 0x02,

    BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_2_100_MODULATION = 0x00,
    BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_2_10_MODULATION = 0x04,

    BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_3_RESPECT_312U_DELAY = 0x00,
    BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_3_WAIT_FOR_SOF = 0x08,

    BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_45_26_KBPS = 0x00,
    BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_45_52_KBPS = 0x10,
    BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_45_6_KBPS = 0x20
};

enum BM019_CODES {
    EFrameRecvOK    = 0x80, // Frame correctly received (additionally see CRC/Parity information)
    EUserStop       = 0x85, // Stopped by user (used only in Card mode)
    ECommError      = 0x86, // Hardware communication error
    EFrameWaitTOut  = 0x87, // Frame wait time out (no valid reception)
    EInvalidSof     = 0x88, // Invalid SOF
    EBufOverflow    = 0x89, // Too many bytes received and data still arriving
    EFramingError   = 0x8A, // if start bit = 1 or stop bit = 0
    EEgtError       = 0x8B, // EGT time out
    EInvalidLen     = 0x8C, // Valid for ISO/IEC 18092, if Length <3
    ECrcError       = 0x8D, // CRC error, Valid only for ISO/IEC 18092
    ERecvLost       = 0x8E, // When reception is lost without EOF received (or subcarrier was lost)
    ENoField        = 0x8F, // When Listen command detects the absence of external field
    EUnintByte      = 0x90, //Residual bits in last byte. Useful for ACK/NAK reception of ISO/IEC 14443 Type A.
};

enum BM019_STATE {
    BM019_STATE_UNKNOWN   = 0, //intial condition
    BM019_STATE_ANSWERING = 1, //if any communiaction has been successful
    BM019_STATE_PROTOCOL  = 2, //a protocol (other then off) has been set

};

struct BM019_IDN
{
    char deviceID[13];
    char romCRC[2];
};

struct BM019_TAG {
    uint8_t crc[2];
    uint8_t uid[8];
};

class BM019
{
    public:
    BM019_STATE getState();

    virtual bool reset(){return false;};
    virtual bool wake(int timeout = BM019_READY_TIMEOUT){return false;};
    bool hybernate();

    bool idn(BM019_IDN *idn);

    bool setProtocolOFF();
    bool setProtocolISO_EIC_15693(BM019_PROTOCOL_ISO_IEC_15693_BYTE_0 configuration);

    bool echo(int timeout = BM019_READY_TIMEOUT, bool log = true);

    bool inventoryISO_IES_15693(BM019_TAG *tag, int timeout = BM019_READY_TIMEOUT);

    int readSingle(uint8_t adr, uint8_t *buf, int len, int timeout = BM019_READY_TIMEOUT);
    int readMulti(uint8_t adr, int count, uint8_t *buf, int len, int timeout = BM019_READY_TIMEOUT);
    
    protected:
    virtual int read(){return -1;};
    virtual int write(uint8_t *tx_buf, int tx_len, int timeout = BM019_READY_TIMEOUT){return -1;};
    virtual int write_read(uint8_t *tx_buf, int tx_len, int timeout = BM019_READY_TIMEOUT){return -1;};

    bool init();

    BM019_STATE _state;
    volatile uint8_t _rxBuffer[BM019_MAX_RX];
    
    private:
};

class BM019_UART : public BM019
{
    public:
    /*
    mbed: Serial serial(BM019_TX, BM019_RX);
    */
#ifdef ARDUINO
    BM019_UART(HardwareSerial *serial);
#else
    BM019_UART(Serial *serial);
#endif
    virtual bool reset();
    virtual bool wake(int timeout = BM019_READY_TIMEOUT);
    
    private:
    virtual int write_read(uint8_t *tx_buf, int tx_len, int timeout);
    virtual int read();
    virtual int write(uint8_t *tx_buf, int tx_len, int timeout = BM019_READY_TIMEOUT);
    
#ifdef ARDUINO
    HardwareSerial *_serial;
#else
    Serial *_serial;
#endif  
};

/* TODO 
class BM019_SPI : public BM019
{
    public:
    
    //mbed: SPI spi(BM019_MOSI, BM019_MISO, BM019_CLK);
    

    BM019_SPI(int csPin, int irqPin, SPIClass spi);
    bool reset();
    bool wake(int timeout = BM019_READY_TIMEOUT);
    
    private:
    int write_read(uint8_t *tx_buf, int tx_len, int timeout);
    int read();
    int write(uint8_t *tx_buf, int tx_len, int timeout = BM019_READY_TIMEOUT);
    
    
    SPIClass _spi;
    DigitalOut _ss;
    DigitalOut _wake;
};*/
#endif /* BM019_H   */