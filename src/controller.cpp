#include "common.h"
#include "controller.h"
#include "filter.h"

bool Controller::Initialize()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        return false;

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
    return m_vMonitorInfo.size();
}

std::wstring Controller::GetMonitorName(int nMonitorIndex )
{
    if (nMonitorIndex < 0 || (unsigned int)nMonitorIndex >= m_vMonitorInfo.size())
        return L"";

    std::wstringstream stream;
    const VMR9MonitorInfo& info = m_vMonitorInfo[nMonitorIndex];
    stream << info.szDevice << " [ " << info.szDescription << " ]";
    return stream.str();
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

    m_vMonitorInfo.resize(dwMonitorCount);
    DWORD dwMonitorInfoWritten = 0;
    hr = pMonitorConfig->GetAvailableMonitors(&m_vMonitorInfo[0], dwMonitorCount, &dwMonitorInfoWritten);
    if (FAILED(hr))
    {
        printf("Can't get available monitors info\n");
        return hr;
    }

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
{
}

HRESULT Controller::SetupRenderer()
{
    HRESULT hr;

    const RECT& rcMonitor = m_vMonitorInfo[m_nMonitorIndex].rcMonitor;
    m_pCanvas->SetPosition(rcMonitor);

    HWND hwndCanvas = m_pCanvas->GetHWND();
    RECT rcPosition;
    if (!GetWindowRect(hwndCanvas, &rcPosition))
        return E_FAIL;
    m_pWindowlessControl->SetVideoClippingWindow(hwndCanvas);
    RECT rcClientPosition = {};
    rcClientPosition.right = rcMonitor.right - rcMonitor.left;
    rcClientPosition.bottom = rcMonitor.bottom - rcMonitor.top;
    m_pWindowlessControl->SetVideoPosition(NULL, &rcClientPosition);

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
    if (nMonitorIndex < 0 || (unsigned int)nMonitorIndex >= m_vMonitorInfo.size())
        return false;

    m_nMonitorIndex = nMonitorIndex;
    return true;
}
