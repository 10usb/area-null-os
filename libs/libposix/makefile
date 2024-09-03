include ../../env.posix.mk
SOURCES=blockstream.c
OBJECTS=$(SOURCES:%.c=obj/%.o)
TARGET=libposix.o

$(TARGET): $(OBJECTS)
	$(LD) -i -o $@ $(OBJECTS)

obj/%.o: src/%.c | obj
	$(CC) -c -I$(INCLUDES) -o $@ $<

obj:
	$(MKDIR) $@

clean:
	$(RM) $(TARGET) $(OBJECTS) obj

.PHONY: clean
