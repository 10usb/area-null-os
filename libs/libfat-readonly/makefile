include ../../env$(ENV).mk
SOURCES=fat.c
OBJECTS=$(SOURCES:%.c=obj/$(ENVDIR)/%.o)
TARGET=libfat-readonly$(ENV).o

$(TARGET): $(OBJECTS)
	$(LD) -i -o $@ $(OBJECTS)

obj/$(ENVDIR)/%.o: src/%.c | obj/$(ENVDIR)
	$(CC) -c -I$(INCLUDES) -o $@ $<

obj/$(ENVDIR):
	$(MKDIR) $@

clean:
	$(RM) $(TARGET) $(OBJECTS) obj

.PHONY: clean
