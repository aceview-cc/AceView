// ==========================================================
// fipMultiPage class implementation
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

fipMultiPage::fipMultiPage(BOOL keep_cache_in_memory) : _mpage(NULL), _bMemoryCache(keep_cache_in_memory) {
    _mapKeyFrame = new std::map<int, fipKeyFrame>;
    _memIO = NULL;
}

fipMultiPage::~fipMultiPage() {
	if(_mpage) {
		// close the stream
		close(0);
	}

    if (_mapKeyFrame != NULL) {
        for (std::map<int, fipKeyFrame>::iterator iter = _mapKeyFrame->begin(); iter != _mapKeyFrame->end(); ++iter) {
            FreeImage_Unload(iter->second.dib);
        }
        _mapKeyFrame->clear();
        delete _mapKeyFrame;
    }

    if (_memIO != NULL) {
        _memIO->close();
        delete _memIO;
        _memIO = NULL;
    }
}

BOOL fipMultiPage::isValid() const {
	return (NULL != _mpage) ? TRUE : FALSE;
}

BOOL fipMultiPage::open(const char* lpszPathName, BOOL create_new, BOOL read_only, int flags) {
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;	// fif is used to get the file type

	// check if lpszPathName is a new file or an already existing file (here, we trust the 'create_new' flag)
	if (create_new) {
		fif = FreeImage_GetFIFFromFilename(lpszPathName);
	}
	else {
		fif = FreeImage_GetFileType(lpszPathName);
	}

	if (fif != FIF_UNKNOWN) {
		// open the stream
		_mpage = FreeImage_OpenMultiBitmap(fif, lpszPathName, create_new, read_only, _bMemoryCache, flags);
	}

	return (NULL != _mpage ) ? TRUE : FALSE;
}

BOOL fipMultiPage::openU(const wchar_t* lpszPathName, BOOL create_new, BOOL read_only, int flags) {
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;	// fif is used to get the file type

    // check if lpszPathName is a new file or an already existing file (here, we trust the 'create_new' flag)
    if (create_new) {
        fif = FreeImage_GetFIFFromFilenameU(lpszPathName);
    }
    else {
        fif = FreeImage_GetFileTypeU(lpszPathName);
    }

    if (fif != FIF_UNKNOWN) {
        BOOL ret = FALSE;
        FILE *handle = _wfopen(lpszPathName, L"rb");
        if (handle != NULL) {
            // if the file size less than 20M, use memio
            fseek(handle, 0, SEEK_END);
            long fileSize = ftell(handle);
            if (fileSize < 20 * 1024 * 1024) {
                fseek(handle, 0, SEEK_SET);

                BYTE* data = (BYTE*)malloc(fileSize);
                long readSize = fread(data, 1, fileSize, handle);
                if (readSize == fileSize) {
                    if (_memIO != NULL) {
                        _memIO->close();
                        delete _memIO;
                    }
                    _memIO = new fipMemoryIO(data, fileSize);

                    ret = open(*_memIO, flags);
                }
            }

            fclose(handle);
        }

        if (ret) {
            return ret;
        }

        // open the stream
        _mpage = FreeImage_OpenMultiBitmapU(fif, lpszPathName, create_new, read_only, _bMemoryCache, flags);
    }

    return (NULL != _mpage) ? TRUE : FALSE;
}

BOOL fipMultiPage::open(fipMemoryIO& memIO, int flags) {
	// try to guess the file format from the memory handle
	FREE_IMAGE_FORMAT fif = memIO.getFileType();

	// open the stream
	_mpage = memIO.loadMultiPage(fif, flags);

	return (NULL != _mpage ) ? TRUE : FALSE;
}

BOOL fipMultiPage::open(FreeImageIO *io, fi_handle handle, int flags) {
	// try to guess the file format from the handle
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromHandle(io, handle);

	// open the stream
	_mpage = FreeImage_OpenMultiBitmapFromHandle(fif, io, handle, flags);

	return (NULL != _mpage ) ? TRUE : FALSE;
}

BOOL fipMultiPage::close(int flags) {
	BOOL bSuccess = FALSE;
	if(_mpage) {
		// close the stream
		bSuccess = FreeImage_CloseMultiBitmap(_mpage, flags);
		_mpage = NULL;
	}

	return bSuccess;
}

BOOL fipMultiPage::saveToHandle(FREE_IMAGE_FORMAT fif, FreeImageIO *io, fi_handle handle, int flags) const {
	BOOL bSuccess = FALSE;
	if(_mpage) {
		bSuccess = FreeImage_SaveMultiBitmapToHandle(fif, _mpage, io, handle, flags);
	}

	return bSuccess;
}

BOOL fipMultiPage::saveToMemory(FREE_IMAGE_FORMAT fif, fipMemoryIO& memIO, int flags) const {
	BOOL bSuccess = FALSE;
	if(_mpage) {
		bSuccess = memIO.saveMultiPage(fif, _mpage, flags);
	}

	return bSuccess;
}

int fipMultiPage::getPageCount() const {
	return _mpage ? FreeImage_GetPageCount(_mpage) : 0;
}

void fipMultiPage::appendPage(fipImage& image) {
	if(_mpage) {
		FreeImage_AppendPage(_mpage, image);
	}
}

void fipMultiPage::insertPage(int page, fipImage& image) {
	if(_mpage) {
		FreeImage_InsertPage(_mpage, page, image);
	}
}

void fipMultiPage::deletePage(int page) {
	if(_mpage) {
		FreeImage_DeletePage(_mpage, page);
	}
}

BOOL fipMultiPage::movePage(int target, int source) {
	return _mpage ? FreeImage_MovePage(_mpage, target, source) : FALSE;
}

FIBITMAP* fipMultiPage::lockPage(int page) {
	return _mpage ? FreeImage_LockPage(_mpage, page) : NULL;
}

void fipMultiPage::unlockPage(fipImage& image, BOOL changed) {
	if(_mpage) {
		FreeImage_UnlockPage(_mpage, image, changed);
		// clear the image so that it becomes invalid.
		// this is possible because of the friend declaration
		image._dib = NULL;
		image._bHasChanged = FALSE;
	}
}

BOOL fipMultiPage::getLockedPageNumbers(int *pages, int *count) const {
	return _mpage ? FreeImage_GetLockedPageNumbers(_mpage, pages, count) : FALSE;
}

void fipMultiPage::appendKeyPage(const fipFrameInfo& info, FIBITMAP* dib, int page) {
    fipFrameInfo ffi;
    FIBITMAP* existDib = getKeyPage(ffi, page);
    if (existDib != NULL && dib != existDib) {
        FreeImage_Unload(existDib);
    }

    fipKeyFrame fkf;
    fkf.dib = FreeImage_Clone(dib);
    fkf.info = info;
    (*_mapKeyFrame)[page] = fkf;
}

FIBITMAP* fipMultiPage::getKeyPage(fipFrameInfo& info, int page) {
    std::map<int, fipKeyFrame>::iterator iter = _mapKeyFrame->find(page);
    if (iter != _mapKeyFrame->end()) {
        info = iter->second.info;
        return iter->second.dib;
    }

    return NULL;
}
