#include "common.h"
#include "canvas.h"

Canvas::Canvas()
    : m_pEventCallback(NULL)
    , m_pInputCallback(NULL)
    , m_hwnd(NULL)
{
}

bool Canvas::Initialize()
{
    WNDCLASSEXW wcx; 

    wcx.cbSize = sizeof(WNDCLASSEXW);							// size of structure 
    wcx.style = CS_HREDRAW | CS_VREDRAW;						// redraw if size changes 
    wcx.lpfnWndProc = stWindowProc;								// points to window procedure 
    wcx.cbClsExtra = 0;											// no extra class memory 
    wcx.cbWndExtra = 0;											// no extra window memory 
    wcx.hInstance = NULL;										// handle to instance 
    wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);				// predefined app. icon 
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);					// predefined arrow 
    wcx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	// white background brush 
    wcx.lpszMenuName = NULL;									// name of menu resource 
    wcx.lpszClassName = L"CanvasWindow";							// name of window class 
    wcx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);				// small class icon 

    RegisterClassExW(&wcx);

    m_hwnd = CreateWindowW(wcx.lpszClassName, L"CanvasWindow2", WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, 
        CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, 
        (void *)this);

    return true;
}

LRESULT CALLBACK Canvas::stWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Canvas* pWnd;

    if (uMsg == WM_INITDIALOG)
    {
        SetWindowLong(hwnd, GWL_USERDATA, lParam);
    }
    else if (uMsg == WM_CREATE)
    {
        SetWindowLong(hwnd, GWL_USERDATA, (LONG_PTR)((LPCREATESTRUCT(lParam))->lpCreateParams));
    }

    // get the pointer to the window
    pWnd = GetObjectFromWindow(hwnd);
    
#ifdef _DEBUG
    if (uMsg != WM_PAINT)
    {
        std::stringstream ss;
        ss << "Window proc message " << std::hex << uMsg << " [ ";
#define MESSAGE_CASE_ENTRY(x) case x: ss << #x; break;
        switch (uMsg)
        {
            MESSAGE_CASE_ENTRY(WM_SETFONT);
            MESSAGE_CASE_ENTRY(WM_INITDIALOG);
            MESSAGE_CASE_ENTRY(WM_GETDLGCODE);
            MESSAGE_CASE_ENTRY(WM_WINDOWPOSCHANGING);
            MESSAGE_CASE_ENTRY(WM_ACTIVATEAPP);
            MESSAGE_CASE_ENTRY(WM_NCACTIVATE);
            MESSAGE_CASE_ENTRY(WM_ACTIVATE);
            MESSAGE_CASE_ENTRY(WM_IME_SETCONTEXT);
            MESSAGE_CASE_ENTRY(WM_IME_NOTIFY);
            MESSAGE_CASE_ENTRY(WM_SETFOCUS);
            MESSAGE_CASE_ENTRY(WM_CHANGEUISTATE);
            MESSAGE_CASE_ENTRY(WM_WINDOWPOSCHANGED);
            MESSAGE_CASE_ENTRY(WM_MOVE);
            MESSAGE_CASE_ENTRY(WM_SIZE);
            MESSAGE_CASE_ENTRY(WM_SHOWWINDOW);
            MESSAGE_CASE_ENTRY(WM_NCPAINT);
            MESSAGE_CASE_ENTRY(WM_GETICON);
            MESSAGE_CASE_ENTRY(WM_ERASEBKGND);
            MESSAGE_CASE_ENTRY(WM_NCCALCSIZE);
            MESSAGE_CASE_ENTRY(WM_KILLFOCUS);
            MESSAGE_CASE_ENTRY(WM_CTLCOLORDLG);
        }
        ss << " ] window found: " << pWnd << std::endl;
        OutputDebugStringA(ss.str().c_str());
    }
#endif

    // if we have the pointer, go to the message handler of the window
    // else, use DefWindowProc
    if (pWnd)
        return pWnd->WindowProc(hwnd, uMsg, wParam, lParam);
    else
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK Canvas::WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch ( uMsg )
    {
    case WM_DISPLAYCHANGE:
        if (m_pEventCallback)
        {
            m_pEventCallback->OnDisplayChange();
            return TRUE;
        }
        break;

    case WM_PAINT:
        if (m_pEventCallback)
        {
            m_pEventCallback->OnPaint((HDC)wParam);
            return TRUE;
        }
        break;

    case WM_KEYDOWN:
        if (m_pInputCallback)
        {
            m_pInputCallback->OnKeyDown(wParam, lParam);
            return TRUE;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


bool Canvas::SetPosition( const RECT& rcMonitor )
{
    if (!m_hwnd)
        return false;

    MoveWindow(m_hwnd, rcMonitor.left, rcMonitor.top,
        rcMonitor.right - rcMonitor.left,
        rcMonitor.bottom - rcMonitor.top,
        TRUE);
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);

    return true;
}

bool Canvas::SetPaintCallback( ICanvasPaintCallback *pCallback )
{
    m_pEventCallback = pCallback;
    return true;
}

bool Canvas::SetInputCallback( ICanvasInputCallback *pCallback )
{
    m_pInputCallback = pCallback;
    return true;
}

HWND Canvas::GetHWND()
{
    return m_hwnd;
}

