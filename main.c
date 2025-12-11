// ELEC2645 Individual Project
// Command Line Application Menu Handling Code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "funcs.h"

static void main_menu(void);
static void print_main_menu(void);
static int  get_user_input(void);
static void select_menu_item(int input);
static void go_back_to_main(void);
static int  is_integer(const char *s);

int main(void)
{
    /* keep calling the menu until the user chooses Exit */
    for (;;)
    {
        main_menu();
    }
    return 0; /* not reached */
}

static void main_menu(void)
{
    int input;

    print_main_menu();
    input = get_user_input();
    select_menu_item(input);
}

/* get a valid integer choice in the range [1,5] */
static int get_user_input(void)
{
    enum { MENU_ITEMS = 5 };
    char buf[64];
    int value = 0;
    int ok = 0;

    do {
        printf("\nSelect item: ");
        if (!fgets(buf, sizeof(buf), stdin))
        {
            puts("\nInput error. Exiting.");
            exit(1);
        }

        buf[strcspn(buf, "\r\n")] = '\0';

        if (!is_integer(buf))
        {
            printf("Enter an integer!\n");
        }
        else
        {
            value = (int)strtol(buf, NULL, 10);
            if (value >= 1 && value <= MENU_ITEMS)
            {
                ok = 1;
            }
            else
            {
                printf("Invalid menu item!\n");
            }
        }
    } while (!ok);

    return value;
}

static void select_menu_item(int input)
{
    switch (input)
    {
        case 1:
            menu_item_1();
            go_back_to_main();
            break;
        case 2:
            menu_item_2();
            go_back_to_main();
            break;
        case 3:
            menu_item_3();
            go_back_to_main();
            break;
        case 4:
            menu_item_4();
            go_back_to_main();
            break;
        case 5:
        default:
            printf("Bye!\n");
            exit(0);
    }
}

static void print_main_menu(void)
{
    printf("\n----------- EnviroSense Main menu -----------\n");
    printf("\n");
    printf("\t1. Sensor setup and overview\n");
    printf("\t2. Single ADC conversion\n");
    printf("\t3. Batch conversion + statistics + plot\n");
    printf("\t4. Save/load, tests and help\n");
    printf("\t5. Exit\n");
    printf("---------------------------------------------\n");
}

/* wait for the user to press 'b' or 'B' before returning */
static void go_back_to_main(void)
{
    char buf[16];

    do {
        printf("\nEnter 'b' or 'B' to go back to main menu: ");
        if (!fgets(buf, sizeof(buf), stdin))
        {
            puts("\nInput error. Exiting.");
            exit(1);
        }
        buf[strcspn(buf, "\r\n")] = '\0';
    } while (!(buf[0] == 'b' || buf[0] == 'B') || buf[1] != '\0');
}

/* check if a string represents a valid integer */
static int is_integer(const char *s)
{
    if (!s || !*s) return 0;

    if (*s == '+' || *s == '-') s++;

    if (!isdigit((unsigned char)*s)) return 0;

    while (*s)
    {
        if (!isdigit((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}
