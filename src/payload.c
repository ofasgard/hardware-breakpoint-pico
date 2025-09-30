#include <windows.h>
#include <processthreadsapi.h>

// Payload that should be executed when the breakpoint is triggered. Manipulate `ExceptionInfo->ContextRecord` to mess with arguments.
// Per MSDN: "The handler should not call functions that acquire synchronization objects or allocate memory, because this can cause problems."
// https://learn.microsoft.com/en-us/windows/win32/api/winnt/nc-winnt-pvectored_exception_handler

WINBASEAPI int WINAPI USER32$MessageBoxW (HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
 
void payload (PEXCEPTION_POINTERS ExceptionInfo) {
    // This default payload will pop up a dialog box, but it will crash the process.
    USER32$MessageBoxW(NULL, L"Hello from a PICO!", L"PICO", MB_OKCANCEL);
}
