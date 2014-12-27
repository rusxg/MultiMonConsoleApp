#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include "controller.h"

int main(void)
{
    Controller controller;
    if (!controller.Initialize())
    {
        printf("Can't initialize DirectShow controller\n");
        return -1;
    }

    if (!controller.Start())
    {
        printf("Can't initialize DirectShow controller\n");
        return -1;
    }

    getchar();

    return 0;
}


