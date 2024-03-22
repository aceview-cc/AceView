
#include "FreeImage.h"
#include "Utilities.h"
#include "../Metadata/FreeImageTag.h"
#include "avif/avif.h"
#include <string>
#include <thread>

// ==========================================================
// Plugin Interface
// ==========================================================

struct AvifDecoderInfo {
    avifDecoder* m_decoder;
    uint8_t* m_pFileBuf;

    AvifDecoderInfo() : m_decoder(NULL), m_pFileBuf(NULL)
    {
    }
};

static int s_format_id;

// ==========================================================
// Plugin Implementation
// ==========================================================

static const char * DLL_CALLCONV
Format() {
    return "AVIF";
}

static const char * DLL_CALLCONV
Description() {
    return "AVIF";
}

static const char * DLL_CALLCONV
Extension() {
    return "avif";
}

static const char * DLL_CALLCONV
MimeType() {
    return "image/avif";
}

static BOOL DLL_CALLCONV
Validate(FreeImageIO *io, fi_handle handle) {
    BYTE avis[] = { 0x61, 0x76, 0x69, 0x73 };	// ASCII code for "avis"
    BYTE avif[] = { 0x61, 0x76, 0x69, 0x66 };	// ASCII code for "avif"
    BYTE signature[4] = { 0, 0, 0, 0 };

    io->seek_proc(handle, 8, SEEK_SET);
    io->read_proc(signature, 1, 4, handle);

    if (memcmp(avis, signature, 4) == 0)
        return TRUE;
    if (memcmp(avif, signature, 4) == 0)
        return TRUE;

    return FALSE;
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
    return FALSE;
}

static void* DLL_CALLCONV
Open(FreeImageIO* io, fi_handle handle, BOOL read) 
{
    if (read) {
        try {
            // read Header (4 bytes)
            if (!Validate(io, handle)) {
                throw FI_MSG_ERROR_MAGIC_NUMBER;
            }

            io->seek_proc(handle, 0, SEEK_END);
            long fileSize = io->tell_proc(handle);
            io->seek_proc(handle, 0, SEEK_SET);

            uint8_t* pFileBuf = (uint8_t*)malloc(fileSize * sizeof(uint8_t));
            if (pFileBuf == NULL) {
                throw FI_MSG_ERROR_MEMORY;
            }

            if (io->read_proc(pFileBuf, 1, (unsigned)fileSize, handle) != fileSize)
            {
                free(pFileBuf);
                throw "Error while reading input stream";
            }

            // ¼ÓÔØAVIFÍ¼Ïñ
            avifDecoder* pDecoder = avifDecoderCreate();
            if (pDecoder == NULL)
            {
                free(pFileBuf);
                return NULL;
            }
            avifResult result = avifDecoderSetIOMemory(pDecoder, pFileBuf, fileSize);
            if (result != AVIF_RESULT_OK) 
            {
                free(pFileBuf);
                avifDecoderDestroy(pDecoder);
                return NULL;
            }

            result = avifDecoderParse(pDecoder);
            if (result != AVIF_RESULT_OK)
            {
                free(pFileBuf);
                avifDecoderDestroy(pDecoder);
                return NULL;
            }

            AvifDecoderInfo* info = new(std::nothrow) AvifDecoderInfo;
            if (info == NULL) {
                free(pFileBuf);
                avifDecoderDestroy(pDecoder);
                throw FI_MSG_ERROR_MEMORY;
            }
            info->m_decoder = pDecoder;
            info->m_pFileBuf = pFileBuf;

            return info;
        }
        catch (const char* msg) {
            FreeImage_OutputMessageProc(s_format_id, msg);
            return NULL;
        }
    }

    return NULL;
}

static void DLL_CALLCONV
Close(FreeImageIO* io, fi_handle handle, void* data)
{
    AvifDecoderInfo* info = (AvifDecoderInfo*)data;
    if (info != NULL)
    {
        if (info->m_decoder != NULL)
        {
            avifDecoderDestroy(info->m_decoder);
            info->m_decoder = NULL;
        }

        if (info->m_pFileBuf != NULL)
        {
            free(info->m_pFileBuf);
            info->m_pFileBuf = NULL;
        }

        delete info;
    }
}

static int DLL_CALLCONV
PageCount(FreeImageIO* io, fi_handle handle, void* data) 
{
    int count = 0;
    AvifDecoderInfo* info = (AvifDecoderInfo*)data;
    if (info != NULL)
    {
        if (info->m_decoder != NULL)
        {
            count = info->m_decoder->imageCount;
        }
    }

    return count;
}

FIBITMAP* Encode(const avifRGBImage* rgb) {

    if (rgb == NULL)
    {
        return NULL;
    }

    // load the bitmap data
    int width = rgb->width;
    int height = rgb->height;
    int rgb_stride = rgb->rowBytes;
    const uint8_t* rgb_data = rgb->pixels;

    // allocate a new dib
    FIBITMAP* dib = FreeImage_Allocate(width, height, 32);
    if (!dib)
        throw (char*)FI_MSG_ERROR_MEMORY;

    // copy the bitmap
    BYTE* pBits = FreeImage_GetBits(dib);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            BYTE bR = 0;
            BYTE bG = 0;
            BYTE bB = 0;
            BYTE bAlpha = 0;
            bR = *(rgb_data + i * rgb_stride + j * 4);
            bG = *(rgb_data + i * rgb_stride + j * 4 + 1);
            bB = *(rgb_data + i * rgb_stride + j * 4 + 2);
            bAlpha = *(rgb_data + i * rgb_stride + j * 4 + 3);

            *(pBits + (height - i - 1) * rgb_stride + j * 4) = bB;
            *(pBits + (height - i - 1) * rgb_stride + j * 4 + 1) = bG;
            *(pBits + (height - i - 1) * rgb_stride + j * 4 + 2) = bR;
            *(pBits + (height - i - 1) * rgb_stride + j * 4 + 3) = bAlpha;
        }
    }

    return dib;
}

static BOOL
FreeImage_SetMetadataAvif(FREE_IMAGE_MDMODEL model, FIBITMAP* dib, const char* key, WORD id, FREE_IMAGE_MDTYPE type, DWORD count, DWORD length, const void* value)
{
    BOOL bResult = FALSE;
    FITAG* tag = FreeImage_CreateTag();
    if (tag) {
        FreeImage_SetTagKey(tag, key);
        FreeImage_SetTagID(tag, id);
        FreeImage_SetTagType(tag, type);
        FreeImage_SetTagCount(tag, count);
        FreeImage_SetTagLength(tag, length);
        FreeImage_SetTagValue(tag, value);
        if (model == FIMD_ANIMATION) {
            TagLib& s = TagLib::instance();
            // get the tag description
            const char* description = s.getTagDescription(TagLib::ANIMATION, id);
            FreeImage_SetTagDescription(tag, description);
        }
        // store the tag
        bResult = FreeImage_SetMetadata(model, dib, key, tag);
        FreeImage_DeleteTag(tag);
    }
    return bResult;
}

static FIBITMAP * DLL_CALLCONV
Load(FreeImageIO *io, fi_handle handle, int page, int flags, void *data, FreeImage_LoadCallBack * callback) 
{
    AvifDecoderInfo* info = (AvifDecoderInfo*)data;
    if (info == NULL || info->m_decoder == NULL)
    {
        return NULL;
    }

    try 
    {
        int num_images = info->m_decoder->imageCount;
        if (num_images == 0) 
        {
            return NULL;
        }

        if (page == -1) 
        {
            page = 1;
            //return NULL;
        }

        if (page <= 0 || page > num_images) {
            return NULL;
        }

        page -= 1;

        int threadCount = std::thread::hardware_concurrency();
        if (threadCount <= 0)
        {
            threadCount = 1;
        }

        info->m_decoder->maxThreads = threadCount;

        avifResult result = avifDecoderNthImage(info->m_decoder, page);
        if (result != AVIF_RESULT_OK) {
            return NULL;
        }

        avifRGBImage rgb;
        memset(&rgb, 0, sizeof(rgb));
        avifRGBImageSetDefaults(&rgb, info->m_decoder->image);
        rgb.depth = 8;

        result = avifRGBImageAllocatePixels(&rgb);
        if (result != AVIF_RESULT_OK) 
        {
            return NULL;
        }

        result = avifImageYUVToRGB(info->m_decoder->image, &rgb);
        if (result != AVIF_RESULT_OK) 
        {
            avifRGBImageFreePixels(&rgb);
            return NULL;
        }

        FIBITMAP* dib = Encode(&rgb);
        int duration = info->m_decoder->duration * 1000 / num_images;
        FreeImage_SetMetadataAvif(FIMD_ANIMATION, dib, "FrameDuration", ANIMTAG_DURATION, FIDT_LONG, 1, 4, &duration);
        FreeImage_SetMetadataAvif(FIMD_ANIMATION, dib, "Width", ANIMTAG_LOGICALWIDTH, FIDT_LONG, 1, 4, &rgb.width);
        FreeImage_SetMetadataAvif(FIMD_ANIMATION, dib, "Height", ANIMTAG_LOGICALHEIGHT, FIDT_LONG, 1, 4, &rgb.height);
        avifRGBImageFreePixels(&rgb);

        return dib;
    }
    catch (const char* msg) {
        FreeImage_OutputMessageProc(s_format_id, msg);
        return NULL;
    }
}

// ==========================================================
//   Init
// ==========================================================

void DLL_CALLCONV
InitAVIF(Plugin *plugin, int format_id) {
    s_format_id = format_id;

    plugin->format_proc = Format;
    plugin->description_proc = Description;
    plugin->extension_proc = Extension;
    plugin->regexpr_proc = NULL;
    plugin->open_proc = Open;
    plugin->close_proc = Close;
    plugin->pagecount_proc = PageCount;
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
