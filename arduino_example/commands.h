
#ifndef COMMANDS
#define COMMANDS

#include <Arduino.h>

typedef bool (*cmd_ptr)(uint8_t*,int);

class Command
{
  public:
    Command(const char* cmd,const char* description,cmd_ptr ptr){
      _cmd = cmd;
      _description = description;
     _cmd_ptr = ptr;  
    }
    bool handle(uint8_t* buf, int cmdlen, int buflen)
    {
      int len = strlen(_cmd);
      if(len != cmdlen)
        return false;
       for(int i = 0; i < len;i++)
       {
        if(_cmd[i] != buf[i])
        {
          return false;
        }
       }
       if(buflen == cmdlen)
       {
          return _cmd_ptr(NULL,0);
       }
       else
       {
          return _cmd_ptr(&buf[cmdlen+1],buflen-cmdlen-1);
       }
    }
    void describe()
    {
      if(_description)
      {
        Serial.print(_cmd);
        Serial.print(" -> ");
        Serial.println(_description);
      }
    }

   private:
    const char* _cmd;
    const char* _description;
    cmd_ptr _cmd_ptr;
};
class CommandList;
typedef CommandList* CommandListPtr;

class CommandsClass
{
  public:
    CommandsClass();
    bool handle(uint8_t* buf,int length);
    CommandListPtr _commands;
    void addCommand(Command *command);
};

extern CommandsClass Commands;
#endif
