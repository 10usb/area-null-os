include ../env.mk
SOURCES=src/main.asm
TARGET=fatboot.bin

$(TARGET): $(SOURCES)
	$(FASM) src/main.asm $@

clean:
	$(RM) $(TARGET)

.PHONY: clean
