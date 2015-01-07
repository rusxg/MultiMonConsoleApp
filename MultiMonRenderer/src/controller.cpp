#include "common.h"
#include "controller.h"
#include "filter.h"

bool Controller::Initialize()
{
    m_bCoInitialized = true;
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        m_bCoInitialized = false;
        if (hr == RPC_E_CHANGED_MODE)
            return false;
    }

    hr = CreateRenderer();
    if (FAILED(hr))
    {
        Uninitialize();
        return false;
    }

    return true;
}

HRESULT Controller::CreateGraphObjects()
{
    HRESULT hr;

    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&m_pGraph);
    if (FAILED(hr))
        return hr;

    hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pControl);
    if (FAILED(hr))
        return hr;

    //
    hr = m_pGraph->AddFilter(m_pRenderer, L"Renderer");
    if (FAILED(hr))
    {
        printf("Can't add GeneratorFilter to graph\n");
        return false;
    }

    //
    IBaseFilterPtr pFilter = (LPUNKNOWN)GeneratorFilter::CreateInstance(NULL, &hr);
    if (FAILED(hr))
    {
        printf("Can't create GeneratorFilter\n");
        return false;
    }
    assert(pFilter);

    hr = m_pGraph->AddFilter(pFilter, L"Picture Generator Filter");
    if (FAILED(hr))
    {
        printf("Can't add GeneratorFilter to graph\n");
        return false;
    }

    //
    IPinPtr pPin;
    hr = pFilter->FindPin(L"1", &pPin);
    if (FAILED(hr))
    {
        printf("Can't find GeneratorFilter pin\n");
        return false;
    }

    hr = m_pGraph->Render(pPin);
    if (FAILED(hr))
    {
        printf("Can't render the graph\n");
        return false;
    }

    return S_OK;
}

void Controller::Uninitialize()
{
    m_pGraph = NULL;
    m_pControl = NULL;
    if (m_bCoInitialized)
        CoUninitialize();
}

bool Controller::Start()
{
    if (!m_pCanvas)
        return false;

    HRESULT hr;
    hr = SetupRenderer();
    if (FAILED(hr))
    {
        return false;
    }

    hr = CreateGraphObjects();
    if (FAILED(hr))
    {
        return false;
    }

    hr = m_pControl->Run();
    if (FAILED(hr))
    {
        printf("Error occurred while playing or pausing or opening the file\n");
        return false;
    }

    return true;
}

int Controller::GetMonitorCount()
{
    return m_nMonitorCount;
}

void Controller::GetMonitorName(int nMonitorIndex, char *cNameBuf, int nNameBufSize )
{
    *cNameBuf = '\0';
    if (nMonitorIndex < 0 || nMonitorIndex >= m_nMonitorCount)
        return;

    const VMR9MonitorInfo& info = m_aMonitorInfo[nMonitorIndex];
    sprintf_s(cNameBuf, nNameBufSize, "%ws [ %ws ]", info.szDevice, info.szDescription);
}

HRESULT Controller::CreateRenderer()
{
    HRESULT hr;
    hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL,
        CLSCTX_INPROC, IID_IBaseFilter, (void**)&m_pRenderer);
    if (FAILED(hr))
    {
        printf("Can't create VideoMixingRenderer9 instance\n");
        return hr;
    }

    IVMRMonitorConfig9Ptr pMonitorConfig;
    hr = m_pRenderer->QueryInterface(IID_IVMRMonitorConfig9, (void **)&pMonitorConfig);
    if (FAILED(hr))
    {
        printf("Can't get IVMRMonitorConfig9 interface\n");
        return hr;
    }

    DWORD dwMonitorCount = 0;
    hr = pMonitorConfig->GetAvailableMonitors(NULL, 0, &dwMonitorCount);
    if (FAILED(hr))
    {
        printf("Can't get available monitors count\n");
        return hr;
    }

    assert(!m_aMonitorInfo);
    m_aMonitorInfo = (VMR9MonitorInfo *)malloc(dwMonitorCount* sizeof(VMR9MonitorInfo));
    if (!m_aMonitorInfo)
    {
        printf("Can't allocate buffer for monitor info\n");
        return E_OUTOFMEMORY;
    }

    DWORD dwMonitorInfoWritten = 0;
    hr = pMonitorConfig->GetAvailableMonitors(&m_aMonitorInfo[0], dwMonitorCount, &dwMonitorInfoWritten);
    if (FAILED(hr))
    {
        printf("Can't get available monitors info\n");
        return hr;
    }
    m_nMonitorCount = dwMonitorInfoWritten;

    IVMRFilterConfig9Ptr pFilterConfig;
    hr = m_pRenderer->QueryInterface(IID_IVMRFilterConfig9, (void **)&pFilterConfig);
    if (FAILED(hr))
        return hr;

    hr = pFilterConfig->SetRenderingMode(VMRMode_Windowless);
    if (FAILED(hr))
        return hr;

    hr = m_pRenderer->QueryInterface(IID_IVMRWindowlessControl9, (void **)&m_pWindowlessControl);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

Controller::Controller()
: m_pCanvas(NULL)
, m_nMonitorIndex(0)
, m_bCoInitialized(false)
, m_nMonitorCount(0)
, m_aMonitorInfo(NULL)
{
}

HRESULT Controller::SetupRenderer()
{
    const RECT& rcMonitor = m_aMonitorInfo[m_nMonitorIndex].rcMonitor;
    m_pCanvas->SetPosition(rcMonitor);

    HWND hwndCanvas = m_pCanvas->GetHWND();
    RECT rcClientPosition;
    if (!GetClientRect(hwndCanvas, &rcClientPosition))
        return E_FAIL;
    HRESULT hr = m_pWindowlessControl->SetVideoClippingWindow(hwndCanvas);
    assert(SUCCEEDED(hr));
    hr = m_pWindowlessControl->SetVideoPosition(NULL, &rcClientPosition);
    assert(SUCCEEDED(hr));

    return S_OK;
}

bool Controller::SetCanvas( ICanvas *pCanvas )
{
    if (m_pCanvas)
    {
        m_pCanvas->SetPaintCallback(NULL);
    }
    m_pCanvas = pCanvas;
    if (m_pCanvas)
    {
        m_pCanvas->SetPaintCallback(this);
    }
    return true;
}

void Controller::OnDisplayChange()
{
    if (m_pWindowlessControl)
        m_pWindowlessControl->DisplayModeChanged();
}

void Controller::OnPaint( HDC hdc )
{
    if (m_pWindowlessControl && m_pCanvas)
        m_pWindowlessControl->RepaintVideo(m_pCanvas->GetHWND(), hdc);
}

bool Controller::SetMonitorIndex( int nMonitorIndex )
{
    if (nMonitorIndex < 0 || nMonitorIndex >= m_nMonitorCount)
        return false;

    m_nMonitorIndex = nMonitorIndex;
    return true;
}
