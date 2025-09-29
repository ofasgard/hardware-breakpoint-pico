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
void run_coff(WIN32FUNCS * funcs, char * srcPico);
void findNeededFunctions(WIN32FUNCS * funcs);

void go() {
    // Resolve necessary WIN32 APIs.
	WIN32FUNCS funcs;
	findNeededFunctions(&funcs);
	
	// Get a pointer to the section containing our PICO.
	char *pico = findAppendedPICO();
	
	// Run the PICO.
	run_coff(&funcs, pico);
}

// These header files must be included AFTER go() to ensure it's the first symbol in the PIC.
#include "loader.h"
#include "loaderdefs.h"
#include "picorun.h"
#include "resolve_eat.h"

void findNeededFunctions(WIN32FUNCS * funcs) {
	// Note that we resolve WIN32 APIs using ROR13 hashes of the function names, i.e. "VirtualAlloc" is 0x91AFCA54.
	char * hModule = (char *)findModuleByHash(KERNEL32DLL_HASH);

	funcs->LoadLibraryA   = (__typeof__(LoadLibraryA) *)   findFunctionByHash(hModule, LOADLIBRARYA_HASH);
	funcs->GetProcAddress = (__typeof__(GetProcAddress) *) findFunctionByHash(hModule, GETPROCADDRESS_HASH);
 	funcs->VirtualAlloc   = (__typeof__(VirtualAlloc) *)   findFunctionByHash(hModule, VIRTUALALLOC_HASH);
 	funcs->VirtualFree   = (__typeof__(VirtualFree) *)   findFunctionByHash(hModule, VIRTUALFREE_HASH);
}

void run_coff(WIN32FUNCS * funcs, char * srcPico) {
    char        * dstCode;
    char        * dstData;
     
    /* allocate memory for our PICO */
    dstData = funcs->VirtualAlloc( NULL, PicoDataSize(srcPico), MEM_RESERVE|MEM_COMMIT|MEM_TOP_DOWN,
                                                PAGE_READWRITE );
    dstCode = funcs->VirtualAlloc( NULL, PicoCodeSize(srcPico), MEM_RESERVE|MEM_COMMIT|MEM_TOP_DOWN,
                                            PAGE_EXECUTE_READWRITE );
  
    /* load our pico into our destination address, thanks! */
    PicoLoad((IMPORTFUNCS *)funcs, srcPico, dstCode, dstData);
  
    /* Call our pico entry point using the hardcoded address of VirtualFree() for demo purposes! */
    char *target_addr = (char *) funcs->VirtualFree;
    PicoEntryPoint(srcPico, dstCode)(target_addr);
  
    /* free everything... */
    funcs->VirtualFree(dstData, 0, MEM_RELEASE);
    funcs->VirtualFree(dstCode, 0, MEM_RELEASE);
}

char __PICODATA__[0] __attribute__((section("my_pico")));
  
char * findAppendedPICO() {
    return (char *)&__PICODATA__;
}
