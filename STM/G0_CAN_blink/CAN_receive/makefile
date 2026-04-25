# Toolchain definition
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# Project Folders
SRCDIR = src
INCDIR = inc
OBJDIR = obj

# Project Files
TARGET  = firmware.elf
SRCS    = $(wildcard $(SRCDIR)/*.c)
ASMS    = $(wildcard $(SRCDIR)/*.s)

# Create object list
OBJS    = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS)) \
          $(patsubst $(SRCDIR)/%.s, $(OBJDIR)/%.o, $(ASMS))

# --- Target Specific Configuration ---
# Only referencing the Flash linker script
LINKER_SCRIPT = STM32G0B1RETX_FLASH.ld

# MCU Flags for STM32G0 (Cortex-M0+)
MCU     = -mcpu=cortex-m0plus -mthumb
DEFINES = -DSTM32G0B1xx
CFLAGS  = $(MCU) $(DEFINES) -I$(INCDIR) -O0 -g3 -Wall -ffunction-sections -fdata-sections
LDFLAGS = $(MCU) -T $(LINKER_SCRIPT) -u _printf_float --specs=nosys.specs --specs=nano.specs -Wl,--gc-sections

# --- Build Rules ---

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@
	@echo --- Flash Build Finished ---

# Rule for C files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for Assembly files
$(OBJDIR)/%.o: $(SRCDIR)/%.s | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	@if not exist $(OBJDIR) mkdir $(OBJDIR)

# --- Upload Rule ---
# Points to the g0x configuration for OpenOCD
flash: $(TARGET)
	openocd -f interface/stlink.cfg -f target/stm32g0x.cfg -c "program $(TARGET) verify reset exit"

# --- Clean Rule ---
clean:
	@if exist $(OBJDIR) rmdir /s /q $(OBJDIR)
	@if exist $(TARGET) del /q $(TARGET)

# ---debug rule---
debug:
	openocd -f interface/stlink.cfg -f target/stm32g0x.cfg

gdb: $(TARGET)
	arm-none-eabi-gdb -ex "target extended-remote localhost:3333" \
	-ex "monitor reset halt" \
	-ex "load" \
	$(TARGET)


.PHONY: all flash clean debug gdb