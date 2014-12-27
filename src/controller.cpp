#include "controller.h"
#include "..\filter.h"

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

    hr = m_pControl->Run();
    if (FAILED(hr))
    {
        printf("Error occured while playing or pausing or opening the file\n");
        return false;
    }

    return true;
}
