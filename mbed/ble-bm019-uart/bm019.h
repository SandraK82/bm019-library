
#ifndef BM019_H
#define BM019_H

#include "log.h"
#include "mbed.h"

#define BM019_TX p9
#define BM019_RX p11

#define BM019_MAX_RX 512
#define BM019_READY_TIMEOUT 50 /* 0 for endless waiting else  ms to wait */

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

extern BM019_STATE stateBM019;

struct BM019_IDN
{
    char deviceID[13];
    char romCRC[2];
};

struct BM019_TAG {
    uint8_t crc[2];
    uint8_t uid[8];
};

BM019_STATE getStateBM019();

bool initBM019();

bool resetBM019();
bool wakeBM019(int timeout = BM019_READY_TIMEOUT);
bool hybernateBM019();

bool idnBM019(BM019_IDN *idn);

bool setProtocolOFF();
bool setProtocolISO_EIC_15693BM019(BM019_PROTOCOL_ISO_IEC_15693_BYTE_0 configuration);

bool echoBM019(int timeout = BM019_READY_TIMEOUT, bool log = true);

bool inventoryISO_IES_15693BM019(BM019_TAG *tag, int timeout = BM019_READY_TIMEOUT);

int readBM019(uint8_t adr, uint8_t *buf, int len, int timeout = BM019_READY_TIMEOUT);
int readMultiBM019(uint8_t adr, int count, uint8_t *buf, int len, int timeout = BM019_READY_TIMEOUT);
#endif /* BM019_H   */