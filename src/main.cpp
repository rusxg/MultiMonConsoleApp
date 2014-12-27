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

    printf("Available monitors:\n\n");
    int nMonitorCount = controller.GetMonitorCount();
    for (int i = 0; i < nMonitorCount; i++)
    {
        std::wstring sMonitorName = controller.GetMonitorName(i);
        printf("%d. %ws\n", i + 1, sMonitorName.c_str());
    }

    if (!controller.Start())
    {
        printf("Can't initialize DirectShow controller\n");
        return -1;
    }

    getchar();

    return 0;
}


