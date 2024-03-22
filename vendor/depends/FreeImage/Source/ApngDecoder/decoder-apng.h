#ifndef DECODER_APNG_H
#define DECODER_APNG_H 1

/** 改解码器主要从第三方开源工程借鉴过来、代码规范和风格尽量和开源代码保持一致
*/
#if defined(FREEIMAGE_LIB)
    #define DLL_API
    #define DLL_CALLCONV
#else
    #define DLL_CALLCONV __stdcall
    #ifdef FREEIMAGE_EXPORTS
         #define DLL_API __declspec(dllexport)
    #else
         #define DLL_API __declspec(dllimport)
    #endif // FREEIMAGE_EXPORTS
#endif // FREEIMAGE_LIB

/** APNG数据格式结构体
*/
struct APNGDATA
{
    /** 帧数据
    */
    unsigned char * pData;

    /** 帧延迟数据区
    */
    unsigned short *pDelay;

    /** 图片的宽
    */
    int nWidth;

    /** 图片的高
    */
    int nHeight;

    /** 帧数
    */
    int nFrames;

    /** 循环次数
    */
    int nLoops;
};

/** 从文件中加载Apng
@param [in] pszFileName 文件名
@param [in] onlyApng 是否只加载多帧的APNG
@param [in] pBackGroundColor 背景颜色指针，如果为NULL则采用棋盘格
@return 返回解码后的APNG数据格式指针
*/
DLL_API APNGDATA* DLL_CALLCONV LoadAPNG_from_file(const wchar_t* pszFileName, bool onlyApng, int* pBackGroundColor);

/** 鉴别APNG
@param [in] pszFileName 文件名
@param [in,out] pOutType 返回具体类别 -1表示非png、0表示单帧png、1表示多帧的apng
@return 成功返回true,失败false
*/
DLL_API bool DLL_CALLCONV APNG_IdentifyApng(const wchar_t* pszFileName, int* pOutType);

/** 从数据块中加载Apng
@param [in] pBuf 数据块指针
@param [in] nLen 数据块长度
@param [in] onlyApng 是否只加载多帧的APNG
@param [in] pBackGroundColor 背景颜色指针，如果为NULL则采用棋盘格
@return 返回解码后的APNG数据格式指针
*/
DLL_API APNGDATA* DLL_CALLCONV LoadAPNG_from_memory(const char* pBuf, size_t nLen, bool onlyApng, int* pBackGroundColor);

/** 销毁APNG数据结构体
@param [in] pApng 需要销毁的APNG数据结构体指针
*/
DLL_API void DLL_CALLCONV APNG_Destroy(APNGDATA* pApng);

/** 克隆APNG数据结构体
@param [in] pApng 需要克隆的APNG数据结构体指针
@return 返回克隆后的APNG数据结构体指针
*/
DLL_API APNGDATA* DLL_CALLCONV APNG_Clone(const APNGDATA* pApng);

#endif