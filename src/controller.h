#pragma once

#include <windows.h>
#include <dshow.h>
#include <comdef.h>

// define smart pointers for used interfaces
_COM_SMARTPTR_TYPEDEF(IPin, __uuidof(IPin));
_COM_SMARTPTR_TYPEDEF(IBaseFilter, __uuidof(IBaseFilter));
_COM_SMARTPTR_TYPEDEF(IGraphBuilder, __uuidof(IGraphBuilder));
_COM_SMARTPTR_TYPEDEF(IMediaControl, __uuidof(IMediaControl));


class Controller
{
public:
    bool Initialize();
    void Uninitialize();
    bool Start();
protected:
    HRESULT CreateGraphObjects();

    IGraphBuilderPtr m_pGraph;
    IMediaControlPtr m_pControl;

};