#include <windows.h>

#include "FreeImage.h"
#include "Utilities.h"

#include "../Metadata/FreeImageTag.h"

#include "../../../LibSVG/LibSVGExports.h"

#include <string>
#include <math.h>

// ==========================================================
// Plugin Interface
// ==========================================================

static int s_format_id;

static HMODULE s_hLibSVG = NULL;
static LibSvgFuncLoadFromBuf s_funcLoadFromBuf = nullptr;
static LibSvgFuncGetBuf s_funcGetBuf = nullptr;
static LibSvgFuncGetWidth s_funcGetWidth = nullptr;
static LibSvgFuncGetHeight s_funcGetHeight = nullptr;
static LibSvgFuncGetRowstride s_funcGetRowstride = nullptr;
static LibSvgFuncGetChannel s_funcGetChannel = nullptr;

// ==========================================================
// Plugin Implementation
// ==========================================================

static const char * DLL_CALLCONV
Format() {
    return "SVG";
}

static const char * DLL_CALLCONV
Description() {
    return "SVG";
}

static const char * DLL_CALLCONV
Extension() {
    return "svg";
}

static const char * DLL_CALLCONV
MimeType() {
    return "image/svg";
}

static BOOL DLL_CALLCONV
Validate(FreeImageIO *io, fi_handle handle) {
    return TRUE;
}

static BOOL DLL_CALLCONV
SupportsExportDepth(int depth) {
    return FALSE;
}

static BOOL DLL_CALLCONV
SupportsExportType(FREE_IMAGE_TYPE type) {
    return FALSE;
}

static BOOL DLL_CALLCONV
SupportsICCProfiles() {
    return TRUE;
}

static BOOL DLL_CALLCONV
SupportsNoPixels() {
    return TRUE;
}

bool InitLibSVG(HMODULE hModule)
{
    if (s_hLibSVG != NULL)
    {
        return true;
    }

    wchar_t szPath[MAX_PATH * 2] = { 0 };
    ::GetModuleFileNameW(hModule, szPath, MAX_PATH);
    wchar_t* backslash = wcsrchr(szPath, '\\');
    if (backslash != nullptr)
    {
        *(backslash + 1) = 0;
        wcscat_s(szPath, L"LibSVG.dll");
    }
    else
    {
        wcscpy_s(szPath, L"LibSVG.dll");
    }

    // FreeImage使用动态加载的方式加载LibSVG.dll，使FreeImage的运行不依赖LibSVG.dll
    s_hLibSVG = ::LoadLibraryW(szPath);
    if (s_hLibSVG == NULL)
    {
        return false;
    }

    s_funcLoadFromBuf = (LibSvgFuncLoadFromBuf)::GetProcAddress(s_hLibSVG, "LibSvgLoadFromBuf");
    if (s_funcLoadFromBuf == nullptr)
    {
        return false;
    }
    s_funcGetBuf = (LibSvgFuncGetBuf)::GetProcAddress(s_hLibSVG, "LibSvgGetBuf");
    if (s_funcGetBuf == nullptr)
    {
        return false;
    }
    s_funcGetWidth = (LibSvgFuncGetWidth)::GetProcAddress(s_hLibSVG, "LibSvgGetWidth");
    if (s_funcGetWidth == nullptr)
    {
        return false;
    }
    s_funcGetHeight = (LibSvgFuncGetHeight)::GetProcAddress(s_hLibSVG, "LibSvgGetHeight");
    if (s_funcGetHeight == nullptr)
    {
        return false;
    }
    s_funcGetRowstride = (LibSvgFuncGetRowstride)::GetProcAddress(s_hLibSVG, "LibSvgGetRowstride");
    if (s_funcGetRowstride == nullptr)
    {
        return false;
    }
    s_funcGetChannel = (LibSvgFuncGetChannel)::GetProcAddress(s_hLibSVG, "LibSvgGetChannel");
    if (s_funcGetChannel == nullptr)
    {
        return false;
    }

    return true;
}

void UnInitLibSVG()
{
    s_funcLoadFromBuf = nullptr;
    s_funcGetBuf = nullptr;
    s_funcGetWidth = nullptr;
    s_funcGetHeight = nullptr;
    s_funcGetRowstride = nullptr;
    s_funcGetChannel = nullptr;
    if (s_hLibSVG != NULL)
    {
        ::FreeLibrary(s_hLibSVG);
        s_hLibSVG = NULL;
    }
}

bool HasLibSVG()
{
    if (s_funcLoadFromBuf == nullptr)
    {
        return false;
    }
    if (s_funcGetBuf == nullptr)
    {
        return false;
    }
    if (s_funcGetWidth == nullptr)
    {
        return false;
    }
    if (s_funcGetHeight == nullptr)
    {
        return false;
    }
    if (s_funcGetRowstride == nullptr)
    {
        return false;
    }
    if (s_funcGetChannel == nullptr)
    {
        return false;
    }

    return true;
}

// ----------------------------------------------------------
// static

FIBITMAP* LoadSVG(FreeImageIO *io, fi_handle handle, int s_format_id, int flags = 0)
{
    BOOL header_only = (flags & FIF_LOAD_NOPIXELS) == FIF_LOAD_NOPIXELS;

    FIBITMAP* dib = NULL;

    // read svg file
    long currentPos = io->tell_proc(handle);
    io->seek_proc(handle, 0, SEEK_END);
    long file_size = io->tell_proc(handle);
    io->seek_proc(handle, currentPos, SEEK_SET);
    
    void* pBuf = (uint8_t*)malloc(file_size * sizeof(BYTE));
    if (!pBuf) {
        throw FI_MSG_ERROR_MEMORY;
    }

    if (io->read_proc(pBuf, 1, (unsigned)file_size, handle) != file_size) {
        throw "Error while reading input stream";
    }

    if (!HasLibSVG())
    {
        return NULL;
    }

    auto pixbuf = s_funcLoadFromBuf(pBuf, file_size);
    if (pixbuf == nullptr)
    {
        return NULL;
    }

    auto pImgBuf = s_funcGetBuf(pixbuf);
    if (pImgBuf == nullptr)
    {
        return NULL;
    }
    auto width = s_funcGetWidth(pixbuf);
    auto height = s_funcGetHeight(pixbuf);
    auto channel = s_funcGetChannel(pixbuf);
    auto bpp = channel * 8;
    auto rowstride = s_funcGetRowstride(pixbuf);
    if (width == 0 || height == 0)
    {
        return NULL;
    }

    dib = FreeImage_Allocate(width, height, bpp);
    if (dib == nullptr)
    {
        return dib;
    }

    for (auto i = 0; i < height; ++i)
    {
        BYTE* src = pImgBuf + i * rowstride;
        BYTE* dest = FreeImage_GetScanLine(dib, height - i - 1);

        memcpy(dest, src, rowstride);

        // RGB 转 BGR
        BYTE* pRGB = dest;
        for (auto x = 0; x < rowstride; x += channel)
        {
            BYTE rgbR = *(pRGB + x);
            *(pRGB + x) = *(pRGB + x + 2);
            *(pRGB + x + 2) = rgbR;
        }
    }

    return dib;
}

static FIBITMAP * DLL_CALLCONV
Load(FreeImageIO *io, fi_handle handle, int page, int flags, void *data, FreeImage_LoadCallBack * callback) {
    if (handle) {
        return LoadSVG(io, handle, flags);
    }

    return NULL;
}

// ==========================================================
//   Init
// ==========================================================

void DLL_CALLCONV
InitSVG(Plugin *plugin, int format_id) {
    s_format_id = format_id;

    plugin->format_proc = Format;
    plugin->description_proc = Description;
    plugin->extension_proc = Extension;
    plugin->regexpr_proc = NULL;
    plugin->open_proc = NULL;
    plugin->close_proc = NULL;
    plugin->pagecount_proc = NULL;
    plugin->pagecapability_proc = NULL;
    plugin->load_proc = Load;
    plugin->save_proc = NULL;
    plugin->validate_proc = Validate;
    plugin->mime_proc = MimeType;
    plugin->supports_export_bpp_proc = SupportsExportDepth;
    plugin->supports_export_type_proc = SupportsExportType;
    plugin->supports_icc_profiles_proc = SupportsICCProfiles;
    plugin->supports_no_pixels_proc = SupportsNoPixels;
}
