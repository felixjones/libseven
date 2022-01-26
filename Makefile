CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar

SOURCES = \
	src/hw/dma.s \
	src/hw/input.s \
	src/hw/irq.s \
	src/hw/lcd.s \
	src/hw/sram.s \
	src/hw/svc.s \
	src/hw/timer.c \
	src/util/log.c \
	src/util/mem.s \
	src/util/profile.s \
	src/util/rand.s \
	src/util/str.s \

INCLUDES = \
	src \
	include

CFLAGS = \
	-Os \
	-g \
	-ffunction-sections \
	-fdata-sections \
	-ffreestanding \
	-std=c99 \
	-Wall \
	-Wpedantic \
	-mabi=aapcs \
	-mcpu=arm7tdmi \
	-mthumb \
	$(INCLUDES:%=-I%)

BUILD = build

TARGET = libseven.a

#
# You don't need to touch anything below this point!
#

OBJECTS = $(patsubst %,$(BUILD)/obj/%.o,$(SOURCES))
DEPENDS = $(patsubst %,$(BUILD)/dep/%.d,$(SOURCES))
OBJDIRS = $(dir $(OBJECTS) $(DEPENDS))

$(BUILD)/$(TARGET): $(OBJECTS)
	@echo "AR      $@"
	@$(AR) rcs $@ $^

$(OBJECTS): | objdirs

$(BUILD)/obj/%.o: %
	@echo "COMPILE $<"
	@$(CC) -c -o $@ $(CFLAGS) -MMD -MP -MF $(BUILD)/dep/$<.d $<

objdirs:
	@echo "OUTDIR  $(BUILD)"
	@mkdir -p $(OBJDIRS)

clean:
	@echo "CLEAN   $(BUILD)"
	@rm -rf $(BUILD)

install: $(BUILD)/$(TARGET)
	@echo "INSTALL $(DESTDIR)$(DEVKITPRO)"
	@mkdir -p $(DESTDIR)$(DEVKITPRO)/libseven/lib
	@cp -rv include $(DESTDIR)$(DEVKITPRO)/libseven/include
	@cp -v $(BUILD)/$(TARGET) $(DESTDIR)$(DEVKITPRO)/libseven/lib

.PHONY: objdirs clean install

-include $(DEPENDS)
