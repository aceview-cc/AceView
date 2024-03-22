// ==========================================================
// FreeImage implementation
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
// - Herv?Drolon (drolon@infonie.fr)
// - Karl-Heinz Bussian (khbussian@moss.de)
//
// This file is part of FreeImage 3
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at your own risk!
// ==========================================================


#ifdef _WIN32
#include <windows.h>
#endif

#include <stdarg.h>

#include "FreeImage.h"
#include "Utilities.h"

#include <WinUser.h>

#include "../../resource.h"
#include "../../pThread/pthread.h"

//----------------------------------------------------------------------

static const char *s_copyright = "This program uses FreeImage, a free, open source image library supporting all common bitmap formats. See http://freeimage.sourceforge.net for details";

//----------------------------------------------------------------------

DWORD s_dIccProfileSize = 0;

void* s_pIccProfileData = NULL;

void InitIccProFileResource(HMODULE hModule);

void ReleaseIccProFileResource();

//bool InitLibSVG(HMODULE hModule);
//void UnInitLibSVG();

#if defined(_WIN32) && !defined(__MINGW32__)
#ifndef FREEIMAGE_LIB

BOOL APIENTRY
DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH :
            InitIccProFileResource((HMODULE)hModule);
            FreeImage_Initialise(TRUE);
            //InitLibSVG((HMODULE)hModule);
            pthread_win32_process_attach_np();
            break;

		case DLL_PROCESS_DETACH :
			FreeImage_DeInitialise();
            //UnInitLibSVG();
            pthread_win32_process_detach_np();
            ReleaseIccProFileResource();
            break;

		case DLL_THREAD_ATTACH :
            pthread_win32_thread_attach_np();
            break;
		case DLL_THREAD_DETACH :
            pthread_win32_thread_detach_np();
            break;
    }

    return TRUE;
}

#endif // FREEIMAGE_LIB

#else // !_WIN32 
#ifndef FREEIMAGE_LIB

void FreeImage_SO_Initialise() __attribute__((constructor));
void FreeImage_SO_DeInitialise() __attribute__((destructor));

void FreeImage_SO_Initialise() {
  FreeImage_Initialise(FALSE);
}

void FreeImage_SO_DeInitialise() {
  FreeImage_DeInitialise();
}
#endif // FREEIMAGE_LIB

#endif // _WIN32

//----------------------------------------------------------------------

const char * DLL_CALLCONV
FreeImage_GetVersion() {
	static char s_version[16];
	sprintf(s_version, "%d.%d.%d", FREEIMAGE_MAJOR_VERSION, FREEIMAGE_MINOR_VERSION, FREEIMAGE_RELEASE_SERIAL);
	return s_version;
}

const char * DLL_CALLCONV
FreeImage_GetCopyrightMessage() {
	return s_copyright;
}

//----------------------------------------------------------------------

BOOL DLL_CALLCONV
FreeImage_IsLittleEndian() {
	union {
		DWORD i;
		BYTE c[4];
	} u;
	u.i = 1;
	return (u.c[0] != 0);
}

//----------------------------------------------------------------------

static FreeImage_OutputMessageFunction freeimage_outputmessage_proc = NULL;
static FreeImage_OutputMessageFunctionStdCall freeimage_outputmessagestdcall_proc = NULL; 

void DLL_CALLCONV
FreeImage_SetOutputMessage(FreeImage_OutputMessageFunction omf) {
	freeimage_outputmessage_proc = omf;
}

void DLL_CALLCONV
FreeImage_SetOutputMessageStdCall(FreeImage_OutputMessageFunctionStdCall omf) {
	freeimage_outputmessagestdcall_proc = omf;
}

void DLL_CALLCONV
FreeImage_OutputMessageProc(int fif, const char *fmt, ...) {
	const int MSG_SIZE = 512; // 512 bytes should be more than enough for a short message

	if ((fmt != NULL) && ((freeimage_outputmessage_proc != NULL) || (freeimage_outputmessagestdcall_proc != NULL))) {
		char message[MSG_SIZE];
		memset(message, 0, MSG_SIZE);

		// initialize the optional parameter list

		va_list arg;
		va_start(arg, fmt);

		// check the length of the format string

		int str_length = (int)( (strlen(fmt) > MSG_SIZE) ? MSG_SIZE : strlen(fmt) );

		// parse the format string and put the result in 'message'

		for (int i = 0, j = 0; i < str_length; ++i) {
			if (fmt[i] == '%') {
				if (i + 1 < str_length) {
					switch(tolower(fmt[i + 1])) {
						case '%' :
							message[j++] = '%';
							break;

						case 'o' : // octal numbers
						{
							char tmp[16];

							_itoa(va_arg(arg, int), tmp, 8);

							strcat(message, tmp);

							j += (int)strlen(tmp);

							++i;

							break;
						}

						case 'i' : // decimal numbers
						case 'd' :
						{
							char tmp[16];

							_itoa(va_arg(arg, int), tmp, 10);

							strcat(message, tmp);

							j += (int)strlen(tmp);

							++i;

							break;
						}

						case 'x' : // hexadecimal numbers
						{
							char tmp[16];

							_itoa(va_arg(arg, int), tmp, 16);

							strcat(message, tmp);

							j += (int)strlen(tmp);

							++i;

							break;
						}

						case 's' : // strings
						{
							char *tmp = va_arg(arg, char*);

							strcat(message, tmp);

							j += (int)strlen(tmp);

							++i;

							break;
						}
					};
				} else {
					message[j++] = fmt[i];
				}
			} else {
				message[j++] = fmt[i];
			};
		}

		// deinitialize the optional parameter list

		va_end(arg);

		// output the message to the user program

		if (freeimage_outputmessage_proc != NULL)
			freeimage_outputmessage_proc((FREE_IMAGE_FORMAT)fif, message);

		if (freeimage_outputmessagestdcall_proc != NULL)
			freeimage_outputmessagestdcall_proc((FREE_IMAGE_FORMAT)fif, message); 
	}
}

// 看图王集成BEGIN
DWORD myGetTickCount()
{
    return ::GetTickCount();
}

void myTrace(wchar_t* pszFormat, ...)
{
    wchar_t szBuf[1024] = { 0 };
    va_list ptr;
    va_start(ptr, pszFormat);
    int nLen = _vsnwprintf_s(szBuf, 1024, 1024, pszFormat, ptr);
    va_end(ptr);
    ::OutputDebugStringW(szBuf);
}

void InitIccProFileResource(HMODULE hModule)
{
    if (s_dIccProfileSize > 0 && s_pIccProfileData != NULL)
    {
        return;
    }

    HRSRC hRsrc = ::FindResource(hModule, MAKEINTRESOURCE(IDI_ICC), L"ICC");
    if (NULL == hRsrc)
    {
        return;
    }

    DWORD dwSize = ::SizeofResource(hModule, hRsrc);
    LPBYTE lpRsrc = (LPBYTE)::LoadResource(hModule, hRsrc);
    if (dwSize > 0 && hRsrc != NULL)
    {
        s_dIccProfileSize = dwSize;
        s_pIccProfileData = lpRsrc;
    }
    ::FreeResource(hRsrc);
}

void ReleaseIccProFileResource()
{
    if (s_dIccProfileSize > 0 && s_pIccProfileData != NULL)
    {
        ::FreeResource(s_pIccProfileData);
        s_dIccProfileSize = 0;
        s_pIccProfileData = NULL;
    }
}
// 看图王集成END