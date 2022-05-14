#include <stdio.h> // printf setvbuf
#include <stdlib.h> // free y malloc
#include <math.h> // round
#include <string.h> // strlen
#include "socal/socal.h" // alt_write_hword alt_read_hword
#include "socal/hps.h" // to BASEADD
#include "uart_core_lib.h"
#include "QueueCommand.h"
#include "Command.h"

// INITIAL CONFIG

UART_T uart_init(uint32_t baseadd)
{
  UART_T uart;
  uart.BASEADD   = baseadd;
  uart.RXDATA    = baseadd + (uint32_t)(0x00);
  uart.TXDATA    = baseadd + (uint32_t)(0x04);
  uart.STATUS    = baseadd + (uint32_t)(0x08);
  uart.CONTROL   = baseadd + (uint32_t)(0x0c);
  uart.DIVISOR   = baseadd + (uint32_t)(0x10);
  uart.ENDOFPACK = baseadd + (uint32_t)(0x14);

  // default buffersize value
  // 10KB
  uart.buffer_size = 10000000;

  return uart;
}

void setBaud(UART_T * uart_p, uint32_t clock_frequency, uint32_t baudrate)
{
  // Formula segun
  // Embedded Peripherals IP User Guide -  UART Core
  float divisor = round((float)clock_frequency/(float)baudrate + 0.5f);
  alt_write_hword(uart_p->DIVISOR, (uint16_t)divisor);
}

void setBufferSize(UART_T * uart_p, uint32_t size)
{
  uart_p->buffer_size = size;
  uart_p->buffer_count = 0;
  uart_p->buffer_pointer_init = malloc(size*sizeof(uint8_t));
  uart_p->buffer_pointer_writing = uart_p->buffer_pointer_init;
  uart_p->buffer_pointer_reading = uart_p->buffer_pointer_init;
}

void freeBuffer(UART_T * uart_p)
{
  free(uart_p->buffer_pointer_init);
}

// UART CORE REGISTER ACCESS

uint16_t uartRegister(uint32_t address)
{
  return alt_read_hword(address);
}

void checkUartRegisters(UART_T * uart_p)
{
  printf("%s\n", "*******************************");
  printf("%s ", "RX Data:");
  printf("%i\n", uartRegister(uart_p->RXDATA));
  printf("%s ", "TX Data:");
  printf("%i\n", uartRegister(uart_p->TXDATA));
  printf("%s ", "Status:");
  uint16_t status = uartRegister(uart_p->STATUS);
  printf("%i\n", status);
  printf("%s ", "Exception:");
  printf("%i\n", BIT(status, 8));
  printf("%s ", "ROE:");
  printf("%i\n", BIT(status, 3));
  printf("%s ", "TOE:");
  printf("%i\n", BIT(status, 4));
  printf("%s ", "Break:");
  printf("%i\n", BIT(status, 2));
  printf("%s ", "Frame Error:");
  printf("%i\n", BIT(status, 1));
  printf("%s\n", "*******************************");
  printf("%s ", "control:");
  printUartAdd(uart_p->CONTROL);
  printf("%s ", "divisor:");
  printUartAdd(uart_p->DIVISOR);
  printf("%s ", "end of packet:");
  printUartAdd(uart_p->ENDOFPACK);
}

void resetStatus(UART_T * uart_p)
{
  alt_write_hword(uart_p->STATUS, 0);
}

// DATA SENDING PROCESS

// Check if record is ready to send new character
// true if ready
bool checkTxdata(UART_T * uart_p)
{
  uint16_t status = alt_read_hword(uart_p->STATUS);
  return BIT(status, 6);
}

void sendChar(UART_T * uart_p, char character)
{
  while(checkTxdata(uart_p) == 0) {}; // txready is 1 when
  // the previous transmission was completed
  alt_write_hword(uart_p->TXDATA, (uint16_t)character);
}

void sendString(UART_T * uart_p, char * str, int num)
{
  for (int i = 0; i < num; ++i) {
    sendChar(uart_p, *str);
    str++;
  }
}

// DATA RECEPTION PROCESS

// Check if there is any data waiting to be read
// true if there is data waiting
bool checkRxdata(UART_T * uart_p)
{
  uint16_t status = alt_read_hword(uart_p->STATUS);
  return BIT(status, 7);
}

uint8_t receiveChar(UART_T * uart_p)
{
  return (char)alt_read_hword(uart_p->RXDATA);
}

// AUTOMATIC DATA RECEPTION PROCESS

void activateRecepcion(UART_T * uart_p)
{
  uart_p->thread_run = true;
  pthread_t thread = uart_p->thread;
  pthread_create(&thread, NULL, activateRecepcion_aux, uart_p);
}

void * activateRecepcion_aux(void * uart_ptr, void * queue_ptr)
{
  UART_T * uart_p = (UART_T*)uart_ptr; 
  CQueueCommand *pQueueCommand;
	pQueueCommand = (CQueueCommand *)ptr;
  setBufferSize(uart_p, uart_p->buffer_size); // create malloc

  while(1){
    uart_p->buffer_count = 0;
    uint8_t *posicionInicial = uart_p->buffer_pointer_writing;
    char Data = ' ';

    while(uart_p->thread_run && Data != 10) {
      if(checkRxdata(uart_p)) {
        *(uart_p->buffer_pointer_writing) = receiveChar(uart_p);
        Data = (char) *(uart_p->buffer_pointer_writing);
        uart_p->buffer_pointer_writing++;
      }
    }

    uart_p->buffer_count = 
    (uint32_t)(uart_p->buffer_pointer_writing) - (uint32_t)posicionInicial;

    bool bFind = false;
    int nNum, i, j, x = 0;
    bool find_equal = false;
    char Data[10]={0};
    nNum = sizeof(gCommandList)/sizeof(gCommandList[0]);
    int thisCommand, thisParam;

    for(i = 0; i < nNum && !bFind; i++){
      if (strncmp((char *) uart_p->buffer_pointer_reading, gCommandList[i].szCommand, 6) == 0){
        thisCommand = gCommandList[i].CommandId;
        uart_p->buffer_pointer_reading = uart_p->buffer_pointer_reading + 6;

        if (gCommandList[i].bParameter){
          while(1){
            if(find_equal == true){
              Data[x] = *(uart_p->buffer_pointer_reading);
              uart_p->buffer_pointer_reading++;
              x++;
            }
            else if(*(uart_p->buffer_pointer_reading) == '=')
              find_equal = true;
          }
          thisParam=atoi(Data);
        }
        bFind = true;
      } 
    }

    if (bFind){
      // push command to command queue
      if (thisCommand == CMD_LED_OFF)
        pQueueCommand->Clear();
      // push command to command queue
      if (!pQueueCommand->IsFull()){ 
        pQueueCommand->Push(thisCommand, thisParam);
      }
    }
  }
  return NULL;
}

void terminateRecepcion(UART_T * uart_p)
{
  uart_p->thread_run = false;
}

uint8_t * emptyBuffer(UART_T * uart_p, uint32_t * num)
{
  *num = uart_p->buffer_count;
  uint8_t * data_array = malloc(*num * sizeof(uint8_t));
  uint8_t * data_array_init = data_array;

  for(int i = 0; i<*num; i++) {
    *data_array = *(uart_p->buffer_pointer_reading);
    uart_p->buffer_pointer_reading++;
    data_array++;
  }
  uart_p->buffer_pointer_reading = uart_p->buffer_pointer_init;
  uart_p->buffer_pointer_writing = uart_p->buffer_pointer_init;
  uart_p->buffer_count = 0;

  return data_array_init;
}