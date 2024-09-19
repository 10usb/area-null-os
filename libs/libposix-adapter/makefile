include ../../env.posix.mk
SOURCES=blockstream.c
OBJECTS=$(SOURCES:%.c=obj/%.o)
TARGET=libposix-adapter.o

$(TARGET): $(OBJECTS)
	$(LD) -i -o $@ $(OBJECTS)

obj/%.o: src/%.c | obj
	$(CC) -c $(CFLAGS) -o $@ $<

obj:
	$(MKDIR) $@

clean:
	$(RM) $(TARGET) $(OBJECTS) obj

clean-all: clean

.PHONY: clean clean-all
