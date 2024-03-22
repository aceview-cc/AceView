/******************************************************************************
 *  版权所有（C）2005-2008，上海瑞创网络科技发展有限公司                      *
 *  保留所有权利。                                                            *
 ******************************************************************************
 *  作者 : 李卿
 *  版本 : 1.0
 *****************************************************************************/
/*  修改记录: 
      日期       版本    修改人             修改内容
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

/** 加载 gettext 的动态库: libintl.dll
* @return 成功返回true，失败返回 false
*/
bool InternationalLoadLibrary();

/** 卸载 gettext 的动态库: libintl.dll
*/
void InternationalUnloadLibrary();

/** 设置当前语言
* @param [in] lang 当前语言名称，例：中文(简体)-"zh_CN"，英文-"en"
* @param [in] isInit 是否是初始化，如果是初始化，需要传入语言文件的根目录
* @param [in] rootPath 语言文件的根目录
* @return 返回设置成功或失败
*/
bool InternationalSetLanguage(const std::string& lang, bool isInit, const std::string& rootPath);

/** 传入字符串 key，获取当前语言对应的字符串
* @param [in] key 字符串的key
* @return 返回当前语言对应的字符串
*/
std::wstring InternationalGetText(const std::string& key);

/** 传入字符串 key，获取当前语言对应的字符串
* @param [in] key 字符串的key
* @return 返回当前语言对应的字符串
*/
std::wstring InternationalGetTextW(const std::wstring& key);

#endif //__RCStringInternational_h_

