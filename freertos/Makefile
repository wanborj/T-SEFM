CROSS_COMPILE=arm-none-eabi-
QEMU_STM32 ?= ../../qemu_stm32/arm-softmmu/qemu-system-arm

ARCH=CM3
VENDOR=ST
PLAT=STM32F10x
CMSIS_LIB=libraries/CMSIS/$(ARCH)
STM32_LIB=libraries/STM32F10x_StdPeriph_Driver

CMSIS_PLAT_SRC = $(CMSIS_LIB)/DeviceSupport/$(VENDOR)/$(PLAT)

FREERTOS_SRC = libraries/FreeRTOS
FREERTOS_INC = $(FREERTOS_SRC)/include/                                       
FREERTOS_PORT_INC = $(FREERTOS_SRC)/portable/GCC/ARM_$(ARCH)/

# benchmark by Wanbo
PAPA_AVR_INC = benchmark/avr/include
PAPA_LIB = benchmark/sw/lib/
PAPA_INC = benchmark/sw/include
PAPAVAR_INC = benchmark/sw/var/include
PAPA_AUTO_SRC = benchmark/sw/airborne/autopilot
PAPA_AUTO_INC = $(PAPA_AUTO_SRC)/include
PAPA_FBW_SRC = benchmark/sw/airborne/fly_by_wire
PAPA_FBW_INC = $(PAPA_FBW_SRC)/include



ifdef PAPABENCH_SINGLE
PAPABENCH_FLAGS = PAPABENCH_SINGLE=yes
FBW_FLAGS = PAPABENCH_NOLINK=yes
endif

all: autopilot main.bin

# add by Wanbo
autopilot: 
	cd $(PAPA_AUTO_SRC); make $(PAPABENCH_FLAGS) all

main.bin: main.c
	$(CROSS_COMPILE)gcc \
		-Wl,-Tmain.ld -nostartfiles \
		-I. -I$(FREERTOS_INC) -I$(FREERTOS_PORT_INC) \
		-Ilibraries/CMSIS/CM3/CoreSupport \
		-Ilibraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x \
		-Ilibraries/STM32F10x_StdPeriph_Driver/inc \
		-I$(PAPA_AVR_INC) \
		-I$(PAPA_INC) \
		-I$(PAPAVAR_INC) \
		-I$(PAPA_AUTO_INC) \
		-I$(PAPA_FBW_INC) \
		-fno-common -O0 \
		-gdwarf-2 -g3 \
		-mcpu=cortex-m3 -mthumb \
		-o main.elf \
		\
		$(CMSIS_LIB)/CoreSupport/core_cm3.c \
		$(CMSIS_PLAT_SRC)/system_stm32f10x.c \
		$(CMSIS_PLAT_SRC)/startup/gcc_ride7/startup_stm32f10x_md.s \
		$(STM32_LIB)/src/stm32f10x_rcc.c \
		$(STM32_LIB)/src/stm32f10x_gpio.c \
		$(STM32_LIB)/src/stm32f10x_usart.c \
		$(STM32_LIB)/src/stm32f10x_exti.c \
		$(STM32_LIB)/src/misc.c \
		\
		$(FREERTOS_SRC)/croutine.c \
		$(FREERTOS_SRC)/eventlist.c\
		$(FREERTOS_SRC)/servant.c\
		$(FREERTOS_SRC)/app.c\
		$(FREERTOS_SRC)/list.c \
		$(FREERTOS_SRC)/queue.c \
		$(FREERTOS_SRC)/tasks.c \
		$(FREERTOS_SRC)/portable/GCC/ARM_CM3/port.c \
		$(FREERTOS_SRC)/portable/MemMang/heap_2.c \
		\
	    $(PAPA_AUTO_SRC)/main_auto.o \
	    $(PAPA_AUTO_SRC)/modem.o\
	    $(PAPA_AUTO_SRC)/link_fbw.o\
	    $(PAPA_AUTO_SRC)/spi_auto.o \
	    $(PAPA_AUTO_SRC)/adc.o \
	    $(PAPA_AUTO_SRC)/gps_ubx.o \
	    $(PAPA_AUTO_SRC)/infrared.o \
	    $(PAPA_AUTO_SRC)/pid.o \
	    $(PAPA_AUTO_SRC)/nav.o \
	    $(PAPA_AUTO_SRC)/uart_auto.o \
	    $(PAPA_AUTO_SRC)/estimator.o \
	    $(PAPA_AUTO_SRC)/if_calib.o \
	    $(PAPA_AUTO_SRC)/uart_auto.o \
	    $(PAPA_AUTO_SRC)/uart_auto.o \
		\
		$(PAPA_LIB)/c/math.c  \
		$(PAPA_FBW_SRC)/adc_fbw.c  \
		$(PAPA_FBW_SRC)/ppm.o \
		$(PAPA_FBW_SRC)/spi_fbw.o \
		$(PAPA_FBW_SRC)/main_fbw.o \
		$(PAPA_FBW_SRC)/servo.o \
		$(PAPA_FBW_SRC)/uart_fbw.o \
		\
		stm32_p103.c \
		main.c
	$(CROSS_COMPILE)objcopy -Obinary main.elf main.bin
	$(CROSS_COMPILE)objdump -S main.elf > main.list


qemu: main.bin $(QEMU_STM32)
#	$(QEMU_STM32) -monitor stdio -M stm32-p103 -kernel main.bin
	$(QEMU_STM32) -M stm32-p103 -kernel main.bin

qemudbg: main.bin $(QEMU_STM32)
	$(QEMU_STM32) -M stm32-p103 \
		-gdb tcp::3333 -S \
		-kernel main.bin

qemuauto: main.bin
	bash emulate.sh main.bin

emu: main.bin
	bash emulate.sh main.bin

clean:
	rm -f *.elf *.bin *.list
	cd benchmark; make clean;
