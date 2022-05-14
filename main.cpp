#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include "socal/alt_uart.h"
#include "hps_0.h" 
#include "uart_core_lib.h"
#include "Command.h"
#include "Queue.h"
#include "QueueCommand.h"
#include "terasic_os.h"

#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )
#define DEBUG_DUMP  /*printf */

enum {FALSE, TRUE};

typedef alt_u8 	uint8_t;
typedef alt_8 	int8_t;
typedef alt_u16 uint16_t;
typedef alt_16 	int16_t;
typedef alt_u32 uint32_t;
typedef alt_32 	int32_t;

typedef enum{
	CMD_MOI,
	CMD_TMP,
	CMD_HUM,
	CMD_LIT,
	CMD_LED_ON,
	CMD_LED_OFF,
	CMD_IDLE,
}COMMAND_ID;

int main()
{
	printf("1\n"); // Debug milestones

	int fd;
	void *virtual_base;
	void *uart_base;

	printf("2\n"); // Debug milestones

	printf("ALT_STM_OFST Value: 0x%08x\n", ALT_STM_OFST);
	printf("ALT_LWFPGASLVS_OFST Value: 0x%08x\n", ALT_LWFPGASLVS_OFST);

	// map the address space for the LED registers into user space so we can interact with them.
	// we'll actually map in the entire CSR span of the HPS since we want to access various registers within that span
	if ((fd = open( "/dev/mem", (O_RDWR | O_SYNC))) == -1) 
	{
		printf("ERROR: could not open \"/dev/mem\"...\n");
		return -1;
	}

	printf("3\n"); // Debug milestones

	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );

	if (virtual_base == MAP_FAILED) 
	{
		printf("ERROR: mmap() failed...\n");
		close(fd);
		return -1;
	}
	
	printf("4\n"); // Debug milestones

	// Define addresses
	uart_base = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + HC05_UART_BASE) & ( unsigned long)( HW_REGS_MASK ) );

	CQueueCommand QueueCommand;
    int Command, Param;
    bool bSleep = false;
    pthread_t id0;
    int ret0;

	// Initializing the Bluetooth UART 
	UART_T uart = uart_init(uart_base);
	setBaud(&uart, HC05_UART_FREQ, HC05_UART_BAUD);
	freeBuffer(&uart);
	checkUartRegisters(&uart);
	resetStatus(&uart);

	// Enabling automatic reception using threads
	activateRecepcion(&uart, (void *)&QueueCommand);

	// Write the iterative function
	while (1) 
	{	
		if(!QueueCommand.IsEmpty() && QueueCommand.Pop(&Command, &Param) ){
			switch(Command){
			case CMD_MOI:printf("CMD_MOI\n");
								// LED_PIO.SetLED(0x1);
							  break;
			case CMD_TMP:printf("CMD_TMP\n");
								// LED_PIO.SetLED(0x2);
							  break;

			case CMD_HUM:printf("CMD_HUM\n");
								// LED_PIO.SetLED(0x4);
							  break;

			case CMD_LIT:printf("CMD_LIT\n");
								// LED_PIO.SetLED(0x8);
							   break;

			case CMD_LED_ON:printf("CMD_ON\n");
								// LED_PIO.SetLED(0xFF);
							   break;

			case CMD_LED_OFF:printf("CMD_OFF\n");
								// LED_PIO.SetLED(0x0);
							   break;

			default:printf("Nothing\n");break;
			} 
		}	
	}	

	terminateRecepcion(&uart);
	int num;
	emptyBuffer(&uart, &num)
	// Clean up our memory mapping and exit
	if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		return( 1 );
	}


	close(fd);
	return 0;
}
