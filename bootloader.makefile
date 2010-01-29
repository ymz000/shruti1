# Copyright 2009 Olivier Gillet.
#
# Author: Olivier Gillet (ol.gillet@gmail.com)
# Parts of this file adapted from the Arduino command line tools Makefile
# M J Oldfield ; mellis, eighthave, oli.keller.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


AVR_TOOLS_PATH = /Applications/arduino-0016/hardware/tools/avr/bin
AVR_ETC_PATH   = /Applications/arduino-0016/hardware/tools/avr/etc

TARGET         = muboot
PACKAGES       = hardware/bootloader
BUILD_DIR      = build/$(TARGET)

MCU            = atmega328p
DMCU           = m328p
F_CPU          = 16000000
# SERIAL_PORT  = /dev/cu.usbserial-A6008iA6
SERIAL_PORT    = /dev/cu.usbserial-A6008hLO


# ------------------------------------------------------------------------------


VPATH          = $(PACKAGES)
CC_FILES       = $(notdir $(wildcard $(patsubst %,%/*.cc,$(PACKAGES))))
AS_FILES       = $(notdir $(wildcard $(patsubst %,%/*.as,$(PACKAGES))))
OBJ_FILES      = $(CC_FILES:.cc=.o) $(AS_FILES:.S=.o)
OBJS           = $(patsubst %,$(BUILD_DIR)/%,$(OBJ_FILES))
DEPS           = $(OBJS:.o=.d)

TARGET_HEX     = $(BUILD_DIR)/$(TARGET).hex
TARGET_ELF     = $(BUILD_DIR)/$(TARGET).elf
TARGETS        = $(BUILD_DIR)/$(TARGET).*
DEP_FILE       = $(BUILD_DIR)/depends.mk

CC             = $(AVR_TOOLS_PATH)/avr-gcc
CXX            = $(AVR_TOOLS_PATH)/avr-g++
OBJCOPY        = $(AVR_TOOLS_PATH)/avr-objcopy
OBJDUMP        = $(AVR_TOOLS_PATH)/avr-objdump
AR             = $(AVR_TOOLS_PATH)/avr-ar
SIZE           = $(AVR_TOOLS_PATH)/avr-size
NM             = $(AVR_TOOLS_PATH)/avr-nm
AVRDUDE        = $(AVR_TOOLS_PATH)/avrdude
REMOVE         = rm -f
CAT            = cat

#TODO(pichenettes): add command for writing bootloader
#/Applications/arduino-0016/hardware/tools/avr/bin/avrdude -C /Applications/arduino-0016/hardware/tools/avr/etc/avrdude.conf -c avrispmkII -P usb -p atmega328p -V -U flash:w:Hardware/build/muboot/muboot.hex
#atmega328_isp: HFUSE = DA
#atmega328_isp: LFUSE = FF
#atmega328_isp: EFUSE = 05
#ISPPORT	   = usb
#ISPSPEED   = -b 115200
#ISPFUSES    = avrdude -c $(ISPTOOL) -p $(MCU_TARGET) -P $(ISPPORT) $(ISPSPEED) \
#-e -u -U lock:w:0x3f:m -U efuse:w:0x$(EFUSE):m -U hfuse:w:0x$(HFUSE):m -U lfuse:w:0x$(LFUSE):m
#ISPFLASH    = avrdude -c $(ISPTOOL) -p $(MCU_TARGET) -P $(ISPPORT) $(ISPSPEED) \
#-U flash:w:$(PROGRAM)_$(TARGET).hex -U lock:w:0x0f:m

CPPFLAGS      = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I. \
			-g -Os -w -Wall \
			-ffunction-sections -fdata-sections \
			-funsigned-char -fno-split-wide-types \
			-fno-inline-small-functions -mcall-prologues
CXXFLAGS      = -fno-exceptions
ASFLAGS       = -mmcu=$(MCU) -I. -x assembler-with-cpp
LDFLAGS       = -mmcu=$(MCU) -lm -Wl,--gc-sections,--section-start=.text=0x7800,--relax -nostartfiles

# ------------------------------------------------------------------------------
# Source compiling
# ------------------------------------------------------------------------------

$(BUILD_DIR)/%.o: %.cc
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s
	$(CC) -c $(CPPFLAGS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/%.d: %.cc
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) $< -MF $@ -MT $(@:.d=.o)

$(BUILD_DIR)/%.d: %.s
	$(CC) -MM $(CPPFLAGS) $(ASFLAGS) $< -MF $@ -MT $(@:.d=.o)


# ------------------------------------------------------------------------------
# Object file conversion
# ------------------------------------------------------------------------------

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(BUILD_DIR)/%.lss: $(BUILD_DIR)/%.elf
	$(OBJDUMP) -h -S $< > $@

$(BUILD_DIR)/%.sym: $(BUILD_DIR)/%.elf
	$(NM) -n $< > $@


# ------------------------------------------------------------------------------
# AVRDude
# ------------------------------------------------------------------------------

AVRDUDE_CONF     = $(AVR_ETC_PATH)/avrdude.conf
AVRDUDE_COM_OPTS = -q -V -p $(DMCU)
AVRDUDE_COM_OPTS += -C $(AVRDUDE_CONF)
AVRDUDE_SER_OPTS = -c stk500v1 -b 57600 -P $(SERIAL_PORT)


# ------------------------------------------------------------------------------
# Main targets
# ------------------------------------------------------------------------------

all: 		$(BUILD_DIR) $(TARGET_HEX)

$(BUILD_DIR):
		mkdir -p $(BUILD_DIR)

$(TARGET_ELF): 	$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(SYS_OBJS)

$(DEP_FILE):	$(BUILD_DIR) $(DEPS)
		cat $(DEPS) > $(DEP_FILE)

upload:		$(TARGET_HEX)
		$(AVRDUDE) $(AVRDUDE_COM_OPTS) $(AVRDUDE_SER_OPTS) \
			-U flash:w:$(TARGET_HEX):i

clean:
		$(REMOVE) $(OBJS) $(TARGETS) $(DEP_FILE) $(DEPS)

depends:	$(DEPS)
		cat $(DEPS) > $(DEP_FILE)

bootloader_size:	$(TARGET_ELF)
		$(SIZE) $(TARGET_ELF) > bootloader_size

$(BUILD_DIR)/$(TARGET).top_symbols:	$(TARGET_ELF)
		$(NM) $(TARGET_ELF) --size-sort -C -f bsd -r > $@

size:	bootloader_size
		cat bootloader_size | awk '{ print $$1+$$2 }' | tail -n1 | figlet | cowsay -n -f moose

size_report:	build/$(TARGET)/$(TARGET).lss build/$(TARGET)/$(TARGET).top_symbols

.PHONY:	all clean depends upload