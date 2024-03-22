#ifndef DECODER_APNG_H
#define DECODER_APNG_H 1

/** �Ľ�������Ҫ�ӵ�������Դ���̽������������淶�ͷ�����Ϳ�Դ���뱣��һ��
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

/** APNG���ݸ�ʽ�ṹ��
*/
struct APNGDATA
{
    /** ֡����
    */
    unsigned char * pData;

    /** ֡�ӳ�������
    */
    unsigned short *pDelay;

    /** ͼƬ�Ŀ�
    */
    int nWidth;

    /** ͼƬ�ĸ�
    */
    int nHeight;

    /** ֡��
    */
    int nFrames;

    /** ѭ������
    */
    int nLoops;
};

/** ���ļ��м���Apng
@param [in] pszFileName �ļ���
@param [in] onlyApng �Ƿ�ֻ���ض�֡��APNG
@param [in] pBackGroundColor ������ɫָ�룬���ΪNULL��������̸�
@return ���ؽ�����APNG���ݸ�ʽָ��
*/
DLL_API APNGDATA* DLL_CALLCONV LoadAPNG_from_file(const wchar_t* pszFileName, bool onlyApng, int* pBackGroundColor);

/** ����APNG
@param [in] pszFileName �ļ���
@param [in,out] pOutType ���ؾ������ -1��ʾ��png��0��ʾ��֡png��1��ʾ��֡��apng
@return �ɹ�����true,ʧ��false
*/
DLL_API bool DLL_CALLCONV APNG_IdentifyApng(const wchar_t* pszFileName, int* pOutType);

/** �����ݿ��м���Apng
@param [in] pBuf ���ݿ�ָ��
@param [in] nLen ���ݿ鳤��
@param [in] onlyApng �Ƿ�ֻ���ض�֡��APNG
@param [in] pBackGroundColor ������ɫָ�룬���ΪNULL��������̸�
@return ���ؽ�����APNG���ݸ�ʽָ��
*/
DLL_API APNGDATA* DLL_CALLCONV LoadAPNG_from_memory(const char* pBuf, size_t nLen, bool onlyApng, int* pBackGroundColor);

/** ����APNG���ݽṹ��
@param [in] pApng ��Ҫ���ٵ�APNG���ݽṹ��ָ��
*/
DLL_API void DLL_CALLCONV APNG_Destroy(APNGDATA* pApng);

/** ��¡APNG���ݽṹ��
@param [in] pApng ��Ҫ��¡��APNG���ݽṹ��ָ��
@return ���ؿ�¡���APNG���ݽṹ��ָ��
*/
DLL_API APNGDATA* DLL_CALLCONV APNG_Clone(const APNGDATA* pApng);

#endif