#ifndef COMMAND_H_
#define COMMAND_H_

class Command {
public:
	typedef enum{
	CMD_MOI,
	CMD_TMP,
	CMD_HUM,
	CMD_LIT,
	CMD_LED_ON,
	CMD_LED_OFF,
	CMD_IDLE
	}COMMAND_ID;

};

#endif /* COMMAND_H_ */