#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <memory.h>
#include "decoder-apng.h"

#include "../LibPNG/png.h"
#include "../LibPNG/pngstruct.h"
#include "../libpng/pnginfo.h"

struct IPngReader
{
    virtual png_size_t read(png_bytep data, png_size_t length) = 0;
};

struct IPngReader_Mem : public IPngReader
{
    const char *pbuf;
    png_size_t   nLen;
    
    IPngReader_Mem(const char *_pbuf,png_size_t _nLen):pbuf(_pbuf),nLen(_nLen){}
    png_size_t read(png_bytep data, png_size_t length)
    {
        if(nLen < length) length = nLen;
        memcpy(data,pbuf,length);
        pbuf += length;
        nLen -= length;
        return length;
    }
};

struct IPngReader_File: public IPngReader
{
    FILE *f;
    IPngReader_File(FILE *_f):f(_f){}
    
    png_size_t read(png_bytep data, png_size_t length)
    {
        return fread(data,1,length,f);
    }
};

void mypng_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   if (png_ptr == NULL)
      return;
   IPngReader * pMem = (IPngReader*)png_ptr->io_ptr;
   png_size_t rc = pMem->read(data,length);
   if(rc < length)
   {
       png_error(png_ptr,"read error");
   }
}

APNGDATA * loadPng(IPngReader *pSrc, bool onlyApng)
{
	png_bytep  dataFrame;
	png_uint_32 bytesPerRow;
	png_uint_32 bytesPerFrame;
    png_bytepp rowPointers;
	png_byte   sig[8];
	
	png_structp png_ptr_read;
	png_infop info_ptr_read;
	
    pSrc->read(sig,8);
    if(!png_check_sig(sig,8))
    {
        return NULL;
    }
    
    png_ptr_read = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    info_ptr_read = png_create_info_struct(png_ptr_read);
    
	if (setjmp(png_jmpbuf(png_ptr_read)))
    {
        png_destroy_read_struct(&png_ptr_read, &info_ptr_read, NULL);
        return NULL;
    }
        
    png_set_read_fn(png_ptr_read,pSrc,mypng_read_data);
    png_set_sig_bytes(png_ptr_read, 8);
 
    if ((png_ptr_read->bit_depth < 8) ||
        (png_ptr_read->color_type == PNG_COLOR_TYPE_PALETTE) ||
        (info_ptr_read->valid & PNG_INFO_tRNS))
        png_set_expand(png_ptr_read);

    png_set_add_alpha(png_ptr_read, 0xff, PNG_FILLER_AFTER);
    png_set_interlace_handling(png_ptr_read);
    png_set_gray_to_rgb(png_ptr_read);
    png_set_strip_16(png_ptr_read);
   
	png_read_info(png_ptr_read, info_ptr_read);
    png_read_update_info(png_ptr_read, info_ptr_read);
    
    bool isApng = true;
    if (!png_get_valid(png_ptr_read, info_ptr_read, PNG_INFO_acTL))
    {
        isApng = false;
    }

    if (!isApng && onlyApng)
    {
        png_destroy_read_struct(&png_ptr_read, &info_ptr_read, NULL);
        return NULL;
    }

    bytesPerRow = png_ptr_read->width * 4;
    bytesPerFrame = bytesPerRow * png_ptr_read->height;
    
    APNGDATA * apng = (APNGDATA*) malloc(sizeof(APNGDATA));
    memset(apng,0,sizeof(APNGDATA));
    apng->nWidth = png_ptr_read->width;
    apng->nHeight = png_ptr_read->height;
    
    //图像帧数据
    dataFrame = (png_bytep)malloc(bytesPerRow * apng->nHeight);
    memset(dataFrame,0,bytesPerFrame);
    //获得扫描行指针
    rowPointers = (png_bytepp)malloc(sizeof(png_bytep)* apng->nHeight);
    for(int i=0;i<apng->nHeight;i++)
        rowPointers[i] = dataFrame + bytesPerRow * i;

	if (!png_get_valid(png_ptr_read, info_ptr_read, PNG_INFO_acTL))
	{//load png doesn't has this trunk.
        
        png_read_image(png_ptr_read,rowPointers, NULL, NULL);
        apng->pData = dataFrame;
        apng->nFrames =1;
	}else
	{//load apng
        apng->nFrames  = png_get_num_frames(png_ptr_read, info_ptr_read);//获取总帧数

        png_bytep data = (png_bytep)malloc( bytesPerFrame * apng->nFrames);//为每一帧分配内存
        png_bytep curFrame = (png_bytep)malloc(bytesPerFrame);
        memset(curFrame,0,bytesPerFrame);
               
        apng->nLoops = png_get_num_plays(png_ptr_read, info_ptr_read);
        apng->pDelay = (unsigned short*)malloc(sizeof(unsigned short)*apng->nFrames);
        
        for(int iFrame = 0;iFrame<apng->nFrames;iFrame++)
        {
            //读帧信息头
            png_read_frame_head(png_ptr_read, info_ptr_read);
            
            //计算出帧延时信息
            if (png_get_valid(png_ptr_read, info_ptr_read, PNG_INFO_fcTL))
            {
                png_uint_16 delay_num = info_ptr_read->next_frame_delay_num,
                            delay_den = info_ptr_read->next_frame_delay_den;
            
                if (delay_den==0 || delay_den==100)
                    apng->pDelay[iFrame] = delay_num;
                else
                    if (delay_den==10)
                        apng->pDelay[iFrame] = delay_num*10;
                    else
                        if (delay_den==1000)
                            apng->pDelay[iFrame] = delay_num/10;
                        else
                            apng->pDelay[iFrame] = delay_num*100/delay_den;
            }else
            {
                apng->pDelay[iFrame] = 0;
            }
            //读取PNG帧到dataFrame中，不含偏移数据
            png_read_image(png_ptr_read, rowPointers, NULL, NULL);
            {//将当前帧数据绘制到当前显示帧中:1)获得绘制的背景；2)计算出绘制位置; 3)使用指定的绘制方式与背景混合


                //1)计算出绘制位置
                png_bytep lineDst=curFrame+info_ptr_read->next_frame_y_offset*bytesPerRow + 4 * info_ptr_read->next_frame_x_offset;
                png_bytep lineSour=dataFrame;
                //2)使用指定的绘制方式与背景混合
                switch(info_ptr_read->next_frame_blend_op)
                {
                case PNG_BLEND_OP_OVER:
                    {
                        for(unsigned int y=0;y<info_ptr_read->next_frame_height;y++)
                        {
                            png_bytep lineDst1=lineDst;
                            png_bytep lineSour1=lineSour;
                            for(unsigned int x=0;x<info_ptr_read->next_frame_width;x++)
                            {
                                png_byte alpha = lineSour1[3];
                                *lineDst1++ = ((*lineDst1)*(255-alpha)+(*lineSour1++)*alpha)>>8;
                                *lineDst1++ = ((*lineDst1)*(255-alpha)+(*lineSour1++)*alpha)>>8;
                                *lineDst1++ = ((*lineDst1)*(255-alpha)+(*lineSour1++)*alpha)>>8;
                                *lineDst1++ = ((*lineDst1)*(255-alpha)+(*lineSour1++)*alpha)>>8;
                            }
                            lineDst += bytesPerRow;
                            lineSour+= bytesPerRow;
                        }
                    }
                    break;
                case PNG_BLEND_OP_SOURCE:
                    {
                        for(unsigned int  y=0;y<info_ptr_read->next_frame_height;y++)
                        {
                            memcpy(lineDst,lineSour,info_ptr_read->next_frame_width*4);
                            lineDst += bytesPerRow;
                            lineSour+= bytesPerRow;
                        }
                    }
                    break;
                default:
                    break;
                }
                
                png_bytep targetFrame = data + bytesPerFrame * iFrame;
                memcpy(targetFrame,curFrame,bytesPerFrame);

                lineDst=curFrame+info_ptr_read->next_frame_y_offset*bytesPerRow + 4 * info_ptr_read->next_frame_x_offset;

                //3)处理当前帧绘制区域
                switch(info_ptr_read->next_frame_dispose_op)
                {
                case PNG_DISPOSE_OP_BACKGROUND://clear background
                    {
                        for(unsigned int y=0;y<info_ptr_read->next_frame_height;y++)
                        {
                            memset(lineDst,0,info_ptr_read->next_frame_width*4);
                            lineDst += bytesPerRow;
                        }

                    }
                    break;
                case PNG_DISPOSE_OP_PREVIOUS://copy previous frame
                    if(iFrame>0)
                    {
                        memcpy(curFrame,targetFrame-bytesPerFrame,bytesPerFrame);
                    }
                    break;
                case PNG_DISPOSE_OP_NONE://using current frame, doing nothing
                    break;
                default:
                    break;
                }
            }

        }
        free(curFrame);
        free(dataFrame);
        apng->pData = data;
	}
    free(rowPointers);

	png_read_end(png_ptr_read,info_ptr_read);
	
    png_destroy_read_struct(&png_ptr_read, &info_ptr_read, NULL);
    return apng;    
}

/** 解码数据帧（数据转换成 RGB格式、用指定的颜色填充背景，没指定背景颜色就用棋盘格为背景）
@param [in] pApng Apng解码的数据
@param [in] pBackGroundColor 指定背景颜色指针
*/
static void DoDecode(APNGDATA* pApng, const int* pBackGroundColor)
{
    if (pApng == NULL)
    {
        return;
    }

    int width = pApng->nWidth;
    int height = pApng->nHeight;

    //swap rgba to bgra and do premultiply

    unsigned char fgR = 0;
    unsigned char fgG = 0;
    unsigned char fgB = 0;
    unsigned char bkR = 0;
    unsigned char bkG = 0;
    unsigned char bkB = 0;
    unsigned char c = 0;

    bool hasBackGroundColor = false;
    if (pBackGroundColor != NULL)
    {
        bkR = (*pBackGroundColor) & 0xFF;
        bkG = ((*pBackGroundColor) >> 8) & 0xFF;
        bkB = ((*pBackGroundColor) >> 16) & 0xFF;
        hasBackGroundColor = true;
    }

    unsigned char* pBuf = pApng->pData;
    for (int i = 0; i < pApng->nFrames; ++i)
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                if (!hasBackGroundColor)
                {
                    //画棋盘格
                    c = (((y & 0x8) == 0) ^ ((x & 0x8) == 0)) * 220;
                    c = c ? c : 255;
                    bkR = c;
                    bkG = c;
                    bkB = c;
                }

                fgR = pBuf[2];
                fgG = pBuf[1];
                fgB = pBuf[0];

                unsigned char alpha = pBuf[3];
                if (alpha == 0)
                {
                    pBuf[0] = bkR;
                    pBuf[1] = bkG;
                    pBuf[2] = bkB;
                }
                else if (alpha == 255)
                {
                    pBuf[0] = fgR;
                    pBuf[1] = fgG;
                    pBuf[2] = fgB;
                }
                else
                {
                    unsigned char notAlpha = (unsigned char)~alpha;
                    pBuf[0] = (alpha * fgR + notAlpha * bkR) >> 8;
                    pBuf[1] = (alpha * fgG + notAlpha * bkG) >> 8;
                    pBuf[2] = (alpha * fgB + notAlpha * bkB) >> 8;
                }

                pBuf += 4;
            }
        }
    }
   
}

/** 鉴别png类别
@param [in] pszFileName 文件名
@param [in,out] pOutType 返回类别 -1 非png、 0 png、1 Apng
@return 成功返回true、失败false
*/
bool IdentifyPngType(IPngReader *pSrc, int* pOutType)
{
    if (pOutType == NULL)
    {
        return false;
    }
    *pOutType = -1;

    png_byte   sig[8];
    png_structp png_ptr_read;
    png_infop info_ptr_read;

    pSrc->read(sig, 8);
    if (!png_check_sig(sig, 8))
    {
        return true;
    }

    png_ptr_read = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr_read == NULL)
    {
        return false;
    }

    info_ptr_read = png_create_info_struct(png_ptr_read);
    if (info_ptr_read == NULL)
    {
        png_destroy_read_struct(&png_ptr_read, NULL, NULL);
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr_read)))
    {
        png_destroy_read_struct(&png_ptr_read, &info_ptr_read, NULL);
        return false;
    }

    png_set_read_fn(png_ptr_read, pSrc, mypng_read_data);
    png_set_sig_bytes(png_ptr_read, 8);
    png_read_info(png_ptr_read, info_ptr_read);

    if (!png_get_valid(png_ptr_read, info_ptr_read, PNG_INFO_acTL))
    {
        *pOutType = 0;
    }
    else
    {
        *pOutType = 1;
    }

    png_destroy_read_struct(&png_ptr_read, &info_ptr_read, NULL);
    return true;
}

/** 鉴别png类别
@param [in] pszFileName 文件名
@param [in,out] pOutType 返回类别 -1 非png、 0 png、1 Apng 
@return 成功返回true、失败false
*/
bool DLL_CALLCONV APNG_IdentifyApng(const wchar_t* pszFileName, int* pOutType)
{
    bool ret = false;
    if (pOutType == NULL)
    {
        return ret;
    }
    *pOutType = -1;

    FILE* pFile = _wfopen(pszFileName, L"rb");
    if (pFile == NULL)
    {
        return ret;
    }

    try
    {
        IPngReader_File file(pFile);
        ret = IdentifyPngType(&file, pOutType);
    }
    catch (...)
    {
        fclose(pFile);
        pFile = NULL;
        ret = false;
    }

    if (pFile != NULL)
    {
        fclose(pFile);
        pFile = NULL;
    }

    return ret;
}

APNGDATA *DLL_CALLCONV LoadAPNG_from_file(const wchar_t * pszFileName, bool onlyApng, int* pBackGroundColor)
{
    APNGDATA* pRet = NULL;
    FILE* pFile = _wfopen(pszFileName, L"rb");
    if (pFile == NULL)
    {
        return NULL;
    }

    try
    {
        IPngReader_File file(pFile);
        pRet = loadPng(&file, onlyApng);
        DoDecode(pRet, pBackGroundColor);
    }
    catch (...)
    {
        fclose(pFile);
        pFile = NULL;
        pRet = NULL;
    }
    
    if (pFile != NULL)
    {
        fclose(pFile);
        pFile = NULL;
    }
    return pRet;
}

APNGDATA *DLL_CALLCONV LoadAPNG_from_memory(const char * pBuf, size_t nLen, bool onlyApng, int* pBackGroundColor)
{
    IPngReader_Mem mem(pBuf,nLen);
    APNGDATA* pRet = loadPng(&mem, onlyApng);
    DoDecode(pRet, pBackGroundColor);
    return pRet;
}

void DLL_CALLCONV APNG_Destroy(APNGDATA* pApng)
{
    if (pApng)
    {
        if (pApng->pData) free(pApng->pData);
        if (pApng->pDelay) free(pApng->pDelay);
        free(pApng);
    }
}

APNGDATA *DLL_CALLCONV APNG_Clone(const APNGDATA* pApng)
{
    if (pApng == NULL)
    {
        return NULL;
    }

    APNGDATA* pNewApng = (APNGDATA*)malloc(sizeof(APNGDATA));
    if (pNewApng == NULL)
    {
        return NULL;
    }

    memset(pNewApng, 0, sizeof(APNGDATA));
    pNewApng->nWidth = pApng->nWidth;
    pNewApng->nHeight = pApng->nHeight;
    pNewApng->nFrames = pApng->nFrames;
    pNewApng->nLoops = pApng->nLoops;

    // delays
    int dataSize = ((sizeof(unsigned short)) * pApng->nFrames);
    pNewApng->pDelay = (unsigned short*)malloc(dataSize);
    if (pNewApng->pDelay == NULL)
    {
        APNG_Destroy(pNewApng);
    }
    memcpy(pNewApng->pDelay, pApng->pDelay, dataSize);

    // datas
    dataSize = pApng->nWidth * pApng->nHeight * 4 * pApng->nFrames;
    pNewApng->pData = (unsigned char *)malloc(dataSize);
    if (pNewApng->pData == NULL)
    {
        APNG_Destroy(pNewApng);
    }
    memcpy(pNewApng->pData, pApng->pData, dataSize);

    return pNewApng;
}
