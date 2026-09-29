#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "keyboard.h"

//Keyboard suport :D

extern void readInput();
extern size_t terminal_row;
extern size_t terminal_column;
extern void printk(const char* data);
extern void terminal_putchar(char c);
extern char inputBuffer[128];
extern int textPointer;

static const char scancode_to_ascii_lowercase[128] = {
    0,    27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,    '\\','z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*',  0,   ' '
};

static const char scancode_to_ascii_uppercase[128] = {
    0,    27,  '!', '@', '#', '$', '%', '"', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', ' ',
    0,    '|','Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*',  0,   ' '
};

extern uint8_t inb(uint16_t port);

uint8_t keyboard_get_scancode(void)
{
    return inb(0x60);
}

inline uint8_t keyboard_status(void)
{
    return inb(0x64);
}

bool keyboard_has_input(void)
{
    return keyboard_status() & 1;
}

bool shift_pressed;

bool keyboard_poll(key_event_t *event)
{
    event->type = KEY_NONE;
    event->character = 0;

    if (!keyboard_has_input())
        return false;

    uint8_t scancode = keyboard_get_scancode();

    // Shift pressionado
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = true;
        return false;
    }

    // Shift solto
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = false;
        return false;
    }

    // Tecla solta
    if (scancode & 0x80)
        return false;

    if (scancode >= 128)
        return false;

    char c;

    if (shift_pressed)
        c = scancode_to_ascii_uppercase[scancode];
    else
        c = scancode_to_ascii_lowercase[scancode];

    if (c == '\n') {
        event->type = KEY_ENTER;
        return true;
    }

    if (c == '\b') {
        event->type = KEY_BACKSPACE;
        return true;
    }

    if (c != 0) {
        event->type = KEY_CHAR;
        event->character = c;
        return true;
    }

    return false;
}

