#include <bm019.h>
#include "commands.h"

BM019_UART bm019 = BM019_UART(&Serial1);

bool _idn(uint8_t* buf, int len)
{
  BM019_IDN idn;
  if (bm019.idn(&idn))
  {
    Serial.print("+idn:");
    int i;
    for (i = 0; i < 13; i++) {
      Serial.print(idn.deviceID[i], HEX);
    }
    Serial.print(":");
    Serial.print(idn.romCRC[0], HEX);
    Serial.print(idn.romCRC[1], HEX);
    Serial.println("!");
  } else {
    Serial.println("-idn!");
  }
  return true;
}
bool _echo(uint8_t* buf, int len)
{
  if (bm019.echo()) {
    Serial.println("+echo!");
  } else {
    Serial.println("-echo!");
  }
  return true;
}

bool _status(uint8_t* buf, int len)
{
  switch (bm019.getState()) {
    case BM019_STATE_UNKNOWN:
      Serial.println("+Status:0! 'UNKNOWN'");
      break;
    case BM019_STATE_ANSWERING:
      Serial.println("+Status:1! 'ANSWERING'");
      break;
    case BM019_STATE_PROTOCOL:
      Serial.println("+Status:2! 'PROTOCOL'");
      break;
    default:
      Serial.print("-Status! '");
      Serial.print(bm019.getState());
      Serial.println("'");
  }
  return true;
}
bool _hybernate(uint8_t* buf, int len)
{
  if (bm019.hybernate())
  {
    Serial.println("+hybernate!");
  }
  else
  {
    Serial.println("-hybernate!");
  }
  return true;
}
bool _wake(uint8_t* buf, int len)
{
  if (bm019.wake())
  {
    Serial.println("+wake!");
  }
  else
  {
    Serial.println("-wake!");
  }
  return true;
}

bool _protocol(uint8_t* buf, int len)
{
  if (len > 0)
    Serial.println("no proto def");
  if (bm019.setProtocolISO_EIC_15693(
        (BM019_PROTOCOL_ISO_IEC_15693_BYTE_0)
        (BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_0_CRC |
         BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_1_SINGLE_SUBCARRIER |
         BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_2_10_MODULATION |
         BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_3_WAIT_FOR_SOF |
         BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_45_26_KBPS))) {
    Serial.println("+protocol!");
  }
  else
  {
    Serial.println("-protocol!");
  }
  return true;
}

bool _tag(uint8_t* buf, int len)
{
  BM019_TAG tag;
  if (bm019.inventoryISO_IES_15693(&tag)) {
    Serial.print("+tag:");
    for (int i = 0; i < 8; i++) {
      Serial.print(tag.uid[i], HEX);
    }
    Serial.println("!");
  } else {
    Serial.println("-tag!");
  }
  return true;
}

bool _data(uint8_t* buf, int len)
{
  if (len > 0) {
    uint8_t adr = 0;
    for(int i = 0; i < len;i++)
    {
      if(buf[i] >=48 && buf[i] <=57)
      {
        adr*=10;
        adr+=(buf[i]-48);
      }
      else
      {
        Serial.println("only dec adr block!");
        return false;
      }
    }
    uint8_t rb[256];
    int l = bm019.readSingle(adr, rb, 256);
    if (l > 0) {
      Serial.print("+data:");
      for (int i = 0; i < l; i++) {
        if(rb[i] < 16) Serial.print(0);
        Serial.print(rb[i],HEX);
      }
      Serial.println("!");
    } else {
      Serial.println("-data!");
    }
  } else {
    Serial.println("-data!");
  }
  return true;
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  Serial.println("starting");

  Commands.addCommand(new Command("idn", "idn command of bm019, alias i,I", &_idn));
  Commands.addCommand(new Command("i", NULL, &_idn));
  Commands.addCommand(new Command("I", NULL, &_idn));
  Commands.addCommand(new Command("echo", "echo to bm019 alias e,E", &_echo));
  Commands.addCommand(new Command("e", NULL, &_echo));
  Commands.addCommand(new Command("E", NULL, &_echo));
  Commands.addCommand(new Command("status", "status of bm019 alias s,S", &_status));
  Commands.addCommand(new Command("s", NULL, &_status));
  Commands.addCommand(new Command("S", NULL, &_status));
  Commands.addCommand(new Command("hybernate", "set bm019 to hybernate alias h,H", &_hybernate));
  Commands.addCommand(new Command("h", NULL, &_hybernate));
  Commands.addCommand(new Command("H", NULL, &_hybernate));
  Commands.addCommand(new Command("wake", "wake bm019 from hybernate alias w,W", &_wake));
  Commands.addCommand(new Command("w", NULL, &_wake));
  Commands.addCommand(new Command("W", NULL, &_wake));
  Commands.addCommand(new Command("protocol", "set bm019 into iso15693 protocol alias p,P", &_protocol));
  Commands.addCommand(new Command("p", NULL, &_protocol));
  Commands.addCommand(new Command("P", NULL, &_protocol));
  Commands.addCommand(new Command("tag", "search for iso15693 tag alias t,T", &_tag));
  Commands.addCommand(new Command("t", NULL, &_tag));
  Commands.addCommand(new Command("T", NULL, &_tag));
  Commands.addCommand(new Command("data:BLOCK", "read the block at adr BLOCK from an iso15693 tag alias d,D", &_data));
  Commands.addCommand(new Command("d", NULL, &_data));
  Commands.addCommand(new Command("D", NULL, &_data));
}

uint8_t buf[512];
int pos = 0;

void loop() {
  if (Serial.available())
  {
    buf[pos] = Serial.read();
    if (buf[pos] == '\n')
    {
      Commands.handle(buf, pos);
      pos = 0;
    }
    else
    {
      pos++;
    }
  }
}
