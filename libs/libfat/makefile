include ../../env$(ENV).mk
SOURCES=creation.c
OBJECTS=$(SOURCES:%.c=obj/$(ENVDIR)/%.o)
# Shared dependancies
DEPENDANCIES=libfat-readonly
LIBS=$(foreach x, $(DEPENDANCIES), $(ROOT)libs/$(x)/$(x)$(ENV).o)
TARGET=libfat$(ENV).o

build: deps $(TARGET)

$(TARGET): $(OBJECTS) $(LIBS)
	$(LD) -i -o $@ $(OBJECTS) $(LIBS)

obj/$(ENVDIR)/%.o: src/%.c | obj/$(ENVDIR)
	$(CC) -c -I$(INCLUDES) -o $@ $<

obj/$(ENVDIR):
	$(MKDIR) $@

deps:
	@$(foreach x,$(LIBS),$(MAKE) --no-print-directory -C $(dir $(x)) ENV=$(ENV))

$(LIBS):
	@$(MAKE) --no-print-directory -C $(dir $@) ENV=$(ENV)

clean:
	$(RM) $(TARGET) $(OBJECTS) obj

clean-all: clean
	@$(foreach x,$(LIBS),$(MAKE) --no-print-directory -C $(dir $(x)) clean-all)

.PHONY: build deps clean clean-all
