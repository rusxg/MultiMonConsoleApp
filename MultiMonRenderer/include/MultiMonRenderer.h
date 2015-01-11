#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MMR_EXPORTS
#    define MMR_API __declspec(dllexport) __cdecl
#else
#    define MMR_API __declspec(dllimport) __cdecl
#endif

// Time in 100ns ticks (10MHz clock), e.g. 400000 for 40ms PAL frame
typedef LONGLONG MMR_TIME;
#define MMR_MILLISECONDS_TO_TIME_TICKS(x) ((LONGLONG)(x) * 10000)

typedef void * MMR_HANDLE;

// Initialize multimon renderer
// Return library handle to be used in later calls
// Return NULL if initialization failed
MMR_HANDLE MMR_API MMR_Initialize();

// Get names of available video outputs.
// "names" array pointer is valid until MMR_Uninitialize is called
void MMR_API MMR_GetCards(MMR_HANDLE handle, char*** names, int *count);

// Prepare DS graph to display series of pictures to selected monitor
// "width" and "height" are the dimensions of pictures to be drawn later
// They can be different from selected monitor resolution
// Return non-zero if succeeded
int MMR_API MMR_Start(MMR_HANDLE handle, int cardIndex, int width, int height);

// Put picture data (in RGB24 format) to the rendering queue
int MMR_API MMR_SendFrame(MMR_HANDLE handle, const char* buf, int bufSize, MMR_TIME frameDuration);

// Stop rendering and destroy canvas window.
// It is safe to restart playback with MMR_Start (if needed)
void MMR_API MMR_Stop(MMR_HANDLE handle);

// Cleanup library internals.
// No functions which use given MMR_HANDLE value can be used after this call
void MMR_API MMR_Uninitialize(MMR_HANDLE handle);

#ifdef __cplusplus
}
#endif

