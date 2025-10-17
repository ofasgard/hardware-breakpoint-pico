#include <windows.h>
#include "breakpoint_hook.h"
#include "tcg.h"

WINBASEAPI HMODULE WINAPI KERNEL32$LoadLibraryA(LPCSTR lpLibFileName);
WINBASEAPI LPVOID WINAPI KERNEL32$GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
WINBASEAPI LPVOID WINAPI KERNEL32$VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
WINBASEAPI BOOL WINAPI KERNEL32$VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD  dwFreeType);

typedef struct {
    __typeof__(LoadLibraryA)   * LoadLibraryA;
    __typeof__(GetProcAddress) * GetProcAddress;
    __typeof__(VirtualAlloc)   * VirtualAlloc;
    __typeof__(VirtualFree)    * VirtualFree;
} WIN32FUNCS;

char * resolve(DWORD modHash, DWORD funcHash) {
    char * hModule = (char *)findModuleByHash(modHash);
    return findFunctionByHash(hModule, funcHash);
}

void run_hwbp_pico(WIN32FUNCS * funcs, char * srcPico, char *target_addr) {
    char        * dstCode;
    char        * dstData;
     
    /* allocate memory for our PICO */
    dstData = funcs->VirtualAlloc( NULL, PicoDataSize(srcPico), MEM_RESERVE|MEM_COMMIT|MEM_TOP_DOWN,
                                                PAGE_READWRITE );
    dstCode = funcs->VirtualAlloc( NULL, PicoCodeSize(srcPico), MEM_RESERVE|MEM_COMMIT|MEM_TOP_DOWN,
                                            PAGE_EXECUTE_READWRITE );
  
    /* load our pico into our destination address, thanks! */
    PicoLoad((IMPORTFUNCS *)funcs, srcPico, dstCode, dstData);
  
    /* Call our pico entry point using the provided address. */
    BREAKPOINT_HOOK_PICO entryPoint = (BREAKPOINT_HOOK_PICO) PicoEntryPoint(srcPico, dstCode);
    entryPoint(target_addr);
  
    /* free everything... */
    funcs->VirtualFree(dstData, 0, MEM_RELEASE);
    funcs->VirtualFree(dstCode, 0, MEM_RELEASE);
}

char __PICODATA__[0] __attribute__((section("hwbp_pico")));
  
char * findAppendedPICO() {
    return (char *)&__PICODATA__;
}

void go() {
    	// Resolve necessary WIN32 APIs.
	WIN32FUNCS funcs;
	funcs.LoadLibraryA = KERNEL32$LoadLibraryA;
	funcs.GetProcAddress = KERNEL32$GetProcAddress;
	funcs.VirtualAlloc = KERNEL32$VirtualAlloc;
	funcs.VirtualFree = KERNEL32$VirtualFree;
	
	// Get a pointer to the section containing our PICO.
	char *pico = findAppendedPICO();
	
	// Resolve the address of the function we want to patch. In this case, it's VirtualFree().
	char *target_addr = KERNEL32$VirtualFree;
	
	// Run the PICO.
	run_hwbp_pico(&funcs, pico, target_addr);
}

