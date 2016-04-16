#include "bm019.h"

volatile uint8_t rxBuffer[BM019_MAX_RX];

uint8_t BM019_CMD_IDN[] = {2,0x01,0x00};

uint8_t BM019_CMD_ECHO[] = {1,0x55};

uint8_t BM019_CMD_PROTOCOL_START[] = {2,0x02,0x02};
uint8_t BM019_CMD_PROTOCOL_ISO_IEC_15693[] = {2,0x01,0x00};
uint8_t BM019_CMD_PROTOCOL_OFF[] = {2,0x00,0x00};

uint8_t BM019_CMD_HYBERNATE[] = {15,
    0x07, //idle
    0x0E, //14 bytes
    0x08, //WU source low pulse irq
    0x04, 0x00, //Enter Control hybernate
    0x04, 0x00, //WU Control hybernate
    0x18, 0x00, //Leave Control hybernate
    0x00, //WU Period 0
    0x00, //Osc Start
    0x00, //DAC Start
    0x00, 0x00, //DAC Data
    0x00, //Swing Count
    0x00  //Max Sleep
};

uint8_t BM019_CMD_ISO_IEC_15693_INVENTORY[] = {5, 0x04, //send recieve
    0x03,//length
    0x26, //options:
    /*
     0 -> bit 0: Subcarrier 0 = ask, 1 =fsk
     1 -> bit 1: uplink data rate 0 = low, 1 = high
     1 -> bit 2: inventory flags 0 -> a), 1 -> b)
     0 -> bit 3: proto extension = 0
     a)
     bit 4: select flag 0 = if(bit 5 = 1 address mode)
     bit 5: address flag 1 = use address
     bit 6: for write = 1, else 0
     bit 7: future usage, always 0
     b)
     0 -> bit 4: afi flag 0 = no afi, 1 = afi (Application family identification)
     1 -> bit 5: slot flag, 0 -> 16 slots, 1 -> 1 slot
     0 -> bit 6: for write = 1, else 0
     0 -> bit 7: future usage, always 0

     */
    0x01, // inventory command
    0x00 // do not know why i sent it, maybe useless?
};

uint8_t BM019_CMD_READ[] = {5,
    0x04, //send recieve
    0x03, //length
    0x02, //options
    0x20, //read
    0x00  //Address

};
uint8_t BM019_CMD_READ_MULTI[] = {6,
    0x04, //send recieve
    0x00, //length
    0x02, //options
    0x23, //read
    0x00, //address
    0x00  //count
};
enum BM019_PROTOCOL {
    BM019_PROTOCOL_Field_OFF = 0x00,
    BM019_PROTOCOL_ISO_IEC_15693 = 0x01,
    //other not yet supported!
    BM019_PROTOCOL_ISO_IEC_14443_Type_A = 0x02,// also NFC Forum Tag Type 1 (Topaz), NFC Forum Tag Type 2, NFC Forum Tag Type 4A
    BM019_PROTOCOL_ISO_IEC_14443_Type_B = 0x03,// also NFC Forum Tag Type 4B
    BM019_PROTOCOL_ISO_IEC_18092 = 0x04 // also NFC Forum Tag Type 3
};

SPI spi(BM019_MOSI, BM019_MISO, BM019_CLK);

DigitalOut ss(BM019_CS);
DigitalOut wake(BM019_IRQ);

BM019_STATE stateBM019 = BM019_STATE_UNKNOWN;

int write_read(uint8_t *tx_buf, int tx_len, int timeout = BM019_READY_TIMEOUT); //write, poll, read

/*bool stateAtLeast(BM019_STATE requiredState)
 {
 if(stateBM019 < requiredState)
 {
 DEBUG("required state %d, having state %d",requiredState,stateBM019);
 return false;
 }
 return true;
 }*/

bool resetBM019()
{
    if(stateBM019 < BM019_STATE_ANSWERING) {
        DEBUG("IDN failed, bm019 not answering\n");
        return false;
    }
    DEBUG("BM019: reset\n");
    ss=0;
    spi.write(0x01);
    ss=1;
    wait_ms(20);
    stateBM019 = BM019_STATE_UNKNOWN;
    return true;
}

bool echoBM019(int timeout, bool log)
{
    if(log) {
        DEBUG("BM019: echo\n");
    }
    int len = write_read(&BM019_CMD_ECHO[1],BM019_CMD_ECHO[0], timeout);
    if(len>=1 && rxBuffer[0] == 0x55) {
        stateBM019 = stateBM019 > BM019_STATE_ANSWERING ? stateBM019 : BM019_STATE_ANSWERING;
        return true;
    } else {
        if(log) {
            DEBUG("recievedlen: %d \n",len);

            for(int i = 0; i < len; i++)
                DEBUG("rx[%d]: %#x\n",i,rxBuffer[i]);
        }
    }
    stateBM019 = BM019_STATE_UNKNOWN;
    return false;
}

BM019_STATE getStateBM019()
{
    return stateBM019;
}

bool wakeBM019(int timeout)
{
    DEBUG("BM019: wake\n");
    ss=0;
    wake = 0;
    wait_ms(10);
    wake = 1;
    ss = 1;

    wait_ms(timeout);
    int t = 10;

    stateBM019 = BM019_STATE_UNKNOWN;
    while(!echoBM019(10,false) && t > 0) {
        wait_ms(10);
        t--;
    }
    if(t<0) {
        return false;
    }
    return true;
}



int waitReady(int timeoutMS = 0)
{
    int ready = 0;

    if(timeoutMS) {
        Timer t;
        t.start();
        ss=0;
        while(!ready) {
            wait_ms(1);
            ready = spi.write(0x03) & 0x08;
            if(t.read_ms()>timeoutMS) {
                break;
            }
        }
        ss = 1;
    } else {
        ss=0;
        while(!ready) {
            wait_ms(1);
            ready = spi.write(0x03);
            ready = ready & 0x08;
        }
        ss = 1;
    }
    return ready;
}

void write(uint8_t *tx_buf, int tx_len)
{
    ss=0;
    spi.write(0x00);
    for(int i = 0; i < tx_len; i++) {
        spi.write(tx_buf[i]);
    }
    ss=1;
}

int readBM019()
{
    ss=0;
    spi.write(0x02);
    rxBuffer[0] = spi.write(0x00);
    int len = 0;
    if(rxBuffer[0] != 0x55) {
        len = rxBuffer[1] = spi.write(0x00);
        for(int i = 0; i < len && i < BM019_MAX_RX; i++) {
            rxBuffer[i+2] = spi.write(0x00);
        }
        len += 2;
    } else {
        len = 1;
    }
    ss=1;
    return len;
}

int write_read(uint8_t *tx_buf, int tx_len, int timeout)
{
    write(tx_buf, tx_len);
    waitReady(timeout);
    return readBM019();
}

bool setProtocolISO_EIC_15693BM019(BM019_PROTOCOL_ISO_IEC_15693_BYTE_0 configuration)
{
    if(stateBM019 < BM019_STATE_ANSWERING) {
        DEBUG("SETTING Protocol failed, bm019 not answering\n");
        return false;
    }
    DEBUG("SETTING Protocol to iso/iec 15693: %#x\n",configuration);

    int len = BM019_CMD_PROTOCOL_START[0]+BM019_CMD_PROTOCOL_ISO_IEC_15693[0];

    uint8_t iso[BM019_CMD_PROTOCOL_START[0]+BM019_CMD_PROTOCOL_ISO_IEC_15693[0]];
    memcpy(iso,&BM019_CMD_PROTOCOL_START[1],BM019_CMD_PROTOCOL_START[0]);
    memcpy(&iso[BM019_CMD_PROTOCOL_START[0]],&BM019_CMD_PROTOCOL_ISO_IEC_15693[1],BM019_CMD_PROTOCOL_ISO_IEC_15693[0]);
    iso[len-1] = configuration;

    int recieved = write_read(iso,len,20);
    if(recieved  >= 2 && rxBuffer[0] == 0x00 && rxBuffer[1] == 0x00) {
        stateBM019 = stateBM019 > BM019_STATE_PROTOCOL ? stateBM019 : BM019_STATE_PROTOCOL;
        return true;
    } else {
        DEBUG("SETTING Protocol failed: %#x\n",rxBuffer[0]);
        stateBM019 = BM019_STATE_UNKNOWN;
        return false;
    }
}

bool setProtocolOFF()
{
    if(stateBM019 < BM019_STATE_ANSWERING) {
        DEBUG("SETTING Protocol failed, bm019 not answering\n");
        return false;
    }
    DEBUG("SETTING Protocol to OFF\n");

    int len = BM019_CMD_PROTOCOL_START[0]+BM019_CMD_PROTOCOL_OFF[0];
    uint8_t off[BM019_CMD_PROTOCOL_START[0]+BM019_CMD_PROTOCOL_OFF[0]];
    memcpy(off,&BM019_CMD_PROTOCOL_START[1],BM019_CMD_PROTOCOL_START[0]);
    memcpy(&off[BM019_CMD_PROTOCOL_START[0]],&BM019_CMD_PROTOCOL_OFF[1],BM019_CMD_PROTOCOL_OFF[0]);

    int recieved = write_read(off,len,20);
    if(recieved  >= 2 && rxBuffer[0] == 0x00 && rxBuffer[1] == 0x00) {
        stateBM019 = BM019_STATE_ANSWERING;
        return true;
    } else {
        DEBUG("SETTING Protocol failed: %#x\n",rxBuffer[0]);
        stateBM019 = BM019_STATE_UNKNOWN;
        return false;
    }
}

bool idnBM019(BM019_IDN *idn)
{
    if(stateBM019 < BM019_STATE_ANSWERING) {
        DEBUG("IDN failed, bm019 not answering\n");
        return false;
    }
    int len = write_read(&BM019_CMD_IDN[1],BM019_CMD_IDN[0]);
    if(len == 17) {
        memcpy(idn->deviceID,(const void *)&rxBuffer[2],13);
        memcpy(idn->romCRC,(const void *)&rxBuffer[15],2);
        return true;
    }
    return false;

}

bool hybernateBM019()
{
    if(stateBM019 < BM019_STATE_ANSWERING) {
        DEBUG("SETTING HYBERNATE failed, bm019 not answering\n");
        return false;
    }

    DEBUG("HYBERNATE bm019 (FIELD_OFF and POWER_DOWN)\n");
    if(setProtocolOFF()) {
        write(&BM019_CMD_HYBERNATE[1],BM019_CMD_HYBERNATE[0]);
        stateBM019 = BM019_STATE_UNKNOWN;
        return true;
    }
    return false;
}


bool inventoryISO_IES_15693BM019(BM019_TAG *tag, int timeout)
{
    if(stateBM019 < BM019_STATE_PROTOCOL) {
        DEBUG("inventory failed, bm019 not in protocol\n");
        return false;
    }

    DEBUG("inventory..");

    int len = write_read(&BM019_CMD_ISO_IEC_15693_INVENTORY[1],BM019_CMD_ISO_IEC_15693_INVENTORY[0]);

    DEBUG("got answer len()=%d\n",len);
    for(int i = 0; i < len; i++) {
        DEBUG("%#04x ",rxBuffer[i]);
    }
    DEBUG("\n");

    if(rxBuffer[0] != EFrameRecvOK) {
        DEBUG("got error %#04x\n",rxBuffer[0]);
        return false;
    }

    int tlen = rxBuffer[1];
    if(tlen < 11) {
        DEBUG("to few bytes recieved \n");
        return false;
    }
    /* this does not work very good, maybe something misinterpreted from docs
     if(rxBuffer[tlen-1] & 0x01) {
     DEBUG("got collision \n");
     return false;
     }
     if(rxBuffer[tlen-1] & 0x02) {
     DEBUG("got bad crc \n");
     return false;
     }*/
    tag->crc[0] = rxBuffer[tlen-2];
    tag->crc[1] = rxBuffer[tlen-3];

    for(int i = 0; i < 9; i++) {
        tag->uid[i] = rxBuffer[11-i];
    }
    return true;

}

int readBM019(uint8_t adr, uint8_t *buf, int len, int timeout)
{
    if(stateBM019 < BM019_STATE_PROTOCOL) {
        DEBUG("read failed, bm019 not in protocol\n");
        return -1;
    }
    uint8_t cmd[BM019_CMD_READ[0]];
    memcpy(cmd,&BM019_CMD_READ[1],BM019_CMD_READ[0]);
    cmd[BM019_CMD_READ[0]-1] = adr & 0xFF;

    DEBUG("read at %#4X..\n",adr);
    for(int i = 0; i < BM019_CMD_READ[0]; i++) {
        DEBUG("%#04x ",cmd[i]);
    }
    int tx = write_read(cmd,BM019_CMD_READ[0]);

    DEBUG("got answer len()=%d\n",tx);
    for(int i = 0; i < tx; i++) {
        DEBUG("%#04x ",rxBuffer[i]);
    }
    DEBUG("\n");
    if(rxBuffer[0] != EFrameRecvOK) {
        DEBUG("got error %#04x\n",rxBuffer[0]);
        return -1;
    }

    DEBUG("flags: %#04x\n",rxBuffer[2]);
    int tlen = rxBuffer[1]-4;
    if(tlen <=0)
        return -1;
    DEBUG("read resultet in %d bytes, copying %d bytes\n",rxBuffer[1],(tlen < len ? tlen : len));
    tlen = (tlen < len ? tlen : len);
    memcpy(buf,(const void *)&rxBuffer[3],tlen);

    return tlen;
}

int readMultiBM019(uint8_t adr, int count, uint8_t *buf, int len, int timeout)
{
    if(stateBM019 < BM019_STATE_PROTOCOL) {
        DEBUG("multi read failed, bm019 not in protocol\n");
        return -1;
    }
    uint8_t cmd[BM019_CMD_READ_MULTI[0]];
    memcpy(cmd,&BM019_CMD_READ_MULTI[1],BM019_CMD_READ_MULTI[0]);
    cmd[BM019_CMD_READ_MULTI[0]-2] = adr & 0xFF;
    cmd[BM019_CMD_READ_MULTI[0]-1] = (count-1) & 0xFF;

    DEBUG("multi read at %#4X for %d..\n",adr, count & 0xFF);
    for(int i = 0; i < BM019_CMD_READ_MULTI[0]; i++) {
        DEBUG("%#04x ",cmd[i]);
    }
    int tx = write_read(cmd,BM019_CMD_READ_MULTI[0]);

    DEBUG("got answer len()=%d\n",tx);
    for(int i = 0; i < tx; i++) {
        DEBUG("%02x ",rxBuffer[i]);
    }
    DEBUG("\n");
    if(rxBuffer[0] != EFrameRecvOK) {
        DEBUG("got error %#04x\n",rxBuffer[0]);
        return -1;
    }

    DEBUG("flags: %#04x\n",rxBuffer[2]);
    int tlen = rxBuffer[1]-4;
    if(tlen <=0)
        return -1;
    DEBUG("read resultet in %d bytes, copying %d bytes\n",rxBuffer[1],(tlen < len ? tlen : len));
    tlen = (tlen < len ? tlen : len);
    memcpy(buf,(const void *)&rxBuffer[3],tlen);

    return tlen;
}
bool initBM019()
{
    DEBUG("BM019: init\n");
    spi.format(8,3);
    spi.frequency(1000000);
    stateBM019 = BM019_STATE_UNKNOWN;
    return true;
}

