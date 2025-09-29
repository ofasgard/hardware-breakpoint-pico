#include <windows.h>
 
WINBASEAPI int WINAPI USER32$MessageBoxW (HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
 
void payload () {
    USER32$MessageBoxW(NULL, L"Hello from a PICO!", L"PICO", MB_OKCANCEL);
}
