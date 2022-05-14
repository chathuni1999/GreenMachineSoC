#include "Command.h"
#include <stdlib.h>

typedef struct{
	char szCommand[10];
	int CommandId;
	bool bParameter;
}COMMAND_INFO;

COMMAND_INFO gCommandList[] = {
		{"AT+MOI\n", Command::CMD_MOI, true},
		{"AT+TMP\n", Command::CMD_TMP, false},
		{"AT+HUM\n", Command::CMD_HUM, false},
		{"AT+LIT\n", Command::CMD_LIT, false},
		{"AT++ON\n"  , Command::CMD_LED_ON, false},
		{"AT+OFF\n", Command::CMD_LED_OFF, false},
};

Command::Command(){

}