include ../../env$(ENV).mk
SOURCES=readonly.c
OBJECTS=$(SOURCES:%.c=obj/$(ENVDIR)/%.o)
TARGET=libfat-readonly$(ENV).o

$(TARGET): $(OBJECTS)
	$(LD) -i -o $@ $(OBJECTS)

obj/$(ENVDIR)/%.o: src/%.c | obj/$(ENVDIR)
	$(CC) -c $(CFLAGS) -o $@ $<

obj/$(ENVDIR):
	$(MKDIR) $@

clean:
	$(RM) $(TARGET) $(OBJECTS) obj

clean-all: clean

.PHONY: clean clean-all
