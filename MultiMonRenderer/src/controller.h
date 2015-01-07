#pragma once

#include <dshow.h>
#include <comdef.h>
#include <d3d9.h>
#include <vmr9.h>
#include <vector>

// define smart pointers for used interfaces
_COM_SMARTPTR_TYPEDEF(IPin, __uuidof(IPin));
_COM_SMARTPTR_TYPEDEF(IBaseFilter, __uuidof(IBaseFilter));
_COM_SMARTPTR_TYPEDEF(IGraphBuilder, __uuidof(IGraphBuilder));
_COM_SMARTPTR_TYPEDEF(IMediaControl, __uuidof(IMediaControl));
_COM_SMARTPTR_TYPEDEF(IVMRMonitorConfig9, __uuidof(IVMRMonitorConfig9));
_COM_SMARTPTR_TYPEDEF(IVMRFilterConfig9, __uuidof(IVMRFilterConfig9));
_COM_SMARTPTR_TYPEDEF(IVMRWindowlessControl9, __uuidof(IVMRWindowlessControl9));

class Controller
    : public IController
    , protected ICanvasPaintCallback
{
public:
    Controller();
    bool Initialize();
    void Uninitialize();

    // IController
    bool SetCanvas(ICanvas *pCanvas);
    int GetMonitorCount();
    void GetMonitorName(int nMonitorIndex, char *cNameBuf, int nNameBufSize);
    bool SetMonitorIndex(int nMonitorIndex);

    // ICanvasPaintCallback
    void OnDisplayChange();
    void OnPaint(HDC hdc);

    bool Start();
protected:
    HRESULT CreateGraphObjects();
    HRESULT CreateRenderer();
    HRESULT SetupRenderer();
    IGraphBuilderPtr m_pGraph;
    IMediaControlPtr m_pControl;
    IBaseFilterPtr m_pRenderer;
    IVMRWindowlessControl9Ptr m_pWindowlessControl;
    int m_nMonitorIndex;
    bool m_bCoInitialized;

    VMR9MonitorInfo *m_aMonitorInfo;
    int m_nMonitorCount;
    ICanvas *m_pCanvas;
};