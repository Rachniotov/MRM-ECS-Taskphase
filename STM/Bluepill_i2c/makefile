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

# Create object list for both .c and .s files
OBJS    = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS)) \
          $(patsubst $(SRCDIR)/%.s, $(OBJDIR)/%.o, $(ASMS))

# Compilation Flags
MCU     = -mcpu=cortex-m3 -mthumb
DEFINES = -DSTM32F103x6
CFLAGS  = $(MCU) $(DEFINES) -I$(INCDIR) -O0 -g3 -Wall -ffunction-sections -fdata-sections
LDFLAGS = $(MCU) -T STM32F103XX_FLASH.ld --specs=nosys.specs --specs=nano.specs -Wl,--gc-sections

# --- Build Rules ---

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@
	@echo --- Build Finished ---

# Rule for C files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for Assembly files (required for Reset_Handler)
$(OBJDIR)/%.o: $(SRCDIR)/%.s | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	@if not exist $(OBJDIR) mkdir $(OBJDIR)

# --- Upload Rule ---
flash: $(TARGET)
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program $(TARGET) verify reset exit"

# --- Clean Rule ---
clean:
	@if exist $(OBJDIR) rmdir /s /q $(OBJDIR)
	@if exist $(TARGET) del /q $(TARGET)

.PHONY: all flash clean