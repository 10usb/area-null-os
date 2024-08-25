include env.mk
TARGET=floppy.img
BOOT=boot/fatboot.bin
LOADER=loader/loader.bin

NumberOfHeads=2
TrackPerHead=80
SectorsPerTrack=18
BytesPerSector=512

ReservedSectors=20

IMAGE_SIZE=$(shell echo $$(($(NumberOfHeads) * $(SectorsPerTrack) * $(TrackPerHead) * $(BytesPerSector))))

build: deps $(TARGET)

$(TARGET): $(BOOT) $(LOADER) 
	truncate -s $(IMAGE_SIZE) $(TARGET)
	dd if=$(BOOT) of=$(TARGET) bs=512 count=1 conv=notrunc 
	printf %02x $(ReservedSectors) | xxd -r -p | dd of=$(TARGET) bs=1 seek=14 count=1 conv=notrunc status=none
	printf %02x $(SectorsPerTrack) | xxd -r -p | dd of=$(TARGET) bs=1 seek=24 count=1 conv=notrunc status=none
	printf %02x $(NumberOfHeads) | xxd -r -p | dd of=$(TARGET) bs=1 seek=26 count=1 conv=notrunc status=none
	dd if=$(LOADER) of=$(TARGET) bs=512 seek=1 count=19 conv=notrunc 

deps:
	@$(MAKE) --no-print-directory -C boot
	@$(MAKE) --no-print-directory -C loader

clean:
	$(RM) $(TARGET)

rebuild: clean $(TARGET)

run: deps $(TARGET)
	$(QEMU) -drive format=raw,file="$(TARGET)",index=0,if=floppy, -m 128M &

test:
	echo $(IMAGE_SIZE)

.PHONY: build deps clean rebuild run test
