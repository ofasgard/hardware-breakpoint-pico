#include <windows.h>

void unhook_process(DWORD pid);

WINBASEAPI DWORD WINAPI KERNEL32$GetCurrentProcessId();
WINBASEAPI int WINAPI USER32$MessageBoxW (HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
 
void payload () {
    USER32$MessageBoxW(NULL, L"Hello from a PICO!", L"PICO", MB_OKCANCEL);
    
    DWORD pid = KERNEL32$GetCurrentProcessId();
    unhook_process(pid);
}
