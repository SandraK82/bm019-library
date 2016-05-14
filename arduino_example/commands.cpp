#include "commands.h"

bool _cmd_help(uint8_t* buf, int len);

class CommandList
{
    public:
      CommandList(Command *cmd, CommandListPtr next)
      {
          _cmd = cmd;
          _next = next;
      }
      bool handle(uint8_t* buf, int cmdlen, int buflen)
      {
        if(_cmd->handle(buf,cmdlen,buflen))
        {
          return true;
        }
        if(_next)
        {
          return _next->handle(buf,cmdlen,buflen);
        }
        return false;
      }
      CommandListPtr next()
      {
        return _next;
      }
      void describe()
      {
        _cmd->describe();
      }
     private:
      Command *_cmd;
      CommandListPtr _next;
};
CommandsClass Commands = CommandsClass();

CommandsClass::CommandsClass()
{
  addCommand(new Command("?",NULL,&_cmd_help));
}

void CommandsClass::addCommand(Command *cmd)
{
  _commands = new CommandList(cmd,_commands);
}
bool CommandsClass::handle(uint8_t* buf, int length)
{
  bool handled = false;
  int i = 0;
  for (; i < length; i++) {
    if (buf[i] == ':') 
    {
      break;
    }
  }
  if(_commands && _commands->handle(buf,i,length))
  {
      handled = true;
  }
  if(!handled)
  {
    Serial.println("try ? for help!");
  }
  return handled;
}

bool _cmd_help(uint8_t* buf,int len)
{
  Serial.println("available commands:");
  CommandListPtr p = Commands._commands;
  while(p)
  {
    p->describe();
    
    p = p->next();
  }
  return true;
}
        /*
          DEBUG("Handle ?\n");
          switch (getStateBM019()) {
            case BM019_STATE_UNKNOWN:
              DEBUG("BM019 Status: unknown\n");
              sprintf(answer, "+?:0!");
              break;
            case BM019_STATE_ANSWERING:
              DEBUG("BM019 Status: answering\n");
              sprintf(answer, "+?:1!");
              break;
            case BM019_STATE_PROTOCOL:
              DEBUG("BM019 Status: protocol set\n");
              sprintf(answer, "+?:2!");
              break;
            default:
              sprintf(answer, "-?%d!", getStateBM019());
              DEBUG("BM019 Status: forgotten state\n");
          }
          handled = true;
        }
        break;

      case 'b':
      case 'B':
        DEBUG("Handling b\n");
        if (i + 5 <= bytesRead) {
          sprintf(answer, "+b!");
          int adr = 0;
          char b[3];
          b[0] = params->data[i + 4];
          b[1] = params->data[i + 5];
          b[2] = 0;
          sscanf(b, "%x", &adr);
          DEBUG("beat in %d sec\n", adr);
          ticker.attach(timerCallback, adr);
          i += 5;
          handled = true;
        } else {
          sprintf(answer, "-b!");
          handled = true;
        }

        break;
      case 'h':
      case 'H': {
          DEBUG("Handling h\n");
          if (hybernateBM019()) {
            sprintf(answer, "+h!");
          } else {
            DEBUG("BM019 did hybernate wake\n")
            sprintf(answer, "-h!");
          }
          handled = true;

        }
        break;

      case 'w':
      case 'W': {
          DEBUG("handle w\n")

          if (wakeBM019(100)) {
            DEBUG("BM019 did wake\n")
            sprintf(answer, "+w!");
          } else {
            DEBUG("BM019 did NOT wake\n")
            sprintf(answer, "-w!");
          }
          handled = true;
        }
        break;

      case 'p':
      case 'P': {
          DEBUG("handle p\n");
          if (setProtocolISO_EIC_15693BM019((BM019_PROTOCOL_ISO_IEC_15693_BYTE_0)(
                                              BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_0_CRC |
                                              BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_1_SINGLE_SUBCARRIER |
                                              BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_2_10_MODULATION |
                                              BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_3_WAIT_FOR_SOF |
                                              BM019_PROTOCOL_ISO_IEC_15693_BYTE_0_45_26_KBPS)
                                           )) {
            DEBUG("BM019 proto\n")
            sprintf(answer, "+p!");
          } else {
            DEBUG("BM019 failed proto\n")
            sprintf(answer, "-p!");
          }

          handled = true;
        }
        break;

      case 'r':
      case 'R': {
          DEBUG("handle r\n");
          resetBM019();
          sprintf(answer, "+r!");
          handled = true;
        }
        break;

      case 't':
      case 'T': {
          DEBUG("handle t\n");
          BM019_TAG tag;
          if (inventoryISO_IES_15693BM019(&tag)) {
            DEBUG("BM019 answered inventory\n");
            sprintf(answer, "+t:");
            for (int i = 0; i < 8; i++) {
              sprintf(&answer[strlen(answer)], "%02x", tag.uid[i]);
            }
            sprintf(&answer[strlen(answer)], "!");
          } else {
            DEBUG("BM019 NOT answered inventory\n");
            sprintf(answer, "-t!");
          }
          handled = true;
        }
        break;

      case 'd':
      case 'D': {
          DEBUG("handle d\n");
          if (i + 5 <= bytesRead) {
            int adr = 0;
            char b[3];
            b[0] = params->data[i + 4];
            b[1] = params->data[i + 5];
            b[2] = 0;
            sscanf(b, "%x", &adr);
            DEBUG("read from %#04x\n", adr);
            i += 5;
            uint8_t rb[256];
            int l = readBM019(adr, rb, 256);
            if (l > 0) {
              DEBUG("BM019 answered read\n");
              sprintf(answer, "+d:");
              for (int i = 0; i < l; i++) {
                sprintf(&answer[strlen(answer)], "%02x", rb[i]);
              }
              sprintf(&answer[strlen(answer)], "!");
            } else {
              DEBUG("BM019 NOT answered read\n");
              sprintf(answer, "-d!");
            }
          } else {
            DEBUG("BM019 NOT answered read, no adr given\n");
            sprintf(answer, "-d!");
          }
          handled = true;
        }
        break;

      case 'm':
      case 'M': {
          DEBUG("handle multi d\n");
          if (i + 10 <= bytesRead) {
            int adr = 0;
            char b[3];
            b[0] = params->data[i + 4];
            b[1] = params->data[i + 5];
            b[2] = 0;
            sscanf(b, "%x", &adr);

            int count = 0;
            b[0] = params->data[i + 9];
            b[1] = params->data[i + 10];
            b[2] = 0;
            sscanf(b, "%x", &count);
            DEBUG("read from %#04x for %d\n", adr, count);
            i += 10;
            uint8_t rb[256];
            int l = readMultiBM019(adr, count, rb, 256);
            if (l > 0) {
              DEBUG("BM019 answered multi\n");
              sprintf(answer, "+m:");
              for (int i = 0; i < l; i++) {
                sprintf(&answer[strlen(answer)], "%02x", rb[i]);
              }
              sprintf(&answer[strlen(answer)], "!");
            } else {
              DEBUG("BM019 NOT answered multi\n");
              sprintf(answer, "-m!");
            }
          } else {
            DEBUG("BM019 NOT answered read, no adr&count given\n");
            sprintf(answer, "-m!");
          }
          handled = true;
        }
        break;
    }
  }

  if (handled) {
  */
