#include <windows.h>

void unhook_process(DWORD pid);

WINBASEAPI DWORD WINAPI KERNEL32$GetCurrentProcessId();
WINBASEAPI int WINAPI USER32$MessageBoxW (HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
WINBASEAPI BOOL WINAPI KERNEL32$VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD  dwFreeType);

// The payload function should have the same signature as the function you're hooking.
// That way, you can reuse the arguments in registers and the stack to seamlessly call the original function when you're done.
// The example below matches the signature of `VirtualFree()`, since that's what the default runner hooks.

BOOL payload (LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType) {
    // Put whatever behaviour you want to inject here.
    USER32$MessageBoxW(NULL, L"Hello from a PICO!", L"PICO", MB_OKCANCEL);
    
    // If you don't want to remove the hook after executing the payload once, remove or modify these lines.
    DWORD pid = KERNEL32$GetCurrentProcessId();
    unhook_process(pid);
    
    // We still have the original arguments, so we can redirect execution back to VirtualFree() when we're done.
    return KERNEL32$VirtualFree(lpAddress, dwSize, dwFreeType);
}
