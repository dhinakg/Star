/*
 * File: kprint.c
 * 
 * Copyright (c) 2017-2018 Sydney Erickson, John Davis
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <main.h>
#include <kprint.h>
#include <driver/serial.h>
#include <driver/vga.h>
#include <kernel/lock.h>
#include <string.h>

lock_t kprintf_mutex = { };

// Print a single character.
void kputchar(char c)
{
    // Print the character to both the screen and the serial.
    serial_write(c);
    vga_putchar(c);
}

// Print a hexadecimal byte.
void kputchar_hex(uint8_t num, bool capital, bool pad)
{
    // Get high and low nibbles.
    const uint8_t high = (num & 0xF0) >> 4;
    const uint8_t low = num & 0x0F;

    // Our hexadecimal ASCII dictionary.
    const char* hex_chars = capital ? "0123456789ABCDEF" : "0123456789abcdef";

    // Print the hex number.
    if (high != 0 || pad)
        kputchar(hex_chars[high]);
    kputchar(hex_chars[low]);
}

// Print a string.
void kputstring(const char *str, size_t max)
{
    // Print out string.
    if (max == 0) {
        while (*str) {
            kputchar(*str);
            str++;
        }
    }
    else {
        while (*str && max) {
            kputchar(*str);
            str++;
            max--;
        }
    }
}

// Print an integer.
void kprint_int(int64_t num)
{
    // If zero, just print zero.
    if (num == 0)
    {
        kputchar('0');
        return;
    }

    // Determine if negative and get absolute value.
    bool negative = num < 0;
    num = negative ? -num : num;

    // Create buffer.
    char buffer[1 + 20 + 1]; // Sign, maximum size of 64-bit int, null terminator.
    buffer[sizeof(buffer) - 1] = '\0';

    // Walk through the number backwards.
    int32_t i = sizeof(buffer) - 2;
    while (num > 0)
    {
        buffer[i--] = '0' + (num % 10);
        num /= 10;
    }

    // Add sign if negative.
    if (negative)
        buffer[i--] = '-';

    // Print out numeral.
    kputstring(&buffer[i + 1], 0);
}

// Print an unsigned integer.
void kprint_uint(uint64_t num)
{
    // If zero, just print zero.
    if (num == 0)
    {
        kputchar('0');
        return;
    }

    // Create buffer.
    char buffer[20 + 1]; // Maximum size of unsigned 64-bit int, null terminator.
    buffer[sizeof(buffer) - 1] = '\0';

    // Walk through the number backwards.
    int32_t i = sizeof(buffer) - 2;
    while (num > 0)
    {
        buffer[i--] = '0' + (num % 10);
        num /= 10;
    }

    // Print out numeral.
    kputstring(&buffer[i + 1], 0);
}

// Print unsigned int as hexadecimal.
void kprint_hex(uint64_t num, bool capital, uint8_t width) {
    bool first = true;
    for (int8_t i = sizeof(num) - 1; i >= 0; i--) {
        // Get byte.
        const uint8_t byte = (num >> (8 * i)) & 0xFF;

        // Are we wanting to ouput a certain width?
        if (width && (i * 2) >= width) {
            continue;
        }
        else if (width == 0) { // No width specified
            // If we have yet to hit the first non-zero byte, just continue to the next one.
            if (first && byte == 0 && i > 0)
                continue;
        }

        // Print hex byte.
        kputchar_hex(byte, capital, !first ? true : width > 0);
        first = false;
        width -= 2;
    }
}

static void kprintf_sgr(const char *sequence, uint32_t length) {
    // If the length is 0, reset params.
    if (length == 0) {
        vga_setcolor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        return;
    }

    // Get parameters until we reach the end.
    uint32_t currentPos = 0;
    while (currentPos < length) {
        // Search until we find a number.
        while ((sequence[currentPos] < '0' || sequence[currentPos] > '9') && currentPos < length)
            currentPos++;

        // If we have reached the end, break out.
        if (currentPos >= length)
            break;

        // Get parameter.
        uint32_t param = 0;
		for (; currentPos < length && sequence[currentPos] >= '0' && sequence[currentPos] <= '9'; currentPos++)
			param = param * 10 + (sequence[currentPos] - '0');

        // Process parameter.
        switch (param) {
            case 0:
                vga_setcolor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
                break;

            // Foreground colors.
            case 30:
                vga_setcolor_fg(VGA_COLOR_BLACK);
                break;

            case 31:
                vga_setcolor_fg(VGA_COLOR_RED);
                break;

            case 32:
                vga_setcolor_fg(VGA_COLOR_GREEN);
                break;

            case 33:
                vga_setcolor_fg(VGA_COLOR_BROWN);
                break;

            case 34:
                vga_setcolor_fg(VGA_COLOR_BLUE);
                break;

            case 35:
                vga_setcolor_fg(VGA_COLOR_MAGENTA);
                break;

            case 36:
                vga_setcolor_fg(VGA_COLOR_CYAN);
                break;

            case 37:
                vga_setcolor_fg(VGA_COLOR_LIGHT_GREY);
                break;

            // Background colors.
            case 40:
                vga_setcolor_bg(VGA_COLOR_BLACK);
                break;

            case 41:
                vga_setcolor_bg(VGA_COLOR_RED);
                break;

            case 42:
                vga_setcolor_bg(VGA_COLOR_GREEN);
                break;

            case 43:
                vga_setcolor_bg(VGA_COLOR_BROWN);
                break;

            case 44:
                vga_setcolor_bg(VGA_COLOR_BLUE);
                break;

            case 45:
                vga_setcolor_bg(VGA_COLOR_MAGENTA);
                break;

            case 46:
                vga_setcolor_bg(VGA_COLOR_CYAN);
                break;

            case 47:
                vga_setcolor_bg(VGA_COLOR_LIGHT_GREY);
                break;

            // Bright foreground colors.
            case 90:
                vga_setcolor_fg(VGA_COLOR_DARK_GREY);
                break;

            case 91:
                vga_setcolor_fg(VGA_COLOR_LIGHT_RED);
                break;

            case 92:
                vga_setcolor_fg(VGA_COLOR_LIGHT_GREEN);
                break;

            case 93:
                vga_setcolor_fg(VGA_COLOR_LIGHT_BROWN);
                break;

            case 94:
                vga_setcolor_fg(VGA_COLOR_LIGHT_BLUE);
                break;

            case 95:
                vga_setcolor_fg(VGA_COLOR_LIGHT_MAGENTA);
                break;

            case 96:
                vga_setcolor_fg(VGA_COLOR_LIGHT_CYAN);
                break;

            case 97:
                vga_setcolor_fg(VGA_COLOR_WHITE);
                break;

            // Bright background colors.
            case 100:
                vga_setcolor_bg(VGA_COLOR_DARK_GREY);
                break;

            case 101:
                vga_setcolor_bg(VGA_COLOR_LIGHT_RED);
                break;

            case 102:
                vga_setcolor_bg(VGA_COLOR_LIGHT_GREEN);
                break;

            case 103:
                vga_setcolor_bg(VGA_COLOR_LIGHT_BROWN);
                break;

            case 104:
                vga_setcolor_bg(VGA_COLOR_LIGHT_BLUE);
                break;

            case 105:
                vga_setcolor_bg(VGA_COLOR_LIGHT_MAGENTA);
                break;

            case 106:
                vga_setcolor_bg(VGA_COLOR_LIGHT_CYAN);
                break;

            case 107:
                vga_setcolor_bg(VGA_COLOR_WHITE);
                break;
        }
    }
}

void kprintf(const char* format, ...) {
    // Get args.
    va_list args;
    va_start(args, format);

    // Call va_list kprintf.
    kprintf_va(true, format, args);
}

void kprintf_nolock(const char* format, ...) {
    // Get args.
    va_list args;
    va_start(args, format);

    // Call va_list kprintf.
    kprintf_va(false, format, args);
}

// https://en.wikipedia.org/wiki/Printf_format_string
// Printf implementation.
void kprintf_va(bool lock, const char* format, va_list args) {
    // Lock.
    if (lock)
        spinlock_lock(&kprintf_mutex);

    // Disable cursor for increased performance.
    vga_disable_cursor();

    // Iterate through format string.
    char c;
    while (*format) {
        // Get current character.
        c = *format++;

        // Do we have the start of a variable?
        if (c == '%') {
            // Get type of formatting.
            char f = *format++;

            // Ignore justify for now.
            if (f == '-')
                f = *format++;

            // Ignore 0.
            if (f == '0')
                f = *format++;

            // Check width.
            size_t width = 0;
            size_t precision = 0;
            if (f >= '0' && f <= '9') {
                width += f - '0';
                f = *format++;
            }

            // Ignore justify for now.
            if (f == '.') {
                f = *format++;

                // Check width.
                if (f >= '0' && f <= '9') {
                    precision += f - '0';
                    f = *format++;
                }
            }



            // Do we have a long long?
            if (f == 'l' && *format++ == 'l') {
                // Handle 32-bit integers.
                switch (*format++) {
                    // If we have a null, skip over.
                    case '\0':
                        break;
                    
                    // If we have a %, print a literal %.
                    case '%':
                        kputchar('%');
                        break;

                    // Print integer.
                    case 'd':
                    case 'i':
                        kprint_int((int64_t)va_arg(args, int64_t));
                        break;

                    // Print unsigned integer.
                    case 'u':
                        kprint_uint((uint64_t)va_arg(args, uint64_t));
                        break;

                    // Print floating point.
                    case 'f':
                    case 'F':
                        // TODO.
                        break;
                    
                    // Print hexadecimal.
                    case 'x':
                        kprint_hex((uint64_t)va_arg(args, uint64_t), false, width);
                        break;

                    // Print hexadecimal (uppercase).
                    case 'X':
                        kprint_hex((uint64_t)va_arg(args, uint64_t), true, width);
                        break;

                    // Print string.
                    case 's':
                        kputstring((const char*)va_arg(args, const char*), width);
                        break;

                    // Print character.
                    case 'c':
                        kputchar((char)va_arg(args, int32_t));
                        break;
                }
            }
            else {
                // Handle 32-bit integers.
                switch (f) {
                    // If we have a null, skip over.
                    case '\0':
                        break;
                    
                    // If we have a %, print a literal %.
                    case '%':
                        kputchar('%');
                        break;

                    // Print integer.
                    case 'd':
                    case 'i':
                        kprint_int((int32_t)va_arg(args, int32_t));
                        break;

                    // Print unsigned integer.
                    case 'u':
                        kprint_uint((uint32_t)va_arg(args, uint32_t));
                        break;

                    // Print floating point.
                    case 'f':
                    case 'F':
                        // TODO.
                        break;
                    
                    // Print hexadecimal.
                    case 'x':
                        kprint_hex((uint32_t)va_arg(args, uint32_t), false, width);
                        break;

                    // Print hexadecimal (uppercase).
                    case 'X':
                        kprint_hex((uint32_t)va_arg(args, uint32_t), true, width);
                        break;

                    // Print pointer as hex.
                    case 'p':
                    case 'P':
                        kprint_hex((uintptr_t)va_arg(args, uintptr_t), true, false);
                        break;

                    // Print string.
                    case 's':
                        kputstring((const char*)va_arg(args, const char*), width);
                        break;

                    // Print character.
                    case 'c':
                        kputchar((char)va_arg(args, int32_t));
                        break;
                }
            }       
        }
        else if (c == '\033') {
            // Escape sequence.
            // Get current character.
            serial_write(c);
            c = *format++;
            serial_write(c);

            // Check if CSI (control sequence introducer).
            if (c == '[') {
                // Get length of sequence. A byte in the range of 0x40–0x7E signals the end.
                bool found = false;
                char *sequence = format;
                uint32_t formatEnd = strlen(sequence);
			    for (uint16_t i = 0; i < formatEnd; i++) {
                    if (sequence[i] >= 0x40 && sequence[i] <= 0x7E) {
                        formatEnd = i;
                        found = true;
                        break;
				    }
			    }

                // Check if we even found the end.
                if (found) {
                    // Print to serial.
                    for (uint32_t i = 0; i <= formatEnd; i++)
                        serial_write(format[i]);

                    // Determine type of sequence (end char).
                    char seqTypeChar = sequence[formatEnd];
                    switch (seqTypeChar) {
                        case 'm':
                            kprintf_sgr(sequence, formatEnd);
                            format += formatEnd + 1;
                            break;
                    }
                }
            }
        }
        else
        {
            // Print any other characters.
            kputchar(c);
        }
    }

    // Re-enable the console driver
    vga_enable_cursor();

    // Release lock.
    if (lock)
        spinlock_release(&kprintf_mutex);
}
