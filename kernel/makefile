include ../env.mk
SOURCES=main.c
OBJECTS=$(patsubst %.asm,obj/asm/%.o,$(patsubst %.c,obj/c/%.o,$(SOURCES)))
# Shared dependancies
DEPENDANCIES=libfat
#DEPENDANCIES=
LIBS=$(foreach x, $(DEPENDANCIES), $(ROOT)libs/$(x)/$(x)$(ENV).o)
TARGET=kernel.sys

build: deps $(TARGET)

$(TARGET): $(OBJECTS)
	@#$(LD) -T kernel.ld -N -nostdlib -s -o $@ obj/*/*.o $(LIBS)
	@#ld -T kernel.ld -N -nostdlib -s -m i386pe --oformat pei-i386 -o $@ obj/*/*.o $(LIBS)
	$(LD) -T kernel.ld -N -nostdlib -s --oformat coff-i386 -o $@ obj/*/*.o $(LIBS)
	@#ld -T kernel.ld -N -nostdlib -s -m i386pe --oformat pe-i386 -o $@ obj/*/*.o $(LIBS)

obj/asm/%.o: src/%.asm | obj/asm
	$(NASM) -f elf -o $@ $<

obj/c/%.o: src/%.c | obj/c
	$(CC) -c $(CFLAGS) -o $@ $<

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
