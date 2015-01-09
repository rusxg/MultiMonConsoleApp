#include "common.h"
#include "MultiMonRenderer.h"
#include "controller.h"
#include "canvas.h"

struct MMR_CONTEXT
{
    Controller m_controller;
    Canvas m_canvas;
    char **m_aMonitorNames;
    int m_nMonitorCount;
};

MMR_HANDLE MMR_API MMR_Initialize()
{
    MMR_CONTEXT *context = new MMR_CONTEXT();
    context->m_controller.Initialize();
    context->m_nMonitorCount = context->m_controller.GetMonitorCount();
    if (context->m_nMonitorCount > 0)
    {
        context->m_aMonitorNames = new char* [context->m_nMonitorCount];
        for (int i = 0; i < context->m_nMonitorCount; i++)
        {
            static const int MONITOR_NAME_MAX_SIZE = 128;
            context->m_aMonitorNames[i] = (char *)malloc(MONITOR_NAME_MAX_SIZE + 1);
            context->m_controller.GetMonitorName(i, context->m_aMonitorNames[i], MONITOR_NAME_MAX_SIZE);
        }
    }
    return context;
}

void MMR_API MMR_GetCards(MMR_HANDLE handle, char*** names, int *count)
{
    *names = NULL;
    *count = 0;

    MMR_CONTEXT *context = (MMR_CONTEXT *)handle;
    if (context)
    {
        *names = &context->m_aMonitorNames[0];
        *count = context->m_nMonitorCount;
    }
}

void MMR_API MMR_Start(MMR_HANDLE handle, int cardIndex, int width, int height)
{
    MMR_CONTEXT *context = (MMR_CONTEXT *)handle;
    context->m_canvas.Initialize();
    context->m_controller.SetCanvas(&context->m_canvas);
    context->m_controller.SetMonitorIndex(cardIndex);

    if (!context->m_controller.Start())
    {
        printf("Can't start DirectShow controller\n");
        return;
    }
}

void MMR_API MMR_SendFrame(MMR_HANDLE handle, const char* buf, int buf_size)
{
    MMR_CONTEXT *context = (MMR_CONTEXT *)handle;
    context->m_controller.DrawFrame(buf, buf_size);
}

void MMR_API MMR_Stop(MMR_HANDLE handle)
{

}

void MMR_API MMR_Uninitialize() 
{

}
