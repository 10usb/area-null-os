include ../env.mk
ENTRY=start.asm
SOURCES=main.c tty.c text.c isr.c isr.asm irq.c
OBJECTS=$(patsubst %.asm,obj/asm/%.o,$(patsubst %.c,obj/c/%.o,$(SOURCES)))
TARGET=loader.bin

$(TARGET): obj/entry.o $(OBJECTS)
	$(LD) -e start -Ttext 8000 -N -nostdlib --oformat binary -o $@ obj/entry.o obj/*/*.o

obj/entry.o: src/$(ENTRY)
	$(NASM) -f elf -o $@ $<

obj/asm/%.o: src/%.asm
	$(NASM) -f elf -o $@ $<

obj/c/%.o: src/%.c
	$(CC) -c -m32 -ffreestanding -nostdlib -o $@ $<

clean:
	$(RM) $(TARGET) obj/entry.o $(OBJECTS)

rebuild: clean $(TARGET)

objects:
	@echo $(OBJECTS)

.PHONY: clean rebuild objects
