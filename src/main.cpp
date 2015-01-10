#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <process.h>

#include "MultiMonRenderer.h"
#include "generator.h"

static const int PICTURE_WIDTH = 320;
static const int PICTURE_HEIGHT = 240;

struct RenderingParameters
{
    MMR_HANDLE mmrHandle;
    int monitorIndex;
    HANDLE hStopEvent;
};

unsigned __stdcall RenderingThread(void *context);

int main(void)
{
    RenderingParameters params = {};
    params.mmrHandle = MMR_Initialize();
    if (!params.mmrHandle)
    {
        printf("Can't initialize MultiMonRenderer instance\n");
        return -1;
    }

    printf("Available monitors:\n\n");
    int nMonitorCount = 0;
    char **ppMonitorNames = NULL;
    MMR_GetCards(params.mmrHandle, &ppMonitorNames, &nMonitorCount);
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
    params.monitorIndex = nMonitorIndex;
    params.hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    printf("Press ESC to exit\n");

    MMR_Start(params.mmrHandle, params.monitorIndex, PICTURE_WIDTH, PICTURE_HEIGHT);

    HANDLE hRenderingThread = (HANDLE)_beginthreadex(NULL, 0, RenderingThread, &params, 0, NULL);
    if (!hRenderingThread)
    {
        printf("Can't create rendering thread\n");
        return -2;
    }

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (_kbhit() && _getch() == VK_ESCAPE)
            break;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    printf("Stopping\n");
    SetEvent(params.hStopEvent);
    if (WaitForSingleObject(hRenderingThread, 1000) == WAIT_OBJECT_0)
    {
        printf("Stopped successfully\n");
    }
    else
    {
        printf("Stop timed out\n");
    }

    MMR_Stop(params.mmrHandle);
    MMR_Uninitialize(params.mmrHandle);

    return 0;
}

unsigned __stdcall RenderingThread(void *context)
{
    RenderingParameters *params = (RenderingParameters *)context;
    static const int PICTURE_BUFFER_SIZE = 3 * PICTURE_WIDTH * PICTURE_HEIGHT;

    BYTE *pPictureBuffer = (BYTE *)malloc(PICTURE_BUFFER_SIZE);
    PictureGenerator generator(PICTURE_WIDTH, PICTURE_HEIGHT);
    LONG lTime = 0;
    for (;;)
    {
        static const int MILLISECONDS_PER_FRAME = 40;
        DWORD dwWaitResult = WaitForSingleObject(params->hStopEvent, MILLISECONDS_PER_FRAME);
        if (dwWaitResult == WAIT_OBJECT_0)
            break;

        generator.MoveBall(lTime);
        lTime += MILLISECONDS_PER_FRAME;

        ZeroMemory(pPictureBuffer, PICTURE_BUFFER_SIZE);

        static const BYTE pBallPixel[] = { 0xFF, 0xFF, 0xFF }; // white color
        generator.PlotBall(pPictureBuffer, &pBallPixel[0], sizeof(pBallPixel));

        MMR_SendFrame(params->mmrHandle, (const char *)pPictureBuffer, PICTURE_BUFFER_SIZE,
            MMR_MILLISECONDS_TO_TIME_TICKS(MILLISECONDS_PER_FRAME));
    }

    free(pPictureBuffer);

    return 0;
}
