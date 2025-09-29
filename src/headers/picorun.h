/*
 * Copyright (C) 2025 Raphael Mudge, Adversary Fan Fiction Writers Guild
 *
 * This file is part of Tradecraft Garden
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, see <https://www.gnu.org/licenses/>.
 */

typedef struct {
	int codeLength;
	int dataLength;
	int rsrcOffset;
	int entryAddress;
} PICO_HDR;

#define FIRST_PICO_DIRECTIVE(x) (PICO_DIRECTIVE_HDR *)((void *)x + sizeof(PICO_HDR))
#define NEXT_PICO_DIRECTIVE(x)  (PICO_DIRECTIVE_HDR *)((void *)x + x->length);

#define PICO_INST_COMPLETE   0x0
#define PICO_INST_PATCH      0x1
#define PICO_INST_COPY       0x2
#define PICO_INST_LL         0x3
#define PICO_INST_GPA        0x4
#define PICO_INST_PATCH_DIFF 0x5
#define PICO_INST_PATCH_FUNC 0x6

#define PICO_PATCH_TEXT_TEXT 0x0
#define PICO_PATCH_TEXT_BASE 0x1
#define PICO_PATCH_BASE_TEXT 0x2
#define PICO_PATCH_BASE_BASE 0x3

#define PICO_PATCHF_FUNC     0x0

#define PICO_CONTEXT_CODE    0x5
#define PICO_CONTEXT_DATA    0x6

typedef struct {
	char  type;
	char  option;
	short length;
} PICO_DIRECTIVE_HDR;

typedef struct {
	PICO_DIRECTIVE_HDR hdr;
	int offset;
} PICO_DIRECTIVE_PATCH;

typedef struct {
	PICO_DIRECTIVE_HDR hdr;
	int src_offset;
	int dst_offset;
	int total;
} PICO_DIRECTIVE_COPY;

typedef void (*PICOMAIN_FUNC)(char * arg);

PICOMAIN_FUNC PicoEntryPoint(char * src, char * base) {
	PICO_HDR * hdr = (PICO_HDR *)src;
	return (PICOMAIN_FUNC)( (char *)base + hdr->entryAddress );
}

int PicoCodeSize(char * src) {
	return ( (PICO_HDR *)src )->codeLength;
}

int PicoDataSize(char * src) {
	return ( (PICO_HDR *)src )->dataLength;
}

void PicoLoad(IMPORTFUNCS * funcs, char * src, char * dstCode, char * dstData) {
	PICO_DIRECTIVE_HDR   * entry;
	PICO_DIRECTIVE_PATCH * patch;
	PICO_DIRECTIVE_COPY  * copy;
	HANDLE                 module;
	char                 * address;
	PICO_HDR             * hdr = (PICO_HDR *)src;

	entry = FIRST_PICO_DIRECTIVE(hdr);
	while (TRUE) {
		/*
		 * The heart and soul of PICO loading. Patching pointers into our destination blob
		 * to make sure everything works as hoped and expected. x86 doesn't do indirect addressing
		 * so there's a lot more patches there. But x64 needs some pointer patching too.
		 */
		if (entry->type == PICO_INST_PATCH) {
			ULONG_PTR   value;
			ULONG_PTR   src;
			patch = (PICO_DIRECTIVE_PATCH *)entry;

			if (entry->option == PICO_PATCH_TEXT_TEXT) {
				src   = (ULONG_PTR)dstCode;
				value = (ULONG_PTR)dstCode;
			}
			else if (entry->option == PICO_PATCH_TEXT_BASE) {
				src   = (ULONG_PTR)dstCode;
				value = (ULONG_PTR)dstData;
			}
			else if (entry->option == PICO_PATCH_BASE_TEXT) {
				src   = (ULONG_PTR)dstData;
				value = (ULONG_PTR)dstCode;
			}
			else if (entry->option == PICO_PATCH_BASE_BASE) {
				src   = (ULONG_PTR)dstData;
				value = (ULONG_PTR)dstData;
			}

			/* get the existing offset (from whatever base) within the .text section */
			value += *(ULONG_PTR *)(src + patch->offset);

			/* set it back */
			*(ULONG_PTR *)(src + patch->offset) = value;
		}
		/*
		 * This block is for updating our function table sitting in our data section. We're
		 * either dumping the last resolved address at some specific slot OR we're pulling in
		 * a pre-determined internal API (which is presumed to be an overloaded IMPORTFUNCS
		 * structure... which we're treating as an array of function pointers basically)
		 */
		else if (entry->type == PICO_INST_PATCH_FUNC) {
			ULONG_PTR value;
			patch = (PICO_DIRECTIVE_PATCH *)entry;

			if (entry->option == PICO_PATCHF_FUNC) {
				value = (ULONG_PTR)address;
			}
			else {
				ULONG_PTR * table = (ULONG_PTR *)funcs;
				value = table[entry->option - 1];
			}

			*(ULONG_PTR *)(dstData + patch->offset) = value;
		}
		/*
		 * This is here to support keeping code + data in separate regions in x64 builds.
		 */
#ifdef WIN_X64
		else if (entry->type == PICO_INST_PATCH_DIFF) {
			DWORD value;
			patch = (PICO_DIRECTIVE_PATCH *)entry;

			/* fetch the value currently at the patch address */
			value   = *(DWORD *)(dstCode + patch->offset);

			/* adjust the value */
			value  += (ULONG_PTR)dstData - (ULONG_PTR)dstCode;

			/* set it back */
			*(DWORD *)(dstCode + patch->offset) = value;
		}
#endif
		/*
		 * Directive copies from our packed src to our destination. We do it this way to allow
		 * our .text section to pack down to its raw value and we expand it to its page-aligned
		 * size after.
		 */
		else if (entry->type == PICO_INST_COPY) {
			copy  = (PICO_DIRECTIVE_COPY *)entry;
			char * dst;

			/* make sure we're copying to the right context */
			if (entry->option == PICO_CONTEXT_CODE)
				dst = dstCode;
			else
				dst = dstData;

			/* do our copy */
			__movsb((unsigned char *)dst + copy->dst_offset, (unsigned char *)src + hdr->rsrcOffset + copy->src_offset, copy->total);
		}
		/*
		 * Directive does a LoadLibraryA() to set our handle. Used as a precursor to any
		 * GetProcAddress lookups based on this later on.
		 */
		else if (entry->type == PICO_INST_LL) {
			char * arg = (char *)entry + sizeof(PICO_DIRECTIVE_HDR);
			module = funcs->LoadLibraryA(arg);
		}
		/*
		 * Call GetProcAddress on an argument. A precursor to a PATCH_FUNC instruction to push
		 * this pointer to the appropriate spot within our PICO blob.
		 */
		else if (entry->type == PICO_INST_GPA) {
			char * arg = (char *)entry + sizeof(PICO_DIRECTIVE_HDR);
			address = (char *)funcs->GetProcAddress(module, arg);
		}
		/*
		 * An instruction to indicate the loading is complete and we should return.
		 */
		else if (entry->type == PICO_INST_COMPLETE) {
			return;
		}

		entry = NEXT_PICO_DIRECTIVE(entry);
	}
}
