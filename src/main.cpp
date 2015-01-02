#include <stdio.h>
#include <conio.h>

#include "common.h"

#include "controller.h"
#include "canvas.h"

struct CanvasInputCallback
    : public ICanvasInputCallback
{
    bool aborted;
    CanvasInputCallback() : aborted(false) {}
    void OnKeyDown(DWORD dwKeyCode, DWORD dwKeyFlags)
    {
        if (dwKeyCode == VK_ESCAPE)
            aborted = true;
    }
};

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

    Canvas canvas;
    canvas.Initialize();
    controller.SetCanvas(&canvas);
    controller.SetMonitorIndex(nMonitorIndex);

    CanvasInputCallback canvasInputCallback;
    canvas.SetInputCallback(&canvasInputCallback);

    if (!controller.Start())
    {
        printf("Can't initialize DirectShow controller\n");
        return -1;
    }

    printf("Press ESC to exit\n");

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (canvasInputCallback.aborted)
            break;

        if (_kbhit() && _getch() == VK_ESCAPE)
            break;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

