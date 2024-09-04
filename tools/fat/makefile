include ../../env.posix.mk
SOURCES=main.c
OBJECTS=$(SOURCES:%.c=obj/%.o)
DEPENDANCIES=libfat-readonly
LIBS=$(foreach x, $(DEPENDANCIES), $(ROOT)libs/$(x)/$(x).posix.o)

POSIX_DEPENDANCIES=libposix-adapter
POSIX_LIBS=$(foreach x, $(POSIX_DEPENDANCIES), $(ROOT)libs/$(x)/$(x).o)
TARGET=fat$(SUFFIX)

$(TARGET): $(OBJECTS) $(LIBS) $(POSIX_LIBS)
	$(CC) -Wall -o $@ $(OBJECTS) $(LIBS) $(POSIX_LIBS)

obj/%.o: src/%.c | obj
	$(CC) -c -I$(INCLUDES) -o $@ $<

obj:
	$(MKDIR) $@

$(LIBS):
	@$(MAKE) --no-print-directory -C $(dir $@) ENV=.posix

$(POSIX_LIBS):
	@$(MAKE) --no-print-directory -C $(dir $@)

clean:
	$(RM) $(TARGET) $(OBJECTS) obj

.PHONY: clean dependencies $(LIBS)
