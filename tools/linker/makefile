include ../../env.posix.mk
SOURCES=main.c elf.c binary.c
OBJECTS=$(SOURCES:%.c=obj/c/%.o)
# Shared dependancies
DEPENDANCIES=
LIBS=$(foreach x, $(DEPENDANCIES), $(ROOT)libs/$(x)/$(x).posix.o)
# Local dependancies
POSIX_DEPENDANCIES=
POSIX_LIBS=$(foreach x, $(POSIX_DEPENDANCIES), $(ROOT)libs/$(x)/$(x).o)
TARGET=linker$(SUFFIX)

build: deps $(TARGET)

$(TARGET): $(OBJECTS) $(LIBS) $(POSIX_LIBS)
	$(CC) -Wall -o $@ $(OBJECTS) $(LIBS) $(POSIX_LIBS)

obj/c/%.o: src/%.c | obj/c
	$(CC) -c -I$(INCLUDES) -o $@ $<

obj/c:
	$(MKDIR) $@

deps:
	@$(foreach x,$(LIBS),$(MAKE) --no-print-directory -C $(dir $(x)) ENV=.posix)
	@$(foreach x,$(POSIX_LIBS),$(MAKE) --no-print-directory -C $(dir $(x)))

$(LIBS):
	@$(MAKE) --no-print-directory -C $(dir $@) ENV=.posix

$(POSIX_LIBS):
	@$(MAKE) --no-print-directory -C $(dir $@)

clean:
	$(RM) $(TARGET) $(OBJECTS) obj

clean-all: clean
	@$(foreach x,$(LIBS),$(MAKE) --no-print-directory -C $(dir $(x)) clean-all)
	@$(foreach x,$(POSIX_LIBS),$(MAKE) --no-print-directory -C $(dir $(x)) clean-all)

.PHONY: build deps clean clean-all
