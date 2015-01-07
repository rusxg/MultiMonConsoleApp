#include <stdio.h>
#include <conio.h>
#include <windows.h>

#include "MultiMonRenderer.h"

int main(void)
{
    MMR_HANDLE handle = MMR_Initialize();
    if (!handle)
    {
        printf("Can't initialize MultiMonRenderer instance\n");
        return -1;
    }

    printf("Available monitors:\n\n");
    int nMonitorCount = 0;
    char **ppMonitorNames = NULL;
    MMR_GetCards(handle, &ppMonitorNames, &nMonitorCount);
    for (int i = 0; i < nMonitorCount; i++)
    {
        printf("%d. %s\n", i + 1, ppMonitorNames[i]);
    }

    int nMonitorIndex = 0;
    for(;;)
    {
        printf("Enter monitor index: [1 .. %d]\n", nMonitorCount);
        if (scanf_s("%d", &nMonitorIndex) == 1)
        {
            printf("Selected monitor index: %d\n", nMonitorIndex);
            if (nMonitorIndex > 0 && nMonitorIndex <= nMonitorCount)
            {
                nMonitorIndex--; // back to zero-based index
                break;
            }
        }
        else
        {
            while (fgetc(stdin) != '\n') // Read until a newline is found
                ;
        }
        printf("Incorrect value\n");
    }

    printf("Press ESC to exit\n");

    MMR_Start(handle, nMonitorIndex, 320, 240);

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (_kbhit() && _getch() == VK_ESCAPE)
            break;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

