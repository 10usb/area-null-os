include ../../env.posix.mk
SOURCES=main.c
OBJECTS=$(SOURCES:%.c=obj/c/%.o)
# Shared dependancies
DEPENDANCIES=libfat-readonly
LIBS=$(foreach x, $(DEPENDANCIES), $(ROOT)libs/$(x)/$(x).posix.o)
# Local dependancies
POSIX_DEPENDANCIES=libposix-adapter
POSIX_LIBS=$(foreach x, $(POSIX_DEPENDANCIES), $(ROOT)libs/$(x)/$(x).o)
TARGET=fat$(SUFFIX)

$(TARGET): $(OBJECTS) | deps
	$(CC) -Wall -o $@ $(OBJECTS) $(LIBS) $(POSIX_LIBS)

obj/c/%.o: src/%.c | obj/c
	$(CC) -c -I$(INCLUDES) -o $@ $<

obj/c:
	$(MKDIR) $@

deps: | $(LIBS) $(POSIX_LIBS)

$(LIBS):
	@$(MAKE) --no-print-directory -C $(dir $@) ENV=.posix

$(POSIX_LIBS):
	@$(MAKE) --no-print-directory -C $(dir $@)

clean:
	$(RM) $(TARGET) $(OBJECTS) obj

clean-all: clean
	@$(foreach x,$(LIBS),$(MAKE) --no-print-directory -C $(dir $(x)) clean)
	@$(foreach x,$(POSIX_LIBS),$(MAKE) --no-print-directory -C $(dir $(x)) clean)

.PHONY: clean clean-all deps $(LIBS) $(POSIX_LIBS)
