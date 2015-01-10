#pragma once
#include "streams.h"
#include <guiddef.h>

// {C49C7E7A-354F-414F-8032-1D091C5D13BA}
DEFINE_GUID(CLSID_GeneratorFilter,
    0xc49c7e7a, 0x354f, 0x414f, 0x80, 0x32, 0x1d, 0x9, 0x1c, 0x5d, 0x13, 0xba);

//------------------------------------------------------------------------------
// Forward Declarations
//------------------------------------------------------------------------------
// The class managing the output pin
class GeneratorStream;


//------------------------------------------------------------------------------
// Class GeneratorFilter
//
class GeneratorFilter
    : public CSource
{
public:

    // The only allowed way to create generator filter
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

private:

    // It is only allowed to to create these objects with CreateInstance
    GeneratorFilter(LPUNKNOWN lpunk, HRESULT *phr);

}; // GeneratorFilter


//------------------------------------------------------------------------------
// Class GeneratorStream
//
class GeneratorStream
    : public CSourceStream
    , public IFrameReceiver
{

public:

    GeneratorStream(HRESULT *phr, GeneratorFilter *pParent, LPCWSTR pPinName);
    ~GeneratorStream();

    HRESULT DoBufferProcessingLoop(void);    // the loop executed whilst running

    HRESULT FillBuffer(IMediaSample *pms);

    // Ask for buffers of the size appropriate to the agreed media type
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

    // Set the agreed media type, and set up the necessary ball parameters
    HRESULT SetMediaType(const CMediaType *pMediaType);

    // Because we calculate the ball there is no reason why we
    // can't calculate it in any one of a set of formats...
    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);

    // Resets the stream time to zero
    HRESULT OnThreadCreate(void);

    // Quality control notifications sent to us
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out void ** ppv);

    // IFrameReceiver
    STDMETHODIMP ReceiveFrame(void *pFrameData, int iFrameDataSize, REFERENCE_TIME frameDuration);

private:

    int m_iImageHeight;                 // The current image height
    int m_iImageWidth;                  // And current image width

    CCritSec m_cSharedState;            // Lock on m_rtSampleTime and m_Ball
    CRefTime m_rtSampleTime;            // The time stamp for each sample

    CAMEvent m_evFrameCopied;
    void * m_pFrameData;
    int m_iFrameDataSize;
    REFERENCE_TIME m_tFrameDuration;

}; // GeneratorStream
    
