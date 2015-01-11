#pragma once

class Canvas
    : public ICanvas
{
public:
    Canvas();

    bool Initialize();
    void Uninitialize();

    // ICanvas
    bool SetPosition(const RECT& rcMonitor);
    bool SetPaintCallback(ICanvasPaintCallback *pCallback);
    bool SetInputCallback(ICanvasInputCallback *pCallback);
    HWND GetHWND();

protected:
    HWND m_hwnd;
    ICanvasPaintCallback *m_pEventCallback;
    ICanvasInputCallback *m_pInputCallback;

    static LRESULT CALLBACK stWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // returns a pointer the window (stored as the WindowLong)
    inline static Canvas *GetObjectFromWindow(HWND hWnd)
    {
        return (Canvas *)GetWindowLong(hWnd, GWL_USERDATA);
    }
};

