// ==========================================================
// fipImage class implementation
//
// Design and implementation by
// - Herv?Drolon (drolon@infonie.fr)
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

#include "FreeImagePlus.h"

// 看图王集成BEGIN
#include <stdint.h>

//////////////////////////////////////////////////////////////////////////

void DLL_CALLCONV RCLoadImage_OnInit(void* handle, void* data)
{

}

//void DLL_CALLCONV RCLoadImage_SetBreak(void* handle)
//{
//    ILoadImageCallBack* callback = (ILoadImageCallBack*)handle;
//    if (callback != NULL)
//    {
//        callback->SetBreak();
//    }
//}

BOOL DLL_CALLCONV RCLoadImage_CheckBreak(void* handle)
{
    ILoadImageCallBack* callback = (ILoadImageCallBack*)handle;
    if (callback != NULL)
    {
        return (callback->CheckBreak()) ? TRUE : FALSE;
    }
    return FALSE;
}

void DLL_CALLCONV RCLoadImage_SetProgress(void* handle, uint64_t completed, uint64_t total)
{
    ILoadImageCallBack* callback = (ILoadImageCallBack*)handle;
    if (callback != NULL)
    {
        callback->SetProgress(completed, total);
    }
}
// 看图王集成END

///////////////////////////////////////////////////////////////////   
// Protected functions

BOOL fipImage::replace(FIBITMAP *new_dib) {
	if (new_dib == NULL) {
		return FALSE;
	}
	if (_dib) {
		FreeImage_Unload(_dib);
	}
	_dib = new_dib;
	_bHasChanged = TRUE;
	return TRUE;
}

///////////////////////////////////////////////////////////////////
// Creation & Destruction

fipImage::fipImage(FREE_IMAGE_TYPE image_type, unsigned width, unsigned height, unsigned bpp) {
	_dib = NULL;
	_fif = FIF_UNKNOWN;
	_bHasChanged = FALSE;
    _multiDib = NULL;
	if(width && height && bpp) {
		setSize(image_type, width, height, bpp);
	}
}

fipImage::~fipImage() {
	if(_dib) {
		FreeImage_Unload(_dib);
		_dib = NULL;
	}
    if (_multiDib) {
        _multiDib->close();
		delete _multiDib;
        _multiDib = NULL;
    }
}

BOOL fipImage::setSize(FREE_IMAGE_TYPE image_type, unsigned width, unsigned height, unsigned bpp, unsigned red_mask, unsigned green_mask, unsigned blue_mask) {
	if(_dib) {
		FreeImage_Unload(_dib);
	}
	if((_dib = FreeImage_AllocateT(image_type, width, height, bpp, red_mask, green_mask, blue_mask)) == NULL) {
		return FALSE;
	}

	if(image_type == FIT_BITMAP) {
		// Create palette if needed
		switch(bpp)	{
			case 1:
			case 4:
			case 8:
				RGBQUAD *pal = FreeImage_GetPalette(_dib);
				for(unsigned i = 0; i < FreeImage_GetColorsUsed(_dib); i++) {
					pal[i].rgbRed = i;
					pal[i].rgbGreen = i;
					pal[i].rgbBlue = i;
				}
				break;
		}
	}

	_bHasChanged = TRUE;

	return TRUE;
}

void fipImage::clear() {
	if(_dib) {
		FreeImage_Unload(_dib);
		_dib = NULL;
	}
	_bHasChanged = TRUE;
}

///////////////////////////////////////////////////////////////////
// Copying

fipImage::fipImage(const fipImage& Image) {
	_dib = NULL;
	FIBITMAP *clone = FreeImage_Clone((FIBITMAP*)Image._dib);
	replace(clone);
	_fif = Image._fif;
}

fipImage& fipImage::operator=(const fipImage& Image) {
	if(this != &Image) {
		FIBITMAP *clone = FreeImage_Clone((FIBITMAP*)Image._dib);
		replace(clone);
		_fif = Image._fif;
	}
	return *this;
}

fipImage& fipImage::operator=(FIBITMAP *dib) {
	if(_dib != dib) {
		replace(dib);
		_fif = FIF_UNKNOWN;
	}
	return *this;
}

BOOL fipImage::copySubImage(fipImage& dst, int left, int top, int right, int bottom) const {
	if(_dib) {
		dst = FreeImage_Copy(_dib, left, top, right, bottom);
		return dst.isValid();
	}
	return FALSE;
}

BOOL fipImage::pasteSubImage(fipImage& src, int left, int top, int alpha) {
	if(_dib) {
		BOOL bResult = FreeImage_Paste(_dib, src._dib, left, top, alpha);
		_bHasChanged = TRUE;
		return bResult;
	}
	return FALSE;
}

BOOL fipImage::crop(int left, int top, int right, int bottom) {
	if(_dib) {
		FIBITMAP *dst = FreeImage_Copy(_dib, left, top, right, bottom);
		return replace(dst);
	}
	return FALSE;
}

BOOL fipImage::createView(fipImage& dynamicView, unsigned left, unsigned top, unsigned right, unsigned bottom) {
	dynamicView = FreeImage_CreateView(_dib, left, top, right, bottom);
	return dynamicView.isValid();
}

BOOL fipImage::cropBlackEdge()
{
    if (_dib)
    {
        unsigned bpp = FreeImage_GetBPP(_dib) / 8;
        if (bpp != 3)
        {
            return FALSE;
        }

        BITMAPINFOHEADER* pHeader = FreeImage_GetInfoHeader(_dib);
        if (pHeader)
        {
            int yStart = 0;
            while (yStart < pHeader->biHeight)
            {
                BYTE* pBits = FreeImage_GetScanLine(_dib, yStart);

                unsigned x = 0;
                for (x = 0; x < pHeader->biWidth; x++)
                {
                    // 不是黑边
                    if ((pBits[0] != 0) || (pBits[1] != 0) || (pBits[2] != 0))
                    {
                        break;
                    }

                    pBits += bpp;
                }

                if (x < pHeader->biWidth)
                {
                    break;
                }

                ++yStart;
            }

            if (yStart == 0)
            {
                return FALSE;
            }

            int yEnd = pHeader->biHeight - 1;
            while (yEnd > yStart)
            {
                BYTE* pBits = FreeImage_GetScanLine(_dib, yEnd);

                unsigned x = 0;
                for (x = 0; x < pHeader->biWidth; x++)
                {
                    // 不是黑边
                    if ((pBits[0] != 0) || (pBits[1] != 0) || (pBits[2] != 0))
                    {
                        break;
                    }

                    pBits += bpp;
                }

                if (x < pHeader->biWidth)
                {
                    break;
                }

                --yEnd;
            }

            if (yEnd == pHeader->biHeight - 1)
            {
                return FALSE;
            }

            if (yStart < yEnd)
            {
                crop(0, yStart, pHeader->biWidth, yEnd);
            }
        }
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////
// Information functions

FREE_IMAGE_TYPE fipImage::getImageType() const {
	return FreeImage_GetImageType(_dib);
}

FREE_IMAGE_FORMAT fipImage::getFIF() const {
	return _fif;
}

unsigned fipImage::getWidth() const {
	return FreeImage_GetWidth(_dib); 
}

unsigned fipImage::getHeight() const {
	return FreeImage_GetHeight(_dib); 
}

unsigned fipImage::getScanWidth() const {
	return FreeImage_GetPitch(_dib);
}

BOOL fipImage::isValid() const {
	return (_dib != NULL) ? TRUE:FALSE;
}

BITMAPINFO* fipImage::getInfo() {
	return FreeImage_GetInfo(_dib);
}

const BITMAPINFOHEADER* fipImage::getInfoHeader() const {
	return FreeImage_GetInfoHeader(_dib);
}

unsigned fipImage::getImageSize() const {
	return FreeImage_GetDIBSize(_dib);
}

unsigned fipImage::getImageMemorySize() const {
	return FreeImage_GetMemorySize(_dib);
}

unsigned fipImage::getBitsPerPixel() const {
	return FreeImage_GetBPP(_dib);
}

unsigned fipImage::getLine() const {
	return FreeImage_GetLine(_dib);
}

double fipImage::getHorizontalResolution() const {
	return (FreeImage_GetDotsPerMeterX(_dib) / (double)100); 
}

double fipImage::getVerticalResolution() const {
	return (FreeImage_GetDotsPerMeterY(_dib) / (double)100);
}

void fipImage::setHorizontalResolution(double value) {
	FreeImage_SetDotsPerMeterX(_dib, (unsigned)(value * 100 + 0.5));
}

void fipImage::setVerticalResolution(double value) {
	FreeImage_SetDotsPerMeterY(_dib, (unsigned)(value * 100 + 0.5));
}


///////////////////////////////////////////////////////////////////
// Palette operations

RGBQUAD* fipImage::getPalette() const {
	return FreeImage_GetPalette(_dib);
}

unsigned fipImage::getPaletteSize() const {
	return FreeImage_GetColorsUsed(_dib) * sizeof(RGBQUAD);
}

unsigned fipImage::getColorsUsed() const {
	return FreeImage_GetColorsUsed(_dib);
}

FREE_IMAGE_COLOR_TYPE fipImage::getColorType() const { 
	return FreeImage_GetColorType(_dib);
}

BOOL fipImage::isGrayscale() const {
	return ((FreeImage_GetBPP(_dib) == 8) && (FreeImage_GetColorType(_dib) != FIC_PALETTE)); 
}

///////////////////////////////////////////////////////////////////
// Thumbnail access

BOOL fipImage::getThumbnail(fipImage& image) const {
	image = FreeImage_Clone( FreeImage_GetThumbnail(_dib) );
    FreeImage_CopyICCProfile(image, _dib);
	return image.isValid();
}

BOOL fipImage::setThumbnail(const fipImage& image) {
	return FreeImage_SetThumbnail(_dib, (FIBITMAP*)image._dib);
}

BOOL fipImage::hasThumbnail() const {
	return (FreeImage_GetThumbnail(_dib) != NULL);
}

BOOL fipImage::clearThumbnail() {
	return FreeImage_SetThumbnail(_dib, NULL);
}


///////////////////////////////////////////////////////////////////
// Pixel access

BYTE* fipImage::accessPixels() const {
	return FreeImage_GetBits(_dib); 
}

BYTE* fipImage::getScanLine(unsigned scanline) const {
	if(scanline < FreeImage_GetHeight(_dib)) {
		return FreeImage_GetScanLine(_dib, scanline);
	}
	return NULL;
}

BOOL fipImage::getPixelIndex(unsigned x, unsigned y, BYTE *value) const {
	return FreeImage_GetPixelIndex(_dib, x, y, value);
}

BOOL fipImage::getPixelColor(unsigned x, unsigned y, RGBQUAD *value) const {
	return FreeImage_GetPixelColor(_dib, x, y, value);
}

BOOL fipImage::setPixelIndex(unsigned x, unsigned y, BYTE *value) {
	_bHasChanged = TRUE;
	return FreeImage_SetPixelIndex(_dib, x, y, value);
}

BOOL fipImage::setPixelColor(unsigned x, unsigned y, RGBQUAD *value) {
	_bHasChanged = TRUE;
	return FreeImage_SetPixelColor(_dib, x, y, value);
}

///////////////////////////////////////////////////////////////////
// File type identification

FREE_IMAGE_FORMAT fipImage::identifyFIF(const char* lpszPathName) {
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	// check the file signature and get its format
	// (the second argument is currently not used by FreeImage)
	fif = FreeImage_GetFileType(lpszPathName, 0);
	if(fif == FIF_UNKNOWN) {
		// no signature ?
		// try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(lpszPathName);
	}

	return fif;
}

FREE_IMAGE_FORMAT fipImage::identifyFIFU(const wchar_t* lpszPathName) {
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	// check the file signature and get its format
	// (the second argument is currently not used by FreeImage)
	fif = FreeImage_GetFileTypeU(lpszPathName, 0);
	if(fif == FIF_UNKNOWN) {
		// no signature ?
		// try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilenameU(lpszPathName);
	}

	return fif;
}

FREE_IMAGE_FORMAT fipImage::identifyFIFFromHandle(FreeImageIO *io, fi_handle handle) {
	if(io && handle) {
		// check the file signature and get its format
		return FreeImage_GetFileTypeFromHandle(io, handle);
	}
	return FIF_UNKNOWN;
}

FREE_IMAGE_FORMAT fipImage::identifyFIFFromMemory(FIMEMORY *hmem) {
	if(hmem != NULL) {
		return FreeImage_GetFileTypeFromMemory(hmem, 0);
	}
	return FIF_UNKNOWN;
}


///////////////////////////////////////////////////////////////////
// Loading & Saving

BOOL fipImage::load(FREE_IMAGE_FORMAT fif, const char* lpszPathName, int flag, ILoadImageCallBack* callback) {
	// free the previous dib
	if (_dib) {
		FreeImage_Unload(_dib);
	}

    FreeImage_LoadCallBack fiCallback = { 0 };
    fiCallback.pHandle = (void*)callback;
    fiCallback.OnInitProc = RCLoadImage_OnInit;
    //fiCallback.SetBreakProc = RCLoadImage_SetBreak;
    fiCallback.CheckBreakProc = RCLoadImage_CheckBreak;
    fiCallback.SetProgressProc = RCLoadImage_SetProgress;

	// load the file
	_dib = FreeImage_Load(fif, lpszPathName, flag, &fiCallback);
	_fif = fif;
	_bHasChanged = TRUE;

	return (_dib == NULL) ? FALSE : TRUE;
}

BOOL fipImage::load(const char* lpszPathName, int flag, ILoadImageCallBack* callback) {
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	// check the file signature and get its format
	// (the second argument is currently not used by FreeImage)
	fif = FreeImage_GetFileType(lpszPathName, 0);
	if(fif == FIF_UNKNOWN) {
		// no signature ?
		// try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(lpszPathName);
	}
	// check that the plugin has reading capabilities ...
	if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
		return load(fif, lpszPathName, flag, callback);
	}

	return FALSE;
}

BOOL fipImage::loadU(FREE_IMAGE_FORMAT fif, const wchar_t* lpszPathName, int flag, ILoadImageCallBack* callback) {
    // free the previous dib
    if (_dib) {
        FreeImage_Unload(_dib);
    }
	_fif = fif;

    if (fif == FIF_WEBP || fif == FIF_AVIF) {
        if (_multiDib == NULL) {
            _multiDib = new fipMultiPage();
        }

        _multiDib->close();
        _multiDib->openU(lpszPathName, 1, 0, flag);

        ActiveFrame(1);
    }

    if (_dib == NULL) {
        // Load the file
        FreeImage_LoadCallBack fiCallback = { 0 };
        fiCallback.pHandle = (void*)callback;
        fiCallback.OnInitProc = RCLoadImage_OnInit;
        //fiCallback.SetBreakProc = RCLoadImage_SetBreak;
        fiCallback.CheckBreakProc = RCLoadImage_CheckBreak;
        fiCallback.SetProgressProc = RCLoadImage_SetProgress;
        _dib = FreeImage_LoadU(fif, lpszPathName, flag, &fiCallback);
    }

    _bHasChanged = TRUE;


	return (_dib == NULL) ? FALSE : TRUE;
}

BOOL fipImage::loadU(const wchar_t* lpszPathName, int flag, ILoadImageCallBack* callback) {
    if (callback && callback->CheckBreak())
    {
        return FALSE;
    }
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	// check the file signature and get its format
	// (the second argument is currently not used by FreeImage)
	fif = FreeImage_GetFileTypeU(lpszPathName, 0);
	if(fif == FIF_UNKNOWN) {
		// no signature ?
		// try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilenameU(lpszPathName);
	}
	// check that the plugin has reading capabilities ...
	if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
		return loadU(fif, lpszPathName, flag, callback);
	}

	return FALSE;
}

BOOL fipImage::loadFromHandle(FreeImageIO *io, fi_handle handle, int flag, ILoadImageCallBack* callback) {
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	// check the file signature and get its format
	fif = FreeImage_GetFileTypeFromHandle(io, handle);
	if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
		// Free the previous dib
		if(_dib) {
			FreeImage_Unload(_dib);			
		}

        // Load the file
        FreeImage_LoadCallBack fiCallback = { 0 };
        fiCallback.pHandle = (void*)callback;
        fiCallback.OnInitProc = RCLoadImage_OnInit;
        //fiCallback.SetBreakProc = RCLoadImage_SetBreak;
        fiCallback.CheckBreakProc = RCLoadImage_CheckBreak;
        fiCallback.SetProgressProc = RCLoadImage_SetProgress;
        _dib = FreeImage_LoadFromHandle(fif, io, handle, flag, &fiCallback);
        _bHasChanged = TRUE;

		_fif = fif;

		return (_dib == NULL) ? FALSE : TRUE;
	}
	return FALSE;
}

BOOL fipImage::loadFromMemory(fipMemoryIO& memIO, int flag, ILoadImageCallBack* callback) {
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	// check the file signature and get its format
	fif = memIO.getFileType();
	if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
		// Free the previous dib
		if(_dib) {
			FreeImage_Unload(_dib);			
		}
		// Load the file
		_dib = memIO.load(fif, flag);
		_fif = fif;
		_bHasChanged = TRUE;

		return (_dib == NULL) ? FALSE : TRUE;
	}
	return FALSE;
}

BOOL fipImage::loadFromMemory(FREE_IMAGE_FORMAT fif, fipMemoryIO& memIO, int flag, ILoadImageCallBack* callback) {
	if (fif != FIF_UNKNOWN) {
		// Free the previous dib
		if (_dib) {
			FreeImage_Unload(_dib);
		}
		// Load the file
		_dib = memIO.load(fif, flag);
		_fif = fif;
		_bHasChanged = TRUE;

		return (_dib == NULL) ? FALSE : TRUE;
	}
	return FALSE;
}

BOOL  fipImage::save(FREE_IMAGE_FORMAT fif, const char* lpszPathName, int flag) {
	BOOL bSuccess = FreeImage_Save(fif, _dib, lpszPathName, flag);
	_fif = fif;
	return bSuccess;
}

BOOL fipImage::save(const char* lpszPathName, int flag) {
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	BOOL bSuccess = FALSE;

	// Try to guess the file format from the file extension
	fif = FreeImage_GetFIFFromFilename(lpszPathName);
	if(fif != FIF_UNKNOWN ) {
		// Check that the dib can be saved in this format
		BOOL bCanSave;

		FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(_dib);
		if(image_type == FIT_BITMAP) {
			// standard bitmap type
			WORD bpp = FreeImage_GetBPP(_dib);
			bCanSave = (FreeImage_FIFSupportsWriting(fif) && FreeImage_FIFSupportsExportBPP(fif, bpp));
		} else {
			// special bitmap type
			bCanSave = FreeImage_FIFSupportsExportType(fif, image_type);
		}

		if(bCanSave) {
			bSuccess = FreeImage_Save(fif, _dib, lpszPathName, flag);
			_fif = fif;
			return bSuccess;
		}
	}
	return bSuccess;
}

BOOL  fipImage::saveU(FREE_IMAGE_FORMAT fif, const wchar_t* lpszPathName, int flag) {
	BOOL bSuccess = FreeImage_SaveU(fif, _dib, lpszPathName, flag);
	_fif = fif;
	return bSuccess;
}

BOOL fipImage::saveU(const wchar_t* lpszPathName, int flag) {
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	BOOL bSuccess = FALSE;

	// Try to guess the file format from the file extension
	fif = FreeImage_GetFIFFromFilenameU(lpszPathName);
	if(fif != FIF_UNKNOWN ) {
		// Check that the dib can be saved in this format
		BOOL bCanSave;

		FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(_dib);
		if(image_type == FIT_BITMAP) {
			// standard bitmap type
			WORD bpp = FreeImage_GetBPP(_dib);
			bCanSave = (FreeImage_FIFSupportsWriting(fif) && FreeImage_FIFSupportsExportBPP(fif, bpp));
		} else {
			// special bitmap type
			bCanSave = FreeImage_FIFSupportsExportType(fif, image_type);
		}

		if(bCanSave) {
			bSuccess = FreeImage_SaveU(fif, _dib, lpszPathName, flag);
			_fif = fif;
			return bSuccess;
		}
	}
	return bSuccess;
}

BOOL fipImage::saveToHandle(FREE_IMAGE_FORMAT fif, FreeImageIO *io, fi_handle handle, int flag) {
	BOOL bSuccess = FALSE;

	if(fif != FIF_UNKNOWN ) {
		// Check that the dib can be saved in this format
		BOOL bCanSave;

		FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(_dib);
		if(image_type == FIT_BITMAP) {
			// standard bitmap type
			WORD bpp = FreeImage_GetBPP(_dib);
			bCanSave = (FreeImage_FIFSupportsWriting(fif) && FreeImage_FIFSupportsExportBPP(fif, bpp));
		} else {
			// special bitmap type
			bCanSave = FreeImage_FIFSupportsExportType(fif, image_type);
		}

		if(bCanSave) {
			bSuccess = FreeImage_SaveToHandle(fif, _dib, io, handle, flag);
			_fif = fif;
			return bSuccess;
		}
	}
	return bSuccess;
}

BOOL fipImage::saveToMemory(FREE_IMAGE_FORMAT fif, fipMemoryIO& memIO, int flag) {
	BOOL bSuccess = FALSE;

	if(fif != FIF_UNKNOWN ) {
		// Check that the dib can be saved in this format
		BOOL bCanSave;

		FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(_dib);
		if(image_type == FIT_BITMAP) {
			// standard bitmap type
			WORD bpp = FreeImage_GetBPP(_dib);
			bCanSave = (FreeImage_FIFSupportsWriting(fif) && FreeImage_FIFSupportsExportBPP(fif, bpp));
		} else {
			// special bitmap type
			bCanSave = FreeImage_FIFSupportsExportType(fif, image_type);
		}

		if(bCanSave) {
			bSuccess = memIO.save(fif, _dib, flag);
			_fif = fif;
			return bSuccess;
		}
	}
	return bSuccess;
}

///////////////////////////////////////////////////////////////////   
// Conversion routines

BOOL fipImage::convertToType(FREE_IMAGE_TYPE image_type, BOOL scale_linear) {
	if(_dib) {
		FIBITMAP *dib = FreeImage_ConvertToType(_dib, image_type, scale_linear);
		return replace(dib);
	}
	return FALSE;
}

BOOL fipImage::threshold(BYTE T) {
	if(_dib) {
		FIBITMAP *dib1 = FreeImage_Threshold(_dib, T);
		return replace(dib1);
	}
	return FALSE;
}

BOOL fipImage::convertTo4Bits() {
	if(_dib) {
		FIBITMAP *dib4 = FreeImage_ConvertTo4Bits(_dib);
		return replace(dib4);
	}
	return FALSE;
}

BOOL fipImage::convertTo8Bits() {
	if(_dib) {
		FIBITMAP *dib8 = FreeImage_ConvertTo8Bits(_dib);
		return replace(dib8);
	}
	return FALSE;
}

BOOL fipImage::convertTo16Bits555() {
	if(_dib) {
		FIBITMAP *dib16_555 = FreeImage_ConvertTo16Bits555(_dib);
		return replace(dib16_555);
	}
	return FALSE;
}

BOOL fipImage::convertTo16Bits565() {
	if(_dib) {
		FIBITMAP *dib16_565 = FreeImage_ConvertTo16Bits565(_dib);
		return replace(dib16_565);
	}
	return FALSE;
}

BOOL fipImage::convertTo24Bits() {
	if(_dib) {
		FIBITMAP *dibRGB = FreeImage_ConvertTo24Bits(_dib);
		return replace(dibRGB);
	}
	return FALSE;
}

BOOL fipImage::convertTo32Bits() {
	if(_dib) {
		FIBITMAP *dib32 = FreeImage_ConvertTo32Bits(_dib);
		return replace(dib32);
	}
	return FALSE;
}

BOOL fipImage::convertToGrayscale() {
	if(_dib) {
		FIBITMAP *dib8 = FreeImage_ConvertToGreyscale(_dib);
		return replace(dib8);
	}
	return FALSE;
}

BOOL fipImage::colorQuantize(FREE_IMAGE_QUANTIZE algorithm) {
	if(_dib) {
		FIBITMAP *dib8 = FreeImage_ColorQuantize(_dib, algorithm);
		return replace(dib8);
	}
	return FALSE;
}

BOOL fipImage::dither(FREE_IMAGE_DITHER algorithm) {
	if(_dib) {
		FIBITMAP *dib = FreeImage_Dither(_dib, algorithm);
		return replace(dib);
	}
	return FALSE;
}

BOOL fipImage::convertToFloat() {
	if(_dib) {
		FIBITMAP *dib = FreeImage_ConvertToFloat(_dib);
		return replace(dib);
	}
	return FALSE;
}

BOOL fipImage::convertToRGBF() {
	if(_dib) {
		FIBITMAP *dib = FreeImage_ConvertToRGBF(_dib);
		return replace(dib);
	}
	return FALSE;
}

BOOL fipImage::convertToRGBAF() {
	if(_dib) {
		FIBITMAP *dib = FreeImage_ConvertToRGBAF(_dib);
		return replace(dib);
	}
	return FALSE;
}

BOOL fipImage::convertToUINT16() {
	if(_dib) {
		FIBITMAP *dib = FreeImage_ConvertToUINT16(_dib);
		return replace(dib);
	}
	return FALSE;
}

BOOL fipImage::convertToRGB16() {
	if(_dib) {
		FIBITMAP *dib = FreeImage_ConvertToRGB16(_dib);
		return replace(dib);
	}
	return FALSE;
}

BOOL fipImage::convertToRGBA16() {
	if(_dib) {
		FIBITMAP *dib = FreeImage_ConvertToRGBA16(_dib);
		return replace(dib);
	}
	return FALSE;
}

BOOL fipImage::toneMapping(FREE_IMAGE_TMO tmo, double first_param, double second_param, double third_param, double fourth_param) {
	if(_dib) {
		FIBITMAP *dst = NULL;
		// Apply a tone mapping algorithm and convert to 24-bit 
		switch(tmo) {
			case FITMO_REINHARD05:
				dst = FreeImage_TmoReinhard05Ex(_dib, first_param, second_param, third_param, fourth_param);
				break;
			default:
				dst = FreeImage_ToneMapping(_dib, tmo, first_param, second_param);
				break;
		}

		return replace(dst);
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////   
// Transparency support: background colour and alpha channel

BOOL fipImage::isTransparent() const {
	return FreeImage_IsTransparent(_dib);
}

unsigned fipImage::getTransparencyCount() const {
	return FreeImage_GetTransparencyCount(_dib);
}

BYTE* fipImage::getTransparencyTable() const {
	return FreeImage_GetTransparencyTable(_dib);
}

void fipImage::setTransparencyTable(BYTE *table, int count) {
	FreeImage_SetTransparencyTable(_dib, table, count);
	_bHasChanged = TRUE;
}

BOOL fipImage::hasFileBkColor() const {
	return FreeImage_HasBackgroundColor(_dib);
}

BOOL fipImage::getFileBkColor(RGBQUAD *bkcolor) const {
	return FreeImage_GetBackgroundColor(_dib, bkcolor);
}

BOOL fipImage::setFileBkColor(RGBQUAD *bkcolor) {
	_bHasChanged = TRUE;
	return FreeImage_SetBackgroundColor(_dib, bkcolor);
}

///////////////////////////////////////////////////////////////////   
// Channel processing support

BOOL fipImage::getChannel(fipImage& image, FREE_IMAGE_COLOR_CHANNEL channel) const {
	if(_dib) {
		image = FreeImage_GetChannel(_dib, channel);
		return image.isValid();
	}
	return FALSE;
}

BOOL fipImage::setChannel(fipImage& image, FREE_IMAGE_COLOR_CHANNEL channel) {
	if(_dib) {
		_bHasChanged = TRUE;
		return FreeImage_SetChannel(_dib, image._dib, channel);
	}
	return FALSE;
}

BOOL fipImage::splitChannels(fipImage& RedChannel, fipImage& GreenChannel, fipImage& BlueChannel) {
	if(_dib) {
		RedChannel = FreeImage_GetChannel(_dib, FICC_RED);
		GreenChannel = FreeImage_GetChannel(_dib, FICC_GREEN);
		BlueChannel = FreeImage_GetChannel(_dib, FICC_BLUE);

		return (RedChannel.isValid() && GreenChannel.isValid() && BlueChannel.isValid());
	}
	return FALSE;
}

BOOL fipImage::combineChannels(fipImage& red, fipImage& green, fipImage& blue) {
	if(!_dib) {
		int width = red.getWidth();
		int height = red.getHeight();
		_dib = FreeImage_Allocate(width, height, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
	}

	if(_dib) {
		BOOL bResult = TRUE;
		bResult &= FreeImage_SetChannel(_dib, red._dib, FICC_RED);
		bResult &= FreeImage_SetChannel(_dib, green._dib, FICC_GREEN);
		bResult &= FreeImage_SetChannel(_dib, blue._dib, FICC_BLUE);

		_bHasChanged = TRUE;

		return bResult;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////   
// Rotation and flipping

BOOL fipImage::rotateEx(double angle, double x_shift, double y_shift, double x_origin, double y_origin, BOOL use_mask) {
	if(_dib) {
		if(FreeImage_GetBPP(_dib) >= 8) {
			FIBITMAP *rotated = FreeImage_RotateEx(_dib, angle, x_shift, y_shift, x_origin, y_origin, use_mask);
			return replace(rotated);
		}
	}
	return FALSE;
}

BOOL fipImage::rotate(double angle, const void *bkcolor) {
	if(_dib) {
		switch(FreeImage_GetImageType(_dib)) {
			case FIT_BITMAP:
				switch(FreeImage_GetBPP(_dib)) {
					case 1:
					case 8:
					case 24:
					case 32:
						break;
					default:
						return FALSE;
				}
				break;

			case FIT_UINT16:
			case FIT_RGB16:
			case FIT_RGBA16:
			case FIT_FLOAT:
			case FIT_RGBF:
			case FIT_RGBAF:
				break;
			default:
				return FALSE;
				break;
		}

		FIBITMAP *rotated = FreeImage_Rotate(_dib, angle, bkcolor);
		return replace(rotated);

	}
	return FALSE;
}

BOOL fipImage::flipVertical() {
	if(_dib) {
		_bHasChanged = TRUE;

		return FreeImage_FlipVertical(_dib);
	}
	return FALSE;
}

BOOL fipImage::flipHorizontal() {
	if(_dib) {
		_bHasChanged = TRUE;

		return FreeImage_FlipHorizontal(_dib);
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////   
// Color manipulation routines

BOOL fipImage::invert() {
	if(_dib) {
		_bHasChanged = TRUE;

		return FreeImage_Invert(_dib);
	}
	return FALSE;
}

BOOL fipImage::adjustCurve(BYTE *LUT, FREE_IMAGE_COLOR_CHANNEL channel) {
	if(_dib) {
		_bHasChanged = TRUE;

		return FreeImage_AdjustCurve(_dib, LUT, channel);
	}
	return FALSE;
}

BOOL fipImage::adjustGamma(double gamma) {
	if(_dib) {
		_bHasChanged = TRUE;

		return FreeImage_AdjustGamma(_dib, gamma);
	}
	return FALSE;
}

BOOL fipImage::adjustBrightness(double percentage) {
	if(_dib) {
		_bHasChanged = TRUE;

		return FreeImage_AdjustBrightness(_dib, percentage);
	}
	return FALSE;
}

BOOL fipImage::adjustContrast(double percentage) {
	if(_dib) {
		_bHasChanged = TRUE;

		return FreeImage_AdjustContrast(_dib, percentage);
	}
	return FALSE;
}

BOOL fipImage::adjustBrightnessContrastGamma(double brightness, double contrast, double gamma) {
	if(_dib) {
		_bHasChanged = TRUE;

		return FreeImage_AdjustColors(_dib, brightness, contrast, gamma, FALSE);
	}
	return FALSE;
}

BOOL fipImage::getHistogram(DWORD *histo, FREE_IMAGE_COLOR_CHANNEL channel) const {
	if(_dib) {
		return FreeImage_GetHistogram(_dib, histo, channel);
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////
// Upsampling / downsampling routine

BOOL fipImage::rescale(unsigned new_width, unsigned new_height, FREE_IMAGE_FILTER filter) {
	if(_dib) {
		switch(FreeImage_GetImageType(_dib)) {
			case FIT_BITMAP:
			case FIT_UINT16:
			case FIT_RGB16:
			case FIT_RGBA16:
			case FIT_FLOAT:
			case FIT_RGBF:
			case FIT_RGBAF:
				break;
			default:
				return FALSE;
				break;
		}

		// Perform upsampling / downsampling
		FIBITMAP *dst = FreeImage_Rescale(_dib, new_width, new_height, filter);
		return replace(dst);
	}
	return FALSE;
}

BOOL fipImage::makeThumbnail(unsigned max_size, BOOL convert) {
	if(_dib) {
		switch(FreeImage_GetImageType(_dib)) {
			case FIT_BITMAP:
			case FIT_UINT16:
			case FIT_RGB16:
			case FIT_RGBA16:
			case FIT_FLOAT:
			case FIT_RGBF:
			case FIT_RGBAF:
				break;
			default:
				return FALSE;
				break;
		}

		// Perform downsampling
		FIBITMAP *dst = FreeImage_MakeThumbnail(_dib, max_size, convert);
		return replace(dst);
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////
// Metadata

unsigned fipImage::getMetadataCount(FREE_IMAGE_MDMODEL model) const {
	return FreeImage_GetMetadataCount(model, _dib);
}

BOOL fipImage::getMetadata(FREE_IMAGE_MDMODEL model, const char *key, fipTag& tag) const {
	FITAG *searchedTag = NULL;
	FreeImage_GetMetadata(model, _dib, key, &searchedTag);
	if(searchedTag != NULL) {
		tag = FreeImage_CloneTag(searchedTag);
		return TRUE;
	} else {
		// clear the tag
		tag = (FITAG*)NULL;
	}
	return FALSE;
}

BOOL fipImage::setMetadata(FREE_IMAGE_MDMODEL model, const char *key, fipTag& tag) {
	return FreeImage_SetMetadata(model, _dib, key, tag);
}

void fipImage::clearMetadata() {
	// clear all metadata attached to the dib
	FreeImage_SetMetadata(FIMD_COMMENTS, _dib, NULL, NULL);			// single comment or keywords
	FreeImage_SetMetadata(FIMD_EXIF_MAIN, _dib, NULL, NULL);		// Exif-TIFF metadata
	FreeImage_SetMetadata(FIMD_EXIF_EXIF, _dib, NULL, NULL);		// Exif-specific metadata
	FreeImage_SetMetadata(FIMD_EXIF_GPS, _dib, NULL, NULL);			// Exif GPS metadata
	FreeImage_SetMetadata(FIMD_EXIF_MAKERNOTE, _dib, NULL, NULL);	// Exif maker note metadata
	FreeImage_SetMetadata(FIMD_EXIF_INTEROP, _dib, NULL, NULL);		// Exif interoperability metadata
	FreeImage_SetMetadata(FIMD_IPTC, _dib, NULL, NULL);				// IPTC/NAA metadata
	FreeImage_SetMetadata(FIMD_XMP, _dib, NULL, NULL);				// Abobe XMP metadata
	FreeImage_SetMetadata(FIMD_GEOTIFF, _dib, NULL, NULL);			// GeoTIFF metadata
	FreeImage_SetMetadata(FIMD_ANIMATION, _dib, NULL, NULL);		// Animation metadata
	FreeImage_SetMetadata(FIMD_CUSTOM, _dib, NULL, NULL);			// Used to attach other metadata types to a dib
	FreeImage_SetMetadata(FIMD_EXIF_RAW, _dib, NULL, NULL);			// Exif metadata as a raw buffer
}

BOOL IsKeyFrame(int page) {
    if ((page - 1) % 3 == 0) {
        return TRUE;
    }

    return FALSE;
}

FIBITMAP* fipImage::GetOriginalFrame(fipFrameInfo& info, int page) {
    if (_multiDib == NULL) {
        return NULL;
    }

	FIBITMAP* pageDib = NULL;
	if (_fif == FIF_AVIF)
	{
		pageDib = FreeImage_LockPageByHandle(*_multiDib, page);
	}
	else
	{
		pageDib = FreeImage_LockPage(*_multiDib, page);
	}

    if (pageDib == NULL) {
        return NULL;
    }

    int width = 0;
    int height = 0;
    FITAG *tag;
    if (FreeImage_GetMetadata(FIMD_ANIMATION, pageDib, "Width", &tag)) {
        if (FreeImage_GetTagType(tag) == FIDT_LONG) {
            width = *(int *)FreeImage_GetTagValue(tag);
        }
    }
    if (FreeImage_GetMetadata(FIMD_ANIMATION, pageDib, "Height", &tag)) {
        if (FreeImage_GetTagType(tag) == FIDT_LONG) {
            height = *(int *)FreeImage_GetTagValue(tag);
        }
    }
    if (FreeImage_GetMetadata(FIMD_ANIMATION, pageDib, "FrameLeft", &tag)) {
        if (FreeImage_GetTagType(tag) == FIDT_LONG) {
            info.x = *(int *)FreeImage_GetTagValue(tag);
        }
    }
    if (FreeImage_GetMetadata(FIMD_ANIMATION, pageDib, "FrameTop", &tag)) {
        if (FreeImage_GetTagType(tag) == FIDT_LONG) {
            info.y = *(int *)FreeImage_GetTagValue(tag);
        }
    }
    if (FreeImage_GetMetadata(FIMD_ANIMATION, pageDib, "FrameDispose", &tag)) {
        if (FreeImage_GetTagType(tag) == FIDT_SHORT) {
            info.dispose = *(WORD *)FreeImage_GetTagValue(tag);
        }
    }
    if (FreeImage_GetMetadata(FIMD_ANIMATION, pageDib, "FrameBlend", &tag)) {
        if (FreeImage_GetTagType(tag) == FIDT_SHORT) {
            info.blend = *(WORD *)FreeImage_GetTagValue(tag);
        }
    }
    if (FreeImage_GetMetadata(FIMD_ANIMATION, pageDib, "FrameDuration", &tag)) {
        if (FreeImage_GetTagType(tag) == FIDT_LONG) {
            info.duration = *(int *)FreeImage_GetTagValue(tag);
        }
    }
    info.w = FreeImage_GetWidth(pageDib);
    info.h = FreeImage_GetHeight(pageDib);
    info.page = page;

    if (page == 1)
    {
        _prevFrameInfo.x = 0;
        _prevFrameInfo.y = 0;
        _prevFrameInfo.w = width;
        _prevFrameInfo.h = height;
        _prevFrameInfo.dispose = 1;
        _prevFrameInfo.blend = 0;
        _prevFrameInfo.page = 1;
    }

    return pageDib;
}

FIBITMAP* fipImage::GetFrame(int page) {
    if (_multiDib == NULL) {
        return NULL;
    }

    FIBITMAP* baseDib = NULL;
	if (_fif == FIF_AVIF)
	{
		FIBITMAP* pageDib = GetOriginalFrame(_curFrameInfo, page);
		if (pageDib != NULL)
		{
			baseDib = FreeImage_Clone(pageDib);
			FreeImage_UnlockPage(*_multiDib, pageDib, FALSE);

			if (_dib != NULL) {
				FreeImage_Unload(_dib);
				_dib = NULL;
			}
		}

		return baseDib;
	}

    fipKeyFrame fkf;
    fkf.dib = _multiDib->getKeyPage(fkf.info, page);
    if (fkf.dib != NULL) {
        if (_dib != NULL) {
            FreeImage_Unload(_dib);
            _dib = NULL;
        }
        baseDib = FreeImage_Clone(fkf.dib);
        _prevFrameInfo = fkf.info;
        return baseDib;
    }
    else {
        int basePage = page;
        if (_curFrameInfo.page + 1 == page) {
            baseDib = _dib;
        }
        else {
            if (_dib != NULL) {
                FreeImage_Unload(_dib);
                _dib = NULL;
            }

            while (basePage - 1 > 0) {
                fkf.dib = _multiDib->getKeyPage(fkf.info, basePage - 1);
                if (fkf.dib != NULL) {
                    baseDib = FreeImage_Clone(fkf.dib);
                    _prevFrameInfo = fkf.info;
                    break;
                }
                --basePage;
            }
        }

        if (baseDib == NULL) {
            FIBITMAP* pageDib = GetOriginalFrame(_curFrameInfo, basePage);
            if (pageDib == NULL) {
                return baseDib;
            }
            int width = _prevFrameInfo.w;
            int height = _prevFrameInfo.h;
            int w = FreeImage_GetWidth(pageDib);
            int h = FreeImage_GetHeight(pageDib);
            if (w != width || h != height)
            {
                RGBQUAD c;
                c.rgbRed = 0xFF;
                c.rgbGreen = 0xFF;
                c.rgbBlue = 0xFF;
                c.rgbReserved = 0x0;
                baseDib = FreeImage_EnlargeCanvas(pageDib, _curFrameInfo.x, _curFrameInfo.y, width - w - _curFrameInfo.x, height - h - _curFrameInfo.y, &c, FI_COLOR_IS_RGBA_COLOR);
			}
            else
            {
                baseDib = FreeImage_Clone(pageDib);
            }
            FreeImage_UnlockPage(*_multiDib, pageDib, FALSE);
            if (page == 1) {
                _prevFrameInfo = _curFrameInfo;
                if (IsKeyFrame(page)) {
                    _multiDib->appendKeyPage(_curFrameInfo, baseDib, page);
                }
                return baseDib;
            }
        }

        basePage--;
        while (basePage++ < page) {
            int curPage = basePage;
            FIBITMAP* pageDib = GetOriginalFrame(_curFrameInfo, curPage);
            if (pageDib == NULL) {
                return baseDib;
            }

            if (_prevFrameInfo.dispose == 1 || _curFrameInfo.blend == 1)
            {
                int xClear = 0, yClear = 0, wClear = 0, hClear = 0;
                if (_prevFrameInfo.dispose == 1)
                {
                    // Clear the previous frame rectangle
                    xClear = _prevFrameInfo.x;
                    yClear = _prevFrameInfo.y;
                    wClear = _prevFrameInfo.w;
                    hClear = _prevFrameInfo.h;
                }
                else if (_curFrameInfo.blend == 1)
                {
                    // no-blending behavior, first clearing the current frame
                    xClear = _curFrameInfo.x;
                    yClear = _curFrameInfo.y;
                    wClear = _curFrameInfo.w;
                    hClear = _curFrameInfo.h;
                }

                RGBQUAD c;
                c.rgbRed = 0xFF;
                c.rgbGreen = 0xFF;
                c.rgbBlue = 0xFF;
                c.rgbReserved = 0x0;
                FIBITMAP *clrDib = FreeImage_AllocateExT(FIT_BITMAP, wClear, hClear, FreeImage_GetBPP(pageDib), &c, FI_COLOR_IS_RGBA_COLOR);
                FreeImage_Paste(baseDib, clrDib, xClear, yClear, 255);
                FreeImage_Unload(clrDib);
            }

            if (FreeImage_GetBPP(baseDib) < FreeImage_GetBPP(pageDib))
            {
                FIBITMAP* dibT = baseDib;
                baseDib = FreeImage_ConvertTo32Bits(dibT);
                FreeImage_Unload(dibT);
            }

            // blend=0: blend, blend=1: not blend
            int alpha = (_curFrameInfo.blend == 0) ? 255 : 256;
            FreeImage_Paste(baseDib, pageDib, _curFrameInfo.x, _curFrameInfo.y, alpha, _curFrameInfo.blend == 1);
            FreeImage_UnlockPage(*_multiDib, pageDib, FALSE);

            _prevFrameInfo = _curFrameInfo;
            if (IsKeyFrame(curPage)) {
                _multiDib->appendKeyPage(_curFrameInfo, baseDib, curPage);
            }
        }
    }
    return baseDib;
}

BOOL fipImage::ActiveFrame(int page) {    if (_multiDib == NULL) {        return FALSE;    }
    FIBITMAP* dibFrame = GetFrame(page);
    if (dibFrame != NULL) {
        _prevFrameInfo = _curFrameInfo;
        _dib = dibFrame;
    }
        
    _bHasChanged = TRUE;

    return TRUE;
}

int fipImage::GetFrameDuration() {
    if (_multiDib == NULL) {        return 0;    }

    return _curFrameInfo.duration;
}
