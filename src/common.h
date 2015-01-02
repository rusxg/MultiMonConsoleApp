#pragma once

#include "Windows.h"
#include <string>
#include <sstream>
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
    virtual int GetMonitorCount() = 0;
    virtual std::wstring GetMonitorName( int nMonitorIndex ) = 0;
    virtual bool SetMonitorIndex( int nMonitorIndex ) = 0;
};

