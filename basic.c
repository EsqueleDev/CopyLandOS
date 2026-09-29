#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "keyboard.h"

extern void terminal_initialize(int VGA_COLOR_TEXT, int VGA_COLOR_BACKGROUND);
extern void printk(const char* data);
extern void terminal_putchar(char c);
extern char inputBuffer[128];
extern int textPointer;
extern int terminal_column;
extern int strcmp_diferenciados(const char *p1, const char *p2, const int chars);
extern void kernel_main(void);

#define MAX_VARIABLES 32

typedef struct {
    char name;
    int value;
    bool used;
} Variable;

Variable variables[MAX_VARIABLES];

extern void intToString(int number, char *buffer);

bool close = false;

void runBasicCommand(char* command);

void basicShell() {
    terminal_initialize(13, 5);

    textPointer = 0;
    inputBuffer[0] = '\0';

    printk("NNK BASIC\n");
    printk("> ");

    while (true) {
        key_event_t event;

        if (!keyboard_poll(&event))
            continue;

        if (event.type == KEY_CHAR) {

            if (textPointer < 127) {
                terminal_putchar(event.character);

                inputBuffer[textPointer] = event.character;
                textPointer++;

                inputBuffer[textPointer] = '\0';
            }
        }

        else if (event.type == KEY_ENTER) {

            runBasicCommand(inputBuffer);
        }

        else if (event.type == KEY_BACKSPACE) {

            if (textPointer > 0) {

                textPointer--;
                inputBuffer[textPointer] = '\0';

                terminal_column--;
                terminal_putchar(' ');
                terminal_column--;
            }
        }
        if(close == true){
            terminal_initialize(15, 0);
            return;
        }
    }
}

int findVariable(char name) {
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].used && variables[i].name == name) {
            return i;
        }
    }

    return -1;
}

void setVariable(char name, int value) {

    int index = findVariable(name);

    // Variável já existe
    if (index != -1) {
        variables[index].value = value;
        return;
    }

    // Procura espaço para uma nova variável
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (!variables[i].used) {
            variables[i].used = true;
            variables[i].name = name;
            variables[i].value = value;
            return;
        }
    }

    printk("VARIABLE ERROR!\n");
}

bool getVariable(char name, int *value) {

    int index = findVariable(name);

    if (index == -1) {
        return false;
    }

    *value = variables[index].value;
    return true;
}

void runBasicCommand(char* command) {

    printk("\n");

    if(strcmp_diferenciados(command, "EXIT", 4) == 0){
        close = true;
        return;
    }

    if (strcmp_diferenciados(command, "LET", 3) == 0) {
        //LIVE AND LET DIEEEEEEEEEEEEEEE
        char variable = command[4];

        if (variable < 'A' || variable > 'Z') {
            printk("SINTAX ERROR!\n");
            printk("> ");
            textPointer = 0;
            inputBuffer[0] = '\0';
            return;
        }

        if (command[5] != ' ' || command[6] != '=') {
            printk("SINTAX ERROR!\n");
            printk("> ");
            textPointer = 0;
            inputBuffer[0] = '\0';
            return;
        }

        int value = 0;
        int i = 8;

        while (command[i] >= '0' && command[i] <= '9') {
            value = value * 10 + (command[i] - '0');
            i++;
        }

        if (command[i] != '\0') {
            printk("SINTAX ERROR!\n");
            printk("> ");
            textPointer = 0;
            inputBuffer[0] = '\0';
            return;
        }

        setVariable(variable, value);
        printk("OK!\n> ");
        textPointer = 0;
        inputBuffer[0] = '\0';
        return;
    } else if (strcmp_diferenciados(command, "PRINT", 5) == 0) {

        if (command[5] == ' ' && command[6] == '"') {

            int i = 7;
            bool foundClosingQuote = false;

            while (command[i] != '\0') {

                if (command[i] == '"') {
                    foundClosingQuote = true;
                    break;
                }

                terminal_putchar(command[i]);
                i++;
            }

            if (!foundClosingQuote) {
                printk("\nSINTAX ERROR!\n");
            } else {
                printk("\n");
            }
        }

        else if (command[5] == ' ' &&
            command[6] >= 'A' &&
            command[6] <= 'Z') {

            if (command[7] != '\0') {
                printk("SINTAX ERROR!\n");
            } else {

                int value;

                if (getVariable(command[6], &value)) {

                    char buffer[12];

                    intToString(value, buffer);
                    printk(buffer);
                    printk("\n");

                } else {
                    printk("VARIABLE ERROR!\n");
                }
            }
            }

            else {
                printk("SINTAX ERROR!\n");
            }
            textPointer = 0;
            inputBuffer[0] = '\0';
    } else {
        printk("SINTAX ERROR!\n");
    }

    textPointer = 0;
    inputBuffer[0] = '\0';

    printk("> ");
}

