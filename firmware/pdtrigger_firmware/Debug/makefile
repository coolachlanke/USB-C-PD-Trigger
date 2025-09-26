################################################################################
# Simple Makefile for STM32F401 + HAL
################################################################################

# Toolchain
CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE    := arm-none-eabi-size

# Project name
TARGET := pdtrigger_firmware

# MCU flags
MCU      := cortex-m4
FPU      := fpv4-sp-d16
FLOATABI := hard

CFLAGS  := -mcpu=$(MCU) -mthumb -mfloat-abi=$(FLOATABI) -mfpu=$(FPU)
CFLAGS  += -std=gnu11 -O0 -g3 -Wall -ffunction-sections -fdata-sections
CFLAGS  += -DUSE_HAL_DRIVER -DSTM32F401xC
CFLAGS  += -I../Core/Inc \
           -I../Drivers/STM32F4xx_HAL_Driver/Inc \
           -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy \
           -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include \
           -I../Drivers/CMSIS/Include

LDFLAGS := -T../STM32F401RCTX_FLASH.ld -Wl,-Map=$(TARGET).map,--gc-sections
LDFLAGS += --specs=nosys.specs --specs=nano.specs

# Sources (auto-collect .c files)
C_SOURCES := $(wildcard ../Core/Src/*.c) \
             $(wildcard ../Drivers/STM32F4xx_HAL_Driver/Src/*.c)

ASM_SOURCES := ../Core/Startup/startup_stm32f401rctx.s

OBJS := $(C_SOURCES:.c=.o) $(ASM_SOURCES:.s=.o)

# Default build
all: $(TARGET).elf $(TARGET).bin $(TARGET).hex size

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

size: $(TARGET).elf
	$(SIZE) $<

flash: $(TARGET).bin
	dfu-util -a 0 -s 0x08000000:leave -D $(TARGET).bin

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin $(TARGET).hex $(TARGET).map
