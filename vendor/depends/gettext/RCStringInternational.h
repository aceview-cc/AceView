/******************************************************************************
 *  ��Ȩ���У�C��2005-2008���Ϻ�������Ƽ���չ���޹�˾                      *
 *  ��������Ȩ����                                                            *
 ******************************************************************************
 *  ���� : ����
 *  �汾 : 1.0
 *****************************************************************************/
/*  �޸ļ�¼: 
      ����       �汾    �޸���             �޸�����
    --------------------------------------------------------------------------
******************************************************************************/

#ifndef __RCStringInternational_h_
#define __RCStringInternational_h_    1

#include "base/RCDefs.h"

#include "gettext/libgnuintl.h"

#include <string>
#include <locale>

#define ACE_TEXT(key) InternationalGetText(key)

#define ACE_TEXT_STR(key) (InternationalGetText(key).c_str())

#define ACE_TEXT_W(key) InternationalGetTextW(key)

#define ACE_TEXT_W_STR(key) InternationalGetTextW(key).c_str()

/** ���� gettext �Ķ�̬��: libintl.dll
* @return �ɹ�����true��ʧ�ܷ��� false
*/
bool InternationalLoadLibrary();

/** ж�� gettext �Ķ�̬��: libintl.dll
*/
void InternationalUnloadLibrary();

/** ���õ�ǰ����
* @param [in] lang ��ǰ�������ƣ���������(����)-"zh_CN"��Ӣ��-"en"
* @param [in] isInit �Ƿ��ǳ�ʼ��������ǳ�ʼ������Ҫ���������ļ��ĸ�Ŀ¼
* @param [in] rootPath �����ļ��ĸ�Ŀ¼
* @return �������óɹ���ʧ��
*/
bool InternationalSetLanguage(const std::string& lang, bool isInit, const std::string& rootPath);

/** �����ַ��� key����ȡ��ǰ���Զ�Ӧ���ַ���
* @param [in] key �ַ�����key
* @return ���ص�ǰ���Զ�Ӧ���ַ���
*/
std::wstring InternationalGetText(const std::string& key);

/** �����ַ��� key����ȡ��ǰ���Զ�Ӧ���ַ���
* @param [in] key �ַ�����key
* @return ���ص�ǰ���Զ�Ӧ���ַ���
*/
std::wstring InternationalGetTextW(const std::wstring& key);

#endif //__RCStringInternational_h_

