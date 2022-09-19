boot.asm:boot.bin
	nasm boot.asm -o boot.bin
clean:
	rm boot.bin
run:
	qemu-system-i386                \
-boot order=c                   \
-drive file=boot.bin,format=raw
