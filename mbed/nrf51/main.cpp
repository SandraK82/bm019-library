#include "mbed.h"
#include "ble/BLE.h"

#include "ble/services/UARTService.h"

#include "log.h"
/*
 This is an example of using the implemented functions of the bm019-library with an nrf51, acting as UART-Service.
 This code uses the spi version of the interface.
 */
#include "bm019.h"

BLEDevice  ble;
UARTService *uartServicePtr;

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    DEBUG("Disconnected!\n\r");
    DEBUG("Restarting the advertising process\n\r");
    ble.startAdvertising();
}
char answer[512];

void onDataWritten(const GattWriteCallbackParams *params)
{
    if ((uartServicePtr != NULL) && (params->handle == uartServicePtr->getTXCharacteristicHandle())) {
        uint16_t bytesRead = params->len;
        bool handled = false;


        for(int i = 0; !handled && i < bytesRead; i++) {
            switch(params->data[i]) {
                case '?': {
                    DEBUG("Handle ?\n");
                    switch(getStateBM019()) {
                        case BM019_STATE_UNKNOWN:
                            DEBUG("BM019 Status: unknown\n");
                            sprintf(answer,"+?:0!");
                            break;
                        case BM019_STATE_ANSWERING:
                            DEBUG("BM019 Status: answering\n");
                            sprintf(answer,"+?:1!");
                            break;
                        case BM019_STATE_PROTOCOL:
                            DEBUG("BM019 Status: protocol set\n");
                            sprintf(answer,"+?:2!");
                            break;
                        default:
                            sprintf(answer,"-?%d!",getStateBM019());
                            DEBUG("BM019 Status: forgotten state\n");
                    }
                    handled = true;
                }
                    break;

                case 'h':
                case 'H': {
                    DEBUG("Handling h\n");
                    if(hybernateBM019()) {
                        sprintf(answer,"+h!");
                    } else {
                        DEBUG("BM019 did hybernate wake\n")
                        sprintf(answer,"-h!");
                    }
                    handled=true;

                }
                    break;

                case 'w':
                case 'W': {
                    DEBUG("handle w\n")

                    if(wakeBM019(100)) {
                        DEBUG("BM019 did wake\n")
                        sprintf(answer,"+w!");
                    } else {
                        DEBUG("BM019 did NOT wake\n")
                        sprintf(answer,"-w!");
                    }
                    handled = true;
                }
                    break;

                case 'i':
                case 'I': {
                    DEBUG("handle i\n");
                    BM019_IDN idn;
                    if(idnBM019(&idn)) {
                        sprintf(answer,"+i:");
                        int i;
                        for(i = 0; i < 13; i++) {
                            sprintf(&answer[strlen(answer)],"%x",idn.deviceID[i]);
                        }
                        sprintf(&answer[strlen(answer)],":");
                        sprintf(&answer[strlen(answer)],"%x%x!",idn.romCRC[0],idn.romCRC[1]);
                        DEBUG("answered: %s",answer);
                    } else {
                        DEBUG("BM019 failed idn\n")
                        sprintf(answer,"-i!");
                    }

                    handled = true;
                }
                    break;

                case 'p':
                case 'P': {
                    DEBUG("handle p\n");
                    if(setProtocolISO_EIC_15693BM019((BM019_PROTOCOL_ISO_IEC_15693_BYTE_0)(
                                                                                           BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_0_CRC |
                                                                                           BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_1_SINGLE_SUBCARRIER |
                                                                                           BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_2_10_MODULATION |
                                                                                           BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_3_WAIT_FOR_SOF |
                                                                                           BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_45_26_KBPS)
                                                     )) {
                        DEBUG("BM019 proto\n")
                        sprintf(answer,"+p!");
                    } else {
                        DEBUG("BM019 failed proto\n")
                        sprintf(answer,"-p!");
                    }

                    handled = true;
                }
                    break;

                case 'r':
                case 'R': {
                    DEBUG("handle r\n");
                    resetBM019();
                    sprintf(answer,"+r!");
                    handled = true;
                }
                    break;

                case 'e':
                case 'E': {
                    DEBUG("handle e\n");
                    if(echoBM019()) {
                        DEBUG("BM019 sent echo\n");
                        sprintf(answer,"+e!");
                    } else {
                        DEBUG("BM019 NOT sent echo\n");
                        sprintf(answer,"-e!");
                    }
                    handled = true;
                }
                    break;

                case 't':
                case 'T': {
                    DEBUG("handle t\n");
                    BM019_TAG tag;
                    if(inventoryISO_IES_15693BM019(&tag)) {
                        DEBUG("BM019 answered inventory\n");
                        sprintf(answer,"+t:");
                        for(int i = 0; i < 8; i++) {
                            sprintf(&answer[strlen(answer)],"%02x",tag.uid[i]);
                        }
                        sprintf(&answer[strlen(answer)],"!");
                    } else {
                        DEBUG("BM019 NOT answered inventory\n");
                        sprintf(answer,"-t!");
                    }
                    handled = true;
                }
                    break;

                case 'd':
                case 'D': {
                    DEBUG("handle d\n");
                    if(i + 5 <= bytesRead) {
                        int adr = 0;
                        char b[3];
                        b[0] = params->data[i+4];
                        b[1] = params->data[i+5];
                        b[2] = 0;
                        sscanf(b,"%x",&adr);
                        DEBUG("read from %#04x\n",adr);
                        i+=5;
                        uint8_t rb[256];
                        int l = readBM019(adr,rb,256);
                        if(l>0) {
                            DEBUG("BM019 answered read\n");
                            sprintf(answer,"+d:");
                            for(int i = 0; i < l; i++) {
                                sprintf(&answer[strlen(answer)],"%02x",rb[i]);
                            }
                            sprintf(&answer[strlen(answer)],"!");
                        } else {
                            DEBUG("BM019 NOT answered read\n");
                            sprintf(answer,"-d!");
                        }
                    } else {
                        DEBUG("BM019 NOT answered read, no adr given\n");
                        sprintf(answer,"-d!");
                    }
                    handled = true;
                }
                    break;

                case 'm':
                case 'M': {
                    DEBUG("handle multi d\n");
                    if(i + 10 <= bytesRead) {
                        int adr = 0;
                        char b[3];
                        b[0] = params->data[i+4];
                        b[1] = params->data[i+5];
                        b[2] = 0;
                        sscanf(b,"%x",&adr);

                        int count = 0;
                        b[0] = params->data[i+9];
                        b[1] = params->data[i+10];
                        b[2] = 0;
                        sscanf(b,"%x",&count);
                        DEBUG("read from %#04x for %d\n",adr,count);
                        i+=10;
                        uint8_t rb[256];
                        int l = readMultiBM019(adr,count,rb,256);
                        if(l>0) {
                            DEBUG("BM019 answered multi\n");
                            sprintf(answer,"+m:");
                            for(int i = 0; i < l; i++) {
                                sprintf(&answer[strlen(answer)],"%02x",rb[i]);
                            }
                            sprintf(&answer[strlen(answer)],"!");
                        } else {
                            DEBUG("BM019 NOT answered multi\n");
                            sprintf(answer,"-m!");
                        }
                    } else {
                        DEBUG("BM019 NOT answered read, no adr&count given\n");
                        sprintf(answer,"-m!");
                    }
                    handled = true;
                }
                    break;
            }
        }

        if(handled) {
            DEBUG("writing \"%s\" with len %d to ble\n",answer,strlen(answer));
            int l = strlen(answer);
            for(int i = 0; i*20 < strlen(answer); i++) {
                int len = 20 < l ? 20 : l;
                ble.updateCharacteristicValue(uartServicePtr->getRXCharacteristicHandle(), (uint8_t *)&answer[i*20], len);
                l -= 20;
            }

        } else {
            DEBUG("received %u bytes.. nothing handled.. echo\n", bytesRead);
            ble.updateCharacteristicValue(uartServicePtr->getRXCharacteristicHandle(), params->data, bytesRead);
        }
    }
}

int main(void)
{
    initBM019();

    DEBUG("Initialising the nRF51822\n\r");
    ble.init();
    ble.onDisconnection(disconnectionCallback);
    ble.onDataWritten(onDataWritten);

    /* setup advertising */
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED);
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    //ble.setAdvertisingType(GapAdvertisingParams::ADV_SCANNABLE_UNDIRECTED);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,
                                     (const uint8_t *)"BLE UART2NFC", sizeof("BLE UART2NFC") - 1);


    /*ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,
     (const uint8_t *)UARTServiceUUID_reversed, sizeof(UARTServiceUUID_reversed));
     */

    ble.accumulateScanResponse(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,
                               (const uint8_t *)UARTServiceUUID_reversed, sizeof(UARTServiceUUID_reversed));
    ble.setAdvertisingInterval(1000); /* 1000ms; in multiples of 0.625ms. */
    //ble.setAdvertisingTimeout(0x1);
    ble.startAdvertising();
    
    UARTService uartService(ble);
    uartServicePtr = &uartService;
    
    while (true) {
        ble.waitForEvent();
    }
}

