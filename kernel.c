#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "keyboard.h"
#include "basic.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

char inputBuffer[128];
int textPointer = 0;

void readInput();
static void terminal_scroll();

extern bool close;

uint8_t inb(uint16_t port)
{
    uint8_t ret;

    __asm__ volatile (
        "inb %1, %0"
        : "=a"(ret)
        : "Nd"(port)
    );

    return ret;
}

/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000

char consoleBuffer[VGA_HEIGHT][VGA_WIDTH];

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;

void terminal_initialize(int VGA_COLOR_TEXT, int VGA_COLOR_BACKGROUND)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_TEXT, VGA_COLOR_BACKGROUND);
	
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c)
{
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;

        if (terminal_row >= VGA_HEIGHT)
            terminal_scroll();

        return;
    }

    terminal_putentryat(
        c,
        terminal_color,
        terminal_column,
        terminal_row
    );

    terminal_column++;

    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;

        if (terminal_row >= VGA_HEIGHT)
            terminal_scroll();
    }
}

void terminal_write(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void printk(const char* data) 
{
	terminal_write(data, strlen(data));
}

int strcmp_diferenciados(const char *p1, const char *p2, const int chars){
    for(int i = 0; i <= chars-1; i++){
        if(p1[i] != p2[i]){
            return 1;
        }
    }
    return 0;
}

void intToString(int number, char *buffer) {
    int i = 0;
    bool negative = false;

    if (number < 0) {
        negative = true;
        number = -number;
    }

    do {
        buffer[i++] = '0' + (number % 10);
        number /= 10;
    } while (number > 0);

    if (negative) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0';

    // Inverte a string
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}

int strcmp(const char *p1, const char *p2)
{
    const unsigned char *s1 = (const unsigned char *)p1;
    const unsigned char *s2 = (const unsigned char *)p2;

    unsigned char c1, c2;

    do {
        c1 = *s1++;
        c2 = *s2++;

        if (c1 == '\0')
            return (int)c1 - (int)c2;

    } while (c1 == c2);

    return (int)c1 - (int)c2;
}

void readInput(){
    printk("\n");

    if(inputBuffer[0] == '\0'){
        printk("> ");
        return;
    }

	if(strcmp(inputBuffer, "info") == 0){
		printk("AND YOU DONT SEEM TO UNDERSAND!\n");
    } else if(strcmp(inputBuffer, "shutdown") == 0){
        __asm__ volatile (
            ".intel_syntax noprefix\n\t"
            "mov dx, 0x604\n\t"
            "mov ax, 0x2000\n\t"
            "out dx, ax\n\t"
            ".att_syntax prefix"
            : : : "dx", "ax"
        );
    } else if (strcmp(inputBuffer, "basic") == 0) {
        basicShell();
    } else {
		printk("Comando invalido!\n");
	}

	textPointer = 0;
	inputBuffer[0] = '\0';

	printk("> ");
}

static void terminal_scroll(void)
{
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[(y - 1) * VGA_WIDTH + x] =
                terminal_buffer[y * VGA_WIDTH + x];
        }
    }

    // Limpa a última linha
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] =
            vga_entry(' ', terminal_color);
    }

    terminal_row = VGA_HEIGHT - 1;
}

void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize(15, 0);

    printk("NNK first compilation 0.0.1. Arthur A\n");

	printk("> ");

	while (true) {
        if(close == true){
            printk("Back to the shell\n");
            close = false;
            printk("> ");
        }

        key_event_t event;

        if (keyboard_poll(&event)) {

            if (event.type == KEY_CHAR) {
                terminal_putchar(event.character);
                inputBuffer[textPointer] = event.character;
                textPointer++;
                inputBuffer[textPointer] = '\0';
            }

            if (event.type == KEY_ENTER) {
                readInput();
            }

            if (event.type == KEY_BACKSPACE) {
                if(terminal_column > 2) {
                    terminal_column--;
                    printk(" ");
                    terminal_column--;
                    textPointer--;
                    inputBuffer[textPointer] = '\0';
                }
            }
        }

	}

}
