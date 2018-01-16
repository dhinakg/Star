#include "main.h"
#include "vga.h"
#include "io.h"

void floppy_detect() {
	unsigned char c;
	outb(0x70, 0x10);
	c = inb(0x71);

	a = c >> 4; // Get high nibble
	b = c & 0xF; // Get low nibble by ANDing out low nibble

	char *drive_type[5] = { "no floppy drive", "360kb 5.25in floppy drive", "1.2mb 5.25in floppy drive", "720kb 3.5in", "1.44mb 3.5in", "2.88mb 3.5in"};
	vga_writes("Floppy drive A is an:\n");
	vga_writes(drive_type[a]);
	vga_writes("\nFloppy drive B is an:\n");
	vga_writes(drive_type[b]);
	vga_writes("\n");
}