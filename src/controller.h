#pragma once

#include <windows.h>
#include <dshow.h>
#include <comdef.h>
#include <d3d9.h>
#include <vmr9.h>
#include <vector>
#include <string>

// define smart pointers for used interfaces
_COM_SMARTPTR_TYPEDEF(IPin, __uuidof(IPin));
_COM_SMARTPTR_TYPEDEF(IBaseFilter, __uuidof(IBaseFilter));
_COM_SMARTPTR_TYPEDEF(IGraphBuilder, __uuidof(IGraphBuilder));
_COM_SMARTPTR_TYPEDEF(IMediaControl, __uuidof(IMediaControl));
_COM_SMARTPTR_TYPEDEF(IVMRMonitorConfig9, __uuidof(IVMRMonitorConfig9));


class Controller
{
public:
    bool Initialize();
    void Uninitialize();

    int GetMonitorCount();
    std::wstring GetMonitorName( int nMonitorIndex );

    bool Start();
protected:
    HRESULT CreateGraphObjects();

    IGraphBuilderPtr m_pGraph;
    IMediaControlPtr m_pControl;
    IVMRMonitorConfig9Ptr m_pMonitorConfig;

    std::vector<VMR9MonitorInfo> m_vMonitorInfo;
};