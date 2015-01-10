#include <streams.h>
#include <olectl.h>
#include <initguid.h>
#include "common.h"
#include "filter.h"

#pragma warning(disable:4710)  // 'function': function not inlined (optimzation)

// Setup data

const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN sudOpPin =
{
    L"Output",              // Pin string name
    FALSE,                  // Is it rendered
    TRUE,                   // Is it an output
    FALSE,                  // Can we have none
    FALSE,                  // Can we have many
    &CLSID_NULL,            // Connects to filter
    NULL,                   // Connects to pin
    1,                      // Number of types
    &sudOpPinTypes };       // Pin details

const AMOVIESETUP_FILTER sudBallax =
{
    &CLSID_GeneratorFilter,    // Filter CLSID
    L"Picture Generator Filter", // String name
    MERIT_DO_NOT_USE,       // Filter merit
    1,                      // Number pins
    &sudOpPin               // Pin details
};


// COM global table of objects in this dll

CFactoryTemplate g_Templates[] = {
  { L"Picture Generator Filter"
  , &CLSID_GeneratorFilter
  , GeneratorFilter::CreateInstance
  , NULL
  , &sudBallax }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


//
// CreateInstance
//
// The only allowed way to create Picture Generator Filters!
//
CUnknown * WINAPI GeneratorFilter::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    ASSERT(phr);

    CUnknown *punk = new GeneratorFilter(lpunk, phr);
    if(punk == NULL)
    {
        if(phr)
            *phr = E_OUTOFMEMORY;
    }
    return punk;

} // CreateInstance


//
// Constructor
//
// Initialise a GeneratorStream object so that we have a pin.
//
GeneratorFilter::GeneratorFilter(LPUNKNOWN lpunk, HRESULT *phr) :
    CSource(NAME("Picture Generator Filter"), lpunk, CLSID_GeneratorFilter)
{
    ASSERT(phr);
    CAutoLock cAutoLock(&m_cStateLock);

    m_paStreams = (CSourceStream **) new GeneratorStream*[1];
    if(m_paStreams == NULL)
    {
        if(phr)
            *phr = E_OUTOFMEMORY;

        return;
    }

    m_paStreams[0] = new GeneratorStream(phr, this, L"A Picture Generator Filter!");
    if(m_paStreams[0] == NULL)
    {
        if(phr)
            *phr = E_OUTOFMEMORY;

        return;
    }

} // (Constructor)


//
// Constructor
//
GeneratorStream::GeneratorStream(HRESULT *phr,
                         GeneratorFilter *pParent,
                         LPCWSTR pPinName) :
    CSourceStream(NAME("Picture Generator Filter"),phr, pParent, pPinName),
    m_iImageWidth(320),
    m_iImageHeight(240),
    m_evFrameCopied(phr),
    m_pFrameData(NULL),
    m_iFrameDataSize(0),
    m_tFrameDuration(0)
{
} // (Constructor)


//
// Destructor
//
GeneratorStream::~GeneratorStream()
{
} // (Destructor)


//
// FillBuffer
//
HRESULT GeneratorStream::FillBuffer(IMediaSample *pms)
{
    CheckPointer(pms,E_POINTER);

    BYTE *pData;
    long lDataLen;

    pms->GetPointer(&pData);
    lDataLen = pms->GetSize();

    ZeroMemory(pData, lDataLen);
    {
        CAutoLock cAutoLockShared(&m_cSharedState);

        assert(m_pFrameData && m_iFrameDataSize);
        assert(m_iFrameDataSize <= lDataLen);
        CopyMemory(pData, m_pFrameData, m_iFrameDataSize);

        // The current time is the sample's start
        CRefTime rtStart = m_rtSampleTime;

        // Increment to find the finish time
        m_rtSampleTime += m_tFrameDuration;

        pms->SetTime((REFERENCE_TIME *) &rtStart,(REFERENCE_TIME *) &m_rtSampleTime);
    }

    pms->SetSyncPoint(TRUE);
    return NOERROR;

} // FillBuffer


//
// Notify
//
// Alter the repeat rate according to quality management messages sent from
// the downstream filter (often the renderer).  Wind it up or down according
// to the flooding level - also skip forward if we are notified of Late-ness
//
STDMETHODIMP GeneratorStream::Notify(IBaseFilter * pSender, Quality q)
{
    // Adjust the repeat rate.

    return NOERROR;

} // Notify


//
// GetMediaType
//
HRESULT GeneratorStream::GetMediaType(int iPosition, CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);

    CAutoLock cAutoLock(m_pFilter->pStateLock());
    if(iPosition < 0)
    {
        return E_INVALIDARG;
    }

    // Have we run off the end of types?

    if(iPosition > 1)
    {
        return VFW_S_NO_MORE_ITEMS;
    }

    VIDEOINFO *pvi = (VIDEOINFO *) pmt->AllocFormatBuffer(sizeof(VIDEOINFO));
    if(NULL == pvi)
        return(E_OUTOFMEMORY);

    ZeroMemory(pvi, sizeof(VIDEOINFO));

    switch(iPosition)
    {
        case 0:
        {   // Return our 24bit format

            pvi->bmiHeader.biCompression = BI_RGB;
            pvi->bmiHeader.biBitCount    = 24;
            break;
        }
    }

    pvi->bmiHeader.biSize       = sizeof(BITMAPINFOHEADER);
    pvi->bmiHeader.biWidth      = m_iImageWidth;
    pvi->bmiHeader.biHeight     = m_iImageHeight;
    pvi->bmiHeader.biPlanes     = 1;
    pvi->bmiHeader.biSizeImage  = GetBitmapSize(&pvi->bmiHeader);
    pvi->bmiHeader.biClrImportant = 0;

    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(FALSE);

    // Work out the GUID for the subtype from the header info.
    const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
    pmt->SetSubtype(&SubTypeGUID);
    pmt->SetSampleSize(pvi->bmiHeader.biSizeImage);

    return NOERROR;

} // GetMediaType


//
// CheckMediaType
//
// We will accept 24 bit video formats, in any
// image size.
// Returns E_INVALIDARG if the mediatype is not acceptable
//
HRESULT GeneratorStream::CheckMediaType(const CMediaType *pMediaType)
{
    CheckPointer(pMediaType,E_POINTER);

    if((*(pMediaType->Type()) != MEDIATYPE_Video) ||   // we only output video
       !(pMediaType->IsFixedSize()))                   // in fixed size samples
    {                                                  
        return E_INVALIDARG;
    }

    // Check for the subtypes we support
    const GUID *SubType = pMediaType->Subtype();
    if (SubType == NULL)
        return E_INVALIDARG;

    if(*SubType != MEDIASUBTYPE_RGB24)
    {
        return E_INVALIDARG;
    }

    // Get the format area of the media type
    VIDEOINFO *pvi = (VIDEOINFO *) pMediaType->Format();

    if(pvi == NULL)
        return E_INVALIDARG;

    if((pvi->bmiHeader.biWidth < 20) || ( abs(pvi->bmiHeader.biHeight) < 20))
    {
        return E_INVALIDARG;
    }

    // Check if the image width & height have changed
//     if(pvi->bmiHeader.biWidth != m_Generator->GetImageWidth() || 
//        abs(pvi->bmiHeader.biHeight) != m_Generator->GetImageHeight())
//     {
//         // If the image width/height is changed, fail CheckMediaType() to force
//         // the renderer to resize the image.
//         return E_INVALIDARG;
//     }


    return S_OK;  // This format is acceptable.

} // CheckMediaType


//
// DecideBufferSize
//
// This will always be called after the format has been successfully
// negotiated. So we have a look at m_mt to see what size image we agreed.
// Then we can ask for buffers of the correct size to contain them.
//
HRESULT GeneratorStream::DecideBufferSize(IMemAllocator *pAlloc,
                                      ALLOCATOR_PROPERTIES *pProperties)
{
    CheckPointer(pAlloc,E_POINTER);
    CheckPointer(pProperties,E_POINTER);

    CAutoLock cAutoLock(m_pFilter->pStateLock());
    HRESULT hr = NOERROR;

    VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();
    pProperties->cBuffers = 1;
    pProperties->cbBuffer = pvi->bmiHeader.biSizeImage;

    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if(FAILED(hr))
    {
        return hr;
    }

    // Is this allocator unsuitable

    if(Actual.cbBuffer < pProperties->cbBuffer)
    {
        return E_FAIL;
    }

    // Make sure that we have only 1 buffer (we erase the ball in the
    // old buffer to save having to zero a 200k+ buffer every time
    // we draw a frame)

    ASSERT(Actual.cBuffers == 1);
    return NOERROR;

} // DecideBufferSize


//
// SetMediaType
//
// Called when a media type is agreed between filters
//
HRESULT GeneratorStream::SetMediaType(const CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    // Pass the call up to my base class

    HRESULT hr = CSourceStream::SetMediaType(pMediaType);

    if(SUCCEEDED(hr))
    {
        VIDEOINFO * pvi = (VIDEOINFO *) m_mt.Format();
        if (pvi == NULL)
            return E_UNEXPECTED;

        switch(pvi->bmiHeader.biBitCount)
        {
            case 24:    // Make a green pixel

//                 m_BallPixel[0] = 0x0;
//                 m_BallPixel[1] = 0xff;
//                 m_BallPixel[2] = 0x0;
//                 m_iPixelSize   = 3;
//                 SetPaletteEntries(Green);
                break;
        }

        return NOERROR;
    } 

    return hr;

} // SetMediaType


//
// OnThreadCreate
//
// As we go active reset the stream time to zero
//
HRESULT GeneratorStream::OnThreadCreate()
{
    CAutoLock cAutoLockShared(&m_cSharedState);
    m_rtSampleTime = 0;

    return NOERROR;

} // OnThreadCreate

HRESULT GeneratorStream::DoBufferProcessingLoop(void)
{

    Command com;

    OnThreadStartPlay();

    do
    {
        while (!CheckRequest(&com))
        {

            IMediaSample *pSample;

            HRESULT hr = GetDeliveryBuffer(&pSample, NULL, NULL, 0);
            if (FAILED(hr))
            {
                Sleep(1);
                continue;	// go round again. Perhaps the error will go away
                // or the allocator is decommited & we will be asked to
                // exit soon.
            }

            CAutoLock cAutoLockShared(&m_cSharedState);
            if (!m_pFrameData)
            {
                pSample->Release();
                Sleep(1);
                continue;	// go round again. Perhaps we will get picture data later
            }

            // Virtual function user will override.
            hr = FillBuffer(pSample);
            m_evFrameCopied.Set();

            if (hr == S_OK)
            {
                hr = Deliver(pSample);
                pSample->Release();

                // downstream filter returns S_FALSE if it wants us to
                // stop or an error if it's reporting an error.
                if (hr != S_OK)
                {
                    DbgLog((LOG_TRACE, 2, TEXT("Deliver() returned %08x; stopping"), hr));
                    return S_OK;
                }

            }
            else if (hr == S_FALSE)
            {
                // derived class wants us to stop pushing data
                pSample->Release();
                DeliverEndOfStream();
                return S_OK;
            }
            else
            {
                // derived class encountered an error
                pSample->Release();
                DbgLog((LOG_ERROR, 1, TEXT("Error %08lX from FillBuffer!!!"), hr));
                DeliverEndOfStream();
                m_pFilter->NotifyEvent(EC_ERRORABORT, hr, 0);
                return hr;
            }

            // all paths release the sample
        }

        // For all commands sent to us there must be a Reply call!

        if (com == CMD_RUN || com == CMD_PAUSE)
        {
            Reply(NOERROR);
        }
        else if (com != CMD_STOP)
        {
            Reply((DWORD)E_UNEXPECTED);
            DbgLog((LOG_ERROR, 1, TEXT("Unexpected command!!!")));
        }
    } while (com != CMD_STOP);

    return S_FALSE;
}

STDMETHODIMP
GeneratorStream::NonDelegatingQueryInterface(REFIID riid, __deref_out void ** ppv)
{
    /* Do we have this interface */

    if (riid == __uuidof(IFrameReceiver)) {
        return GetInterface((IFrameReceiver *) this, ppv);
    }
    else {
        return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
    }
}

STDMETHODIMP GeneratorStream::ReceiveFrame(void *pFrameData, int iFrameDataSize, REFERENCE_TIME frameDuration)
{
    HRESULT hr = E_FAIL;
    {
        CAutoLock cAutoLockShared(&m_cSharedState);
        m_pFrameData = pFrameData;
        m_iFrameDataSize = iFrameDataSize;
        m_tFrameDuration = frameDuration;
    }
    static const int FEEDBACK_WAIT_TIMEOUT = 3000; // 3 sec
    if (m_evFrameCopied.Wait(FEEDBACK_WAIT_TIMEOUT))
    {
        // frame copied successfully
        hr = S_OK;
    }
    {
        CAutoLock cAutoLockShared(&m_cSharedState);
        m_pFrameData = NULL;
        m_iFrameDataSize = 0;
    }
    return hr;
}

STDMETHODIMP GeneratorStream::SetFrameDimensions(int nWidth, int nHeight)
{
    if (nWidth <= 0 || nHeight <= 0)
    {
        return E_INVALIDARG;
    }

    m_iImageWidth = nWidth;
    m_iImageHeight = nHeight;
    return S_OK;
}



