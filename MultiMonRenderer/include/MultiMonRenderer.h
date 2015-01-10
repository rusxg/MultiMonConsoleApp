#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MMR_EXPORTS
#    define MMR_API __declspec(dllexport) __cdecl
#else
#    define MMR_API __declspec(dllimport) __cdecl
#endif

typedef LONGLONG MMR_TIME; // Time in 100ns ticks (10MHz clock)
#define MMR_MILLISECONDS_TO_TIME_TICKS(x) ((LONGLONG)(x) * 10000)

typedef void * MMR_HANDLE;

MMR_HANDLE MMR_API MMR_Initialize();
void       MMR_API MMR_GetCards(MMR_HANDLE handle, char*** names, int *count);
void       MMR_API MMR_Start(MMR_HANDLE handle, int cardIndex, int width, int height);
void       MMR_API MMR_SendFrame(MMR_HANDLE handle, const char* buf, int bufSize, MMR_TIME frameDuration);
void       MMR_API MMR_Stop(MMR_HANDLE handle);
void       MMR_API MMR_Uninitialize(MMR_HANDLE handle);

#ifdef __cplusplus
}
#endif

