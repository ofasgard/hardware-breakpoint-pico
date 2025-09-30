#include <windows.h>
#include <processthreadsapi.h>
#include <tlhelp32.h>

void payload();

void hook_process(DWORD pid, uintptr_t address);
LONG WINAPI ExceptionHandler();

WINBASEAPI BOOL WINAPI KERNEL32$GetThreadContext (HANDLE hThread, LPCONTEXT lpContext);
WINBASEAPI BOOL WINAPI KERNEL32$SetThreadContext (HANDLE hThread, LPCONTEXT lpContext);
WINBASEAPI PVOID WINAPI KERNEL32$AddVectoredExceptionHandler(ULONG First, PVECTORED_EXCEPTION_HANDLER Handler);
WINBASEAPI DWORD WINAPI KERNEL32$GetCurrentProcessId();
WINBASEAPI HANDLE WINAPI KERNEL32$OpenThread(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId);
WINBASEAPI BOOL WINAPI KERNEL32$CloseHandle(HANDLE hObject);
WINBASEAPI HANDLE WINAPI KERNEL32$CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID);
WINBASEAPI BOOL WINAPI KERNEL32$Thread32First(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
WINBASEAPI BOOL WINAPI KERNEL32$Thread32Next(HANDLE hSnapshot, LPTHREADENTRY32 lpte);

void go(char *target_addr) {
	// Set up the VEH.
	KERNEL32$AddVectoredExceptionHandler(1, ExceptionHandler);
	
	// Add the breakpoint on the specified address.
	DWORD pid = KERNEL32$GetCurrentProcessId();
	hook_process(pid, (uintptr_t) target_addr);
}

BOOL set_hardware_breakpoint(HANDLE thd, uintptr_t address) {
	CONTEXT context = { .ContextFlags = CONTEXT_DEBUG_REGISTERS };
	KERNEL32$GetThreadContext(thd, &context);
	
	// set the breakpoint address
	context.Dr0 = (uintptr_t) address;
	
	// set bits 0 and 1 of dr7 to '10' (enable dr0 local breakpoint)
	context.Dr7 |= 1ull << 0;
	context.Dr7 &= ~(1ull << 1);

	// set bits 16 and 17 of dr7 to '00' (set dr0 break trigger to "execute")
	context.Dr7 &= ~(1ull << 16);
	context.Dr7 &= ~(1ull << 17);

	// set bits 18 and 19 of dr7 to '00' (set dr0 break size to 1 byte)
	context.Dr7 &= ~(1ull << 18);
	context.Dr7 &= ~(1ull << 19);	

	return KERNEL32$SetThreadContext(thd, &context);
}

BOOL unset_hardware_breakpoint(HANDLE thd) {
	CONTEXT context = { .ContextFlags = CONTEXT_DEBUG_REGISTERS };
	KERNEL32$GetThreadContext(thd, &context);
	
	// unset the breakpoint address
	context.Dr0 = 0ull;
	
	// unset bit 0 of dr7
	context.Dr7 &= ~(1ull << 0);
	
	return KERNEL32$SetThreadContext(thd, &context);
}

void hook_process(DWORD pid, uintptr_t address) {
	HANDLE h = KERNEL32$CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	THREADENTRY32 te = { .dwSize = sizeof(THREADENTRY32) };
	KERNEL32$Thread32First(h, &te);
	
	do {
		if (te.th32OwnerProcessID == pid && (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))) {
			HANDLE thd = KERNEL32$OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
			set_hardware_breakpoint(thd, address);
		}
	} while (KERNEL32$Thread32Next(h, &te));
	
	KERNEL32$CloseHandle(h);
}

void unhook_process(DWORD pid) {
	HANDLE h = KERNEL32$CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	THREADENTRY32 te = { .dwSize = sizeof(THREADENTRY32) };
	KERNEL32$Thread32First(h, &te);
	
	do {
		if (te.th32OwnerProcessID == pid && (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))) {
			HANDLE thd = KERNEL32$OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
			unset_hardware_breakpoint(thd);
		}
	} while (KERNEL32$Thread32Next(h, &te));
	
	KERNEL32$CloseHandle(h);
}

LONG WINAPI ExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo) {
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP) {
		ExceptionInfo->ContextRecord->Rip = payload;
		
		ExceptionInfo->ContextRecord->EFlags |= (1 << 16); // set resume flag
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	return EXCEPTION_CONTINUE_SEARCH;
}
