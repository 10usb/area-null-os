include ../../../env.mk
SOURCES=a.c d.c
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

build: output.rel output.abs

output.rel: $(OBJECTS)
	$(LD) -r -T linker.ld -N -nostdlib -o $@ $(OBJECTS)

output.abs: $(OBJECTS)
	$(LD) --emit-relocs -T linker.ld -N -nostdlib -o $@ $(OBJECTS)

mangle.out: $(OBJECTS)  mangle.o
	$(LD) -r -T linker.ld -N -nostdlib -o $@ a.o mangle.o

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	$(RM) $(OBJECTS)

.PHONY: build clean
