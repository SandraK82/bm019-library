#include "bm019.h"
#include "log.h"

/*
cmd defines
*/
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
    0x01, // invenoty command
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
    //those not yet supported!
    //TODO implement other protocols -> get more tags
    BM019_PROTOCOL_ISO_IEC_14443_Type_A = 0x02,// also NFC Forum Tag Type 1 (Topaz), NFC Forum Tag Type 2, NFC Forum Tag Type 4A
    BM019_PROTOCOL_ISO_IEC_14443_Type_B = 0x03,// also NFC Forum Tag Type 4B
    BM019_PROTOCOL_ISO_IEC_18092 = 0x04 // also NFC Forum Tag Type 3
};

BM019_STATE BM019::getState()
{
    return _state;
}

bool BM019::init()
{
    _state = BM019_STATE_UNKNOWN;
    return true;
}
bool BM019::hybernate()
{
    if(_state < BM019_STATE_ANSWERING) {
        DEBUG("SETTING HYBERNATE failed, bm019 not answering\n");
        return false;
    }

    DEBUG("HYBERNATE bm019 (FIELD_OFF and POWER_DOWN)\n");
    if(setProtocolOFF()) {
        write(&BM019_CMD_HYBERNATE[1],BM019_CMD_HYBERNATE[0]);
        _state = BM019_STATE_UNKNOWN;
        return true;
    }
    return false;
}
bool BM019::idn(BM019_IDN *idn)
{
    if(_state < BM019_STATE_ANSWERING) {
        DEBUG("IDN failed, bm019 not answering\n");
        return false;
    }
    int len = write_read(&BM019_CMD_IDN[1],BM019_CMD_IDN[0]);
    if(len == 17) {
        memcpy(idn->deviceID,(const void *)&_rxBuffer[2],13);
        memcpy(idn->romCRC,(const void *)&_rxBuffer[15],2);
        return true;
    }
    return false;
}
bool BM019::setProtocolOFF()
{
    if(_state < BM019_STATE_ANSWERING) {
        DEBUG("SETTING Protocol failed, bm019 not answering\n");
        return false;
    }
    DEBUG("SETTING Protocol to OFF\n");

    int len = BM019_CMD_PROTOCOL_START[0]+BM019_CMD_PROTOCOL_OFF[0];
    uint8_t off[BM019_CMD_PROTOCOL_START[0]+BM019_CMD_PROTOCOL_OFF[0]];
    memcpy(off,&BM019_CMD_PROTOCOL_START[1],BM019_CMD_PROTOCOL_START[0]);
    memcpy(&off[BM019_CMD_PROTOCOL_START[0]],&BM019_CMD_PROTOCOL_OFF[1],BM019_CMD_PROTOCOL_OFF[0]);

    int recieved = write_read(off,len,20);
    if(recieved  >= 2 && _rxBuffer[0] == 0x00 && _rxBuffer[1] == 0x00) {
        _state = BM019_STATE_ANSWERING;
        return true;
    } else {
        DEBUG("SETTING Protocol failed: %#x\n",_rxBuffer[0]);
        _state = BM019_STATE_UNKNOWN;
        return false;
    }
}
bool BM019::setProtocolISO_EIC_15693(BM019_PROTOCOL_ISO_IEC_15693_BYTE_0 configuration)
{
    if(_state < BM019_STATE_ANSWERING) {
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
    if(recieved  >= 2 && _rxBuffer[0] == 0x00 && _rxBuffer[1] == 0x00) {
        _state = _state > BM019_STATE_PROTOCOL ? _state : BM019_STATE_PROTOCOL;
        return true;
    } else {
        DEBUG("SETTING Protocol failed: %#x\n",_rxBuffer[0]);
        _state = BM019_STATE_UNKNOWN;
        return false;
    }
}
bool BM019::echo(int timeout, bool log)
{
    if(log) {
        DEBUG("BM019: echo\n");
    }
    int len = write_read(&BM019_CMD_ECHO[1],BM019_CMD_ECHO[0], timeout);
    if(len>=1 && _rxBuffer[0] == 0x55) {
        _state = _state > BM019_STATE_ANSWERING ? _state : BM019_STATE_ANSWERING;
        return true;
    } else {
        if(log) {
            DEBUG("recievedlen: %d \n",len);

            for(int i = 0; i < len; i++)
                DEBUG("rx[%d]: %#x\n",i,rxBuffer[i]);
        }
    }
    _state = BM019_STATE_UNKNOWN;
    return false;
}
bool BM019::inventoryISO_IES_15693(BM019_TAG *tag, int timeout)
{
    if(_state < BM019_STATE_PROTOCOL) {
        DEBUG("inventory failed, bm019 not in protocol\n");
        return false;
    }

    DEBUG("inventory..");

    int len = write_read(&BM019_CMD_ISO_IEC_15693_INVENTORY[1],BM019_CMD_ISO_IEC_15693_INVENTORY[0]);

    DEBUG("got answer len()=%d\n",len);
    for(int i = 0; i < len; i++) {
        DEBUG("%#04x ",_rxBuffer[i]);
    }
    DEBUG("\n");

    if(_rxBuffer[0] != EFrameRecvOK) {
        DEBUG("got error %#04x\n",_rxBuffer[0]);
        return false;
    }

    int tlen = _rxBuffer[1];
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
    tag->crc[0] = _rxBuffer[tlen-2];
    tag->crc[1] = _rxBuffer[tlen-3];

    for(int i = 0; i < 9; i++) {
        tag->uid[i] = _rxBuffer[11-i];
    }
    return true;
}

int BM019::readSingle(uint8_t adr, uint8_t *buf, int len, int timeout)
{
    if(_state < BM019_STATE_PROTOCOL) {
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
        DEBUG("%#04x ",_rxBuffer[i]);
    }
    DEBUG("\n");
    if(_rxBuffer[0] != EFrameRecvOK) {
        DEBUG("got error %#04x\n",_rxBuffer[0]);
        return -1;
    }

    DEBUG("flags: %#04x\n",_rxBuffer[2]);
    int tlen = _rxBuffer[1]-4;
    if(tlen <=0)
        return -1;
    DEBUG("read resultet in %d bytes, copying %d bytes\n",rxBuffer[1],(tlen < len ? tlen : len));
    tlen = (tlen < len ? tlen : len);
    memcpy(buf,(const void *)&_rxBuffer[3],tlen);

    return tlen;
}
int BM019::readMulti(uint8_t adr, int count, uint8_t *buf, int len, int timeout)
{
    if(_state < BM019_STATE_PROTOCOL) {
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
    if(_rxBuffer[0] != EFrameRecvOK) {
        DEBUG("got error %#04x\n",_rxBuffer[0]);
        return -1;
    }

    DEBUG("flags: %#04x\n",_rxBuffer[2]);
    int tlen = _rxBuffer[1]-4;
    if(tlen <=0)
        return -1;
    DEBUG("read resultet in %d bytes, copying %d bytes\n",_rxBuffer[1],(tlen < len ? tlen : len));
    tlen = (tlen < len ? tlen : len);
    memcpy(buf,(const void *)&_rxBuffer[3],tlen);

    return tlen;
}

#ifdef ARDUINO
BM019_UART::BM019_UART(HardwareSerial *serial)
{
    _serial = serial;
    
    DEBUG("BM019: init\n");
    _serial->begin(19200,SERIAL_8N2);
    _serial->write((uint8_t)0);
    _serial->end();
    _serial->begin(57600,SERIAL_8N2);
}
#else
BM019_UART::BM019_UART(Serial *serial)
{
    _serial = serial;
    _serial->format(8,SerialBase::None,2);
    _serial->baud(57600);
    _serial->send_break();
}
#endif
    
bool BM019_UART::reset()
{
    if(_state < BM019_STATE_ANSWERING) {
        DEBUG("reset failed, bm019 not answering\n");
        return false;
    }
    DEBUG("BM019: reset\n");
    _state = BM019_STATE_UNKNOWN;
    return true;
}
bool BM019_UART::wake(int timeout)
{
    DEBUG("BM019: wake\n");
#ifdef ARDUINO
    _serial->end();
    _serial->begin(19200,SERIAL_8N2);
    _serial->write((uint8_t)0);
    _serial->end();
    _serial->begin(57600,SERIAL_8N2);
#else
    _serial->send_break();
#endif
    wait_ms(timeout);
    int t = 10;
    _state = BM019_STATE_UNKNOWN;
    while(!this->echo(10,false) && t > 0) {
        wait_ms(10);
        t--;
    }
    if(t<0) {
        return false;
    }
    return true;
}

int BM019_UART::write_read(uint8_t *tx_buf, int tx_len, int timeout)
{
    if(timeout) {
#ifndef ARDUINO
        Timer t;
        t.start();
        while (_serial->readable()) {
            _serial->getc();
            if(t.read_ms()>timeout) {
                return 0;
            }
        }
#else
        long ms = millis();
        while(_serial->available())
        {
            _serial->read();
            if(ms +timeout < millis()) {
                return 0;
            }
        }
#endif            
        write(tx_buf, tx_len, timeout);
//TODO MAYBE need to reset timer?!?
#ifndef ARDUINO        
        while (!_serial->readable()) {
            wait_ms(1);            
            if(t.read_ms()>timeout) {
#else
        while (!_serial->available()) {
            wait_ms(1);            
            if(ms +timeout < millis()) {
#endif            
                return 0;
            }
        }
        return read();
    } else {
#ifndef ARDUINO        
        while (_serial->readable()) {
            _serial->getc();
#else
        while (_serial->available()) {
            _serial->read();
#endif
            
        }
        write(tx_buf, tx_len, timeout);
        #ifndef ARDUINO        
        while (!_serial->readable()) {
#else
        while (!_serial->available()) {
#endif            
            return 0;
        }
        return read();
    }
}   
int BM019_UART::read()
{
#ifndef ARDUINO
    _rxBuffer[0] = _serial->getc();
#else
    _serial->readBytes((char*)&_rxBuffer[0],1);
#endif
    int len = 0;
    if(_rxBuffer[0] != 0x55) {
#ifndef ARDUINO
        len = _rxBuffer[1] = _serial->getc();
#else
        _serial->readBytes((char*)&_rxBuffer[1],1);
        len = _rxBuffer[1];
#endif 
        
#ifndef ARDUINO
        for(int i = 0; i < len && i < BM019_MAX_RX; i++) {
            _rxBuffer[i+2] = _serial->getc();    
        }
#else
        _serial->readBytes((char*)&_rxBuffer[2],len);
#endif           
        len += 2;
    } else {
        len = 1;
    }
    return len;
}
int BM019_UART::write(uint8_t *tx_buf, int tx_len, int timeout)
{
    int wl = 0;
    if(timeout) {
#ifndef ARDUINO 
        Timer t;
        t.start();
#else
        unsigned long sm = millis();
#endif          
        for(int i = 0; i < tx_len; i++) {
#ifndef ARDUINO
            while (!_serial->writeable()) {
#else
            while (!_serial->availableForWrite()) {
#endif            
                wait_ms(1);
#ifndef ARDUINO                
                if(t.read_ms()>timeout) {
                    return wl;
                }
#else
                if(millis() > timeout +sm) {
                    return wl;
                }             
#endif              
            }
#ifndef ARDUINO
            _serial->putc(tx_buf[i]);
#else
           _serial->write(tx_buf[i]);
#endif  
            wl++;
        }
    } else {
        for(int i = 0; i < tx_len; i++) {
#ifndef ARDUINO
            while (!_serial->writeable()) {
#else
            while (!_serial->availableForWrite()) {
#endif            
                return wl;
            }
#ifndef ARDUINO
            _serial->putc(tx_buf[i]);
#else
           _serial->write(tx_buf[i]);
#endif  
            wl++;
        }
    }
    return wl;
}
    
/* TODO

BM019_SPI::BM019_SPI(int csPin, int irqPin,SPIClass spi)
{
}
bool BM019_SPI::reset()
{
}
bool BM019_SPI::wake(int timeout)
{
}
int BM019_SPI::read()
{
}
int BM019_SPI::write(uint8_t *tx_buf, int tx_len, int timeout)
{
}
*/