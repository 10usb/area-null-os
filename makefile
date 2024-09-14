include env.mk
FLOPPY=floppy.img
BOOT=boot/fatboot.bin
LOADER=loader/loader.bin
FAT=tools/fat/fat$(SUFFIX)

build: deps $(FLOPPY)

$(FLOPPY): $(BOOT) $(LOADER) $(FAT) $(FLOPPY).info
	$(FAT) store $(FLOPPY) 0:0 $(BOOT)
	$(FAT) store $(FLOPPY) 1:19 $(LOADER)

$(FLOPPY).info: $(FAT)
	$(FAT) create $(FLOPPY) 2880 -T 18 -H 2 -r 20 > $(FLOPPY).info

deps:
	@$(MAKE) --no-print-directory -C boot
	@$(MAKE) --no-print-directory -C loader

$(FAT):
	@$(MAKE) --no-print-directory -C tools/fat

clean:
	$(RM) $(FLOPPY)
	@$(MAKE) --no-print-directory -C boot clean
	@$(MAKE) --no-print-directory -C loader clean

rebuild: clean $(FLOPPY)

run: deps $(FLOPPY)
	$(QEMU) -drive format=raw,file="$(FLOPPY)",index=0,if=floppy, -m 128M &

.PHONY: build deps clean rebuild run test x y
