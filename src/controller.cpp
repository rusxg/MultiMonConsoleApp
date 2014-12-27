#include "controller.h"
#include "filter.h"
#include  <sstream>

bool Controller::Initialize()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        return false;

    hr = CreateGraphObjects();
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
    IBaseFilterPtr pRenderer;
    hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL,
        CLSCTX_INPROC, IID_IBaseFilter, (void**)&pRenderer);
    if (FAILED(hr))
    {
        printf("Can't create VideoMixingRenderer9 instance\n");
        return false;
    }

    hr = m_pGraph->AddFilter(pRenderer, L"Renderer");
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

    //
    hr = pRenderer->QueryInterface(IID_IVMRMonitorConfig9, (void **)&m_pMonitorConfig);
    if (FAILED(hr))
    {
        printf("Can't get IVMRMonitorConfig9 interface\n");
        return false;
    }

    DWORD dwMonitorCount = 0;
    hr = m_pMonitorConfig->GetAvailableMonitors(NULL, 0, &dwMonitorCount);
    if (FAILED(hr))
    {
        printf("Can't get available monitors count\n");
        return false;
    }

    m_vMonitorInfo.resize(dwMonitorCount);
    DWORD dwMonitorInfoWritten = 0;
    hr = m_pMonitorConfig->GetAvailableMonitors(&m_vMonitorInfo[0], dwMonitorCount, &dwMonitorInfoWritten);
    if (FAILED(hr))
    {
        printf("Can't get available monitors info\n");
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
    if (!m_pGraph)
    {
        printf("Not initialized\n");
        return false;
    }

    HRESULT hr;
    hr = m_pControl->Run();
    if (FAILED(hr))
    {
        printf("Error occured while playing or pausing or opening the file\n");
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
