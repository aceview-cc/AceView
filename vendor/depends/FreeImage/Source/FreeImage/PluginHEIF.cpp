
#include "FreeImage.h"
#include "Utilities.h"

#include "../Metadata/FreeImageTag.h"

#include "libheif/heif.h"
#include "libheif/heif_exif.h"

#include <string>
#include <math.h>

// ==========================================================
// Plugin Interface
// ==========================================================

static int s_format_id;

// ==========================================================
// Plugin Implementation
// ==========================================================

static const char * DLL_CALLCONV
Format() {
    return "HEIF";
}

static const char * DLL_CALLCONV
Description() {
    return "HEIF";
}

static const char * DLL_CALLCONV
Extension() {
    return "heic";
}

static const char * DLL_CALLCONV
MimeType() {
    return "image/heic";
}

static BOOL DLL_CALLCONV
Validate(FreeImageIO *io, fi_handle handle) {
    BYTE heic_id[] = { 0x68, 0x65, 0x69, 0x63 };

    BYTE signature[4] = { 0, 0, 0, 0 };

    io->seek_proc(handle, 8, SEEK_SET);
    io->read_proc(signature, 1, 4, handle);

    if (memcmp(heic_id, signature, 4) == 0)
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
    return TRUE;
}

// ----------------------------------------------------------
// static
uint8_t* ReadExifMetaData(const struct heif_image_handle* handle, size_t* size) {
    heif_item_id metadata_id;
    const char kMetadataTypeExif[] = "Exif";
    int count = heif_image_handle_get_list_of_metadata_block_IDs(handle, kMetadataTypeExif,
        &metadata_id, 1);

    for (int i = 0; i < count; i++) {
        size_t datasize = heif_image_handle_get_metadata_size(handle, metadata_id);
        uint8_t* data = static_cast<uint8_t*>(malloc(datasize));
        if (!data) {
            continue;
        }

        heif_error error = heif_image_handle_get_metadata(handle, metadata_id, data);
        if (error.code != heif_error_Ok) {
            free(data);
            continue;
        }

        *size = datasize;
        return data;
    }

    return nullptr;
}

// ----------------------------------------------------------
// static
void EncodeExif(FIBITMAP* dib, const struct heif_image_handle* handle) {
    // read exif meta data
    size_t exif_size = 0;
    uint8_t* exif_data = ReadExifMetaData(handle, &exif_size);
    if (exif_data == NULL) {
        printf("Read exif data failed.\n");
        return;
    }

    // parse exif meta data
    EXIF exif;
    if (exif.parseFromEXIFSegment(exif_data + 4, exif_size - 4) != PARSE_EXIF_SUCCESS) {
        printf("Exif parse failed.\n");
        return;
    }

    // write exif meta data to freeimage dib

    // 方向
    char buf[_MAX_PATH] = { 0 };
    FreeImage_SetMetadataKeyValue(FIMD_EXIF_MAIN, dib, "Orientation", std::to_string(exif.Orientation).c_str());

    // 拍摄时间
    FreeImage_SetMetadataKeyValue(FIMD_EXIF_MAIN, dib, "DateTime", exif.DateTime.c_str());

    // 相机厂商
    FreeImage_SetMetadataKeyValue(FIMD_EXIF_MAIN, dib, "Make", exif.Make.c_str());

    // 设备型号
    FreeImage_SetMetadataKeyValue(FIMD_EXIF_MAIN, dib, "Model", exif.Model.c_str());

    // 镜头型号
    FreeImage_SetMetadataKeyValue(FIMD_EXIF_EXIF, dib, "LensModel", exif.LensInfo.Model.c_str());

    // 光圈值
    sprintf(buf, "F%2.1f", exif.FNumber);
    FreeImage_SetMetadataKeyValue(FIMD_EXIF_EXIF, dib, "FNumber", buf);

    // 最大光圈

    // 曝光时间
    if (abs(exif.ExposureTime) < 0.000001) {
        sprintf(buf, "%d sec", 0);
    } else {
        sprintf(buf, "1/%d sec", uint32_t(1.0 / exif.ExposureTime));
    }
    FreeImage_SetMetadataKeyValue(FIMD_EXIF_EXIF, dib, "ExposureTime", buf);

    // 曝光补偿
    sprintf(buf, "%d", (int)exif.ExposureBiasValue);
    FreeImage_SetMetadataKeyValue(FIMD_EXIF_EXIF, dib, "ExposureBiasValue", buf);

    // ISO感光度
    FreeImage_SetMetadataKeyValue(FIMD_EXIF_EXIF, dib, "ISOSpeedRatings", std::to_string(exif.ISOSpeedRatings).c_str());

    // 焦距
    sprintf(buf, "%2.1f mm", exif.FocalLength);
    FreeImage_SetMetadataKeyValue(FIMD_EXIF_EXIF, dib, "FocalLength", buf);

    // 测光模式
    FreeImage_SetMetadataKeyValue(FIMD_EXIF_EXIF, dib, "MeteringMode", std::to_string(exif.MeteringMode).c_str());

    // 闪光灯
    FreeImage_SetMetadataKeyValue(FIMD_EXIF_EXIF, dib, "Flash", std::to_string(exif.Flash).c_str());

    // 白平衡
    FreeImage_SetMetadataKeyValue(FIMD_EXIF_EXIF, dib, "WhiteBalance", std::to_string(exif.WhiteBalance).c_str());

    // 亮度

    // 曝光程序
    FreeImage_SetMetadataKeyValue(FIMD_EXIF_EXIF, dib, "ExposureProgram", std::to_string(exif.ExposureProgram).c_str());

    return;
}

class ContextReleaser {
public:
    ContextReleaser(struct heif_context* ctx) : ctx_(ctx) {}
    ~ContextReleaser() {
        heif_context_free(ctx_);
    }

private:
    struct heif_context* ctx_;
};


FIBITMAP* EncodeThumbnail(BOOL header_only, const struct heif_image_handle* handle) {
    FIBITMAP* dib = NULL;

    // --- thumbnails
    int nThumbnails = heif_image_handle_get_number_of_thumbnails(handle);
    heif_item_id* thumbnailIDs = (heif_item_id*)calloc(nThumbnails, sizeof(heif_item_id));

    nThumbnails = heif_image_handle_get_list_of_thumbnail_IDs(handle, thumbnailIDs, nThumbnails);

    for (int thumbnailIdx = 0; thumbnailIdx < nThumbnails; thumbnailIdx++) {
        heif_image_handle* thumbnail_handle;
        struct heif_error err = heif_image_handle_get_thumbnail(handle, thumbnailIDs[thumbnailIdx], &thumbnail_handle);
        if (err.code) {
            free(thumbnailIDs);
            break;
        }

        int th_width = heif_image_handle_get_width(thumbnail_handle);
        int th_height = heif_image_handle_get_height(thumbnail_handle);

        printf("thumbnail: %dx%d\n", th_width, th_height);

        FIBITMAP* thumbnail_dib = FreeImage_Allocate(th_width, th_height, 32);
        if (thumbnail_dib) {
            struct heif_decoding_options* decode_options = heif_decoding_options_alloc();
            struct heif_image* thumbnail_image;
            err = heif_decode_image(thumbnail_handle,
                                    &thumbnail_image,
                                    heif_colorspace_RGB,
                                    heif_chroma_interleaved_RGBA,
                                    decode_options);
            heif_decoding_options_free(decode_options);
            if (err.code) {
                heif_image_handle_release(handle);
                break;
            }

            dib = FreeImage_AllocateHeaderT(header_only, FIT_BITMAP, th_width, th_height, 32);
            if (!dib)
                throw (char*)FI_MSG_ERROR_MEMORY;

            FreeImage_SetThumbnail(dib, thumbnail_dib);
            FreeImage_Unload(thumbnail_dib);
        }

        heif_image_handle_release(thumbnail_handle);
    }

    free(thumbnailIDs);

    return dib;
}

FIBITMAP* Encode(const struct heif_image_handle* handle,
    const struct heif_image* image ) {

    FIBITMAP* dib = NULL;

    // load the bitmap data
    int rgb_stride;
    const uint8_t* rgb_data = heif_image_get_plane_readonly(image, heif_channel_interleaved, &rgb_stride);

    int height = heif_image_get_height(image, heif_channel_interleaved);
    int width = heif_image_get_width(image, heif_channel_interleaved);

    // allocate a new dib
    dib = FreeImage_Allocate(width, height, 32);
    if (!dib) 
        throw (char*)FI_MSG_ERROR_MEMORY;

    // encode exif info
    EncodeExif(dib, handle);

    // copy the bitmap
    BYTE* bP = (BYTE*)rgb_data;
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

FIBITMAP* LoadHEIF(FreeImageIO *io, fi_handle handle, int s_format_id, int flags = 0)
{
    BOOL header_only = (flags & FIF_LOAD_NOPIXELS) == FIF_LOAD_NOPIXELS;

    FIBITMAP* dib = NULL;

    // --- read the HEIF file
    heif_encoding_options encodingoption;
    encodingoption.version = 1;

    struct heif_context* ctx = heif_context_alloc();
    if (!ctx) {
        return NULL;
    }

    ContextReleaser cr(ctx);
    struct heif_error err;

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

    err = heif_context_read_from_memory(ctx, pBuf, file_size, nullptr);
    if (err.code != 0) {
        return NULL;
    }

    int num_images = heif_context_get_number_of_top_level_images(ctx);
    if (num_images == 0) {
        return NULL;
    }

    printf("File contains %d images\n", num_images);

    heif_item_id* image_IDs = (heif_item_id*)alloca(num_images * sizeof(heif_item_id));
    num_images = heif_context_get_list_of_top_level_image_IDs(ctx, image_IDs, num_images);

    for (int idx = 0; idx < num_images; ++idx) {
        struct heif_image_handle* handle;
        err = heif_context_get_image_handle(ctx, image_IDs[idx], &handle);
        if (err.code) {
            return NULL;
        }

        int has_alpha = heif_image_handle_has_alpha_channel(handle);
        struct heif_decoding_options* decode_options = heif_decoding_options_alloc();

        int bit_depth = heif_image_handle_get_luma_bits_per_pixel(handle);
        if (bit_depth < 0) {
            heif_decoding_options_free(decode_options);
            heif_image_handle_release(handle);
            return NULL;
        }

        if (header_only) {
            // header only mode
            dib = EncodeThumbnail(header_only, handle);
            heif_image_handle_release(handle);
            return dib;
        }

        struct heif_image* image;
        err = heif_decode_image(handle,
                                &image,
                                heif_colorspace_RGB,
                                heif_chroma_interleaved_RGBA,
                                decode_options);
        heif_decoding_options_free(decode_options);
        if (err.code) {
            heif_image_handle_release(handle);
            return NULL;
        }

        if (image) {
            dib = Encode(handle, image);
            if (dib == NULL) {
                fprintf(stderr, "could not write image\n");
            } else {
                printf("Written to\n");
            }

            heif_image_release(image);

            int has_depth = heif_image_handle_has_depth_image(handle);
            if (has_depth) {
                heif_item_id depth_id;
                int nDepthImages = heif_image_handle_get_list_of_depth_image_IDs(handle, &depth_id, 1);
                assert(nDepthImages == 1);
                (void)nDepthImages;

                struct heif_image_handle* depth_handle;
                err = heif_image_handle_get_depth_image_handle(handle, depth_id, &depth_handle);
                if (err.code) {
                    heif_image_handle_release(handle);
                    return NULL;
                }

                int bit_depth = heif_image_handle_get_luma_bits_per_pixel(depth_handle);

                struct heif_image* depth_image;
                err = heif_decode_image(depth_handle,
                    &depth_image,
                    heif_colorspace_RGB,
                    heif_chroma_interleaved_RGBA,
                    nullptr);
                if (err.code) {
                    heif_image_handle_release(depth_handle);
                    heif_image_handle_release(handle);
                    return NULL;
                }
            }
            heif_image_handle_release(handle);
        }
    }

    return dib;
}

static FIBITMAP * DLL_CALLCONV
Load(FreeImageIO *io, fi_handle handle, int page, int flags, void *data, FreeImage_LoadCallBack * callback) {
    if (handle) {
        return LoadHEIF(io, handle, flags);
    }

    return NULL;
}

// ==========================================================
//   Init
// ==========================================================

void DLL_CALLCONV
InitHEIF(Plugin *plugin, int format_id) {
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
