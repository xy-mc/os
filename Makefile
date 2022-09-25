##################################################
# Makefile
##################################################

BOOT:=boot.asm
LDR:=loader.asm
BOOT_BIN:=$(subst .asm,.bin,$(BOOT))
LDR_BIN:=$(subst .asm,.bin,$(LDR))

.PHONY : everything

everything : $(BOOT_BIN) $(LDR_BIN)
	@dd if=/dev/zero of=a.img bs=512 count=2880
	@mkfs -t vfat a.img
	@dd if=$(BOOT_BIN) of=a.img bs=512 count=1 conv=notrunc
	@sudo mount -o loop a.img /mnt
	@sudo cp $(LDR_BIN) /mnt -v
	@sudo umount /mnt

clean :
	@rm -f $(BOOT_BIN) $(LDR_BIN)

$(BOOT_BIN) : $(BOOT)
	@nasm $< -o $@

$(LDR_BIN) : $(LDR)
	@nasm $< -o $@


run:
	@qemu-system-i386		\
	-boot order=c			\
	-drive file=a.img,format=raw	\

gdb:
	@qemu-system-i386		\
	-boot order=c			\
	-drive file=a.img,format=raw	\
	-S -s

monitor:
	@gdb				\
	-ex 'set tdesc filename target.xml' \
	-ex 'target remote localhost:1234'

