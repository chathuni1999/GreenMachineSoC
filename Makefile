#
TARGET = HPS_FPGA_BT

#
ALT_DEVICE_FAMILY ?= soc_cv_av
SOCEDS_ROOT ?= $(SOCEDS_DEST_ROOT)
HWLIBS_ROOT = $(SOCEDS_ROOT)/ip/altera/hps/altera_hps/hwlib
CROSS_COMPILE = arm-linux-gnueabihf-
CFLAGS = -g -Wall -std=c++11 -I$(HWLIBS_ROOT)/include -I$(HWLIBS_ROOT)/include/$(ALT_DEVICE_FAMILY) -D$(ALT_DEVICE_FAMILY) -I./bt/inc
LDFLAGS =  -g -Wall  -lstdc++ -L./bt/lib -lbluetooth -lrt -lpthread
CC = $(CROSS_COMPILE)g++
ARCH= arm

all: $(TARGET)

$(TARGET): main.o terasic_os.o mmap.o uart_core_lib.o Command.o Queue.o QueueCommand.o
	$(CC) $(LDFLAGS)   $^ -o $@

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) *.a *.o *~
