#pragma once

#include "Windows.h"
#include <assert.h>

class ICanvasPaintCallback
{
public:
    virtual void OnDisplayChange() = 0;
    virtual void OnPaint(HDC hdc) = 0;
};

class ICanvasInputCallback
{
public:
    virtual void OnKeyDown(DWORD dwKeyCode, DWORD dwKeyFlags) = 0;
};

class ICanvas
{
public:
    virtual bool SetPosition(const RECT& rcMonitor) = 0;
    virtual bool SetPaintCallback(ICanvasPaintCallback *pCallback) = 0;
    virtual bool SetInputCallback(ICanvasInputCallback *pCallback) = 0;
    virtual HWND GetHWND() = 0;
};

class IController
{
public:
    virtual bool SetCanvas(ICanvas *pCanvas) = 0;
    virtual int  GetMonitorCount() = 0;
    virtual void GetMonitorName(int nMonitorIndex, char *cNameBuf, int nNameBufSize) = 0;
    virtual bool SetMonitorIndex(int nMonitorIndex) = 0;
    virtual bool SetFrameDimensions(int nWidth, int nHeight) = 0;
    virtual bool Start() = 0;
    virtual bool DrawFrame(const void *pFrameData, int nFrameSize, LONGLONG frameDuration) = 0;
    virtual void Stop() = 0;
};

#undef  INTERFACE
#define INTERFACE   IFrameReceiver

DECLARE_INTERFACE_IID_(IFrameReceiver, IUnknown, "2BFC1CAC-6E74-4249-A579-F6FE20FDC7E4")
{
    BEGIN_INTERFACE

        // *** IUnknown methods ***
        STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
        STDMETHOD_(ULONG, AddRef)(THIS)PURE;
        STDMETHOD_(ULONG, Release)(THIS)PURE;

        // ** IFrameReceiver methods ***
        STDMETHOD(SetFrameDimensions)(THIS_ int nWidth, int nHeight) PURE;
        STDMETHOD(ReceiveFrame)(THIS_ void *pFrameData, int iFrameDataSize, LONGLONG frameDuration) PURE;

    END_INTERFACE
}; 


