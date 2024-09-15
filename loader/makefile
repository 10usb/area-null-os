include ../env.mk
ENTRY=start.asm
SOURCES=main.c tty.c text.c isr.c isr.asm irq.c memory.c rtc.c floppy.c dma.c
OBJECTS=$(patsubst %.asm,obj/asm/%.o,$(patsubst %.c,obj/c/%.o,$(SOURCES)))
# Shared dependancies
DEPENDANCIES=libfat-readonly
LIBS=$(foreach x, $(DEPENDANCIES), $(ROOT)libs/$(x)/$(x)$(ENV).o)
TARGET=loader.bin

build: deps $(TARGET)

$(TARGET): obj/entry.o $(OBJECTS)
	$(LD) -e start -Ttext 8000 -N -nostdlib --oformat binary -o $@ obj/entry.o obj/*/*.o $(LIBS)

obj/entry.o: src/$(ENTRY)  | obj
	$(NASM) -f elf -o $@ $<

obj/asm/%.o: src/%.asm | obj/asm
	$(NASM) -f elf -o $@ $<

obj/c/%.o: src/%.c | obj/c
	$(CC) -c $(CFLAGS) -o $@ $<
	@#objcopy -j .text -O binary $@ $@.bin

obj obj/c obj/asm:
	$(MKDIR) $@

deps:
	@$(foreach x,$(LIBS),$(MAKE) --no-print-directory -C $(dir $(x)) ENV=$(ENV))

$(LIBS):
	@$(MAKE) --no-print-directory -C $(dir $@) ENV=$(ENV)

clean:
	$(RM) $(TARGET) obj/entry.o $(OBJECTS) obj

clean-all: clean
	@$(foreach x,$(LIBS),$(MAKE) --no-print-directory -C $(dir $(x)) clean-all)

rebuild: clean $(TARGET)

.PHONY: build deps  clean clean-all rebuild objects
