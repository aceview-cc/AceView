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

#include "common/RCStringInternational.h"
#include "common/RCStringConvert.h"

#include <codecvt>

/** 声明 libintl.dll 的导出函数类型
*/
typedef char* (*i18n_bindtextdomain)(const char* __domainname, const char* __dirname);
typedef char* (*i18n_textdomain)(const char* __domainname);
typedef char* (*i18n_gettext)(const char* __msgid);

/** 定义 libintl.dll 的导出函数指针
*/
static i18n_bindtextdomain s_pfnBindTextDomain = NULL;
static i18n_textdomain s_pfnTextDomain = NULL;
static i18n_gettext s_pfnGetText = NULL;

/** 加载 libintl.dll 的句柄
*/
static HMODULE s_dllModule = NULL;

bool InternationalLoadLibrary()
{
    if (s_dllModule != NULL)
    {
        return true;
    }

    TCHAR szPath[MAX_PATH] = { 0 };
    if (!::GetModuleFileName(NULL, szPath, MAX_PATH))
    {
        return false;
    }

    TCHAR* slash = _tcsrchr(szPath, _T('\\'));
    if (slash == NULL)
    {
        return false;
    }
    _tcscpy(slash + 1, _T("libintl.dll"));

    HMODULE dllModule = ::LoadLibraryEx(szPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (dllModule != NULL)
    {
        s_pfnBindTextDomain = (i18n_bindtextdomain)::GetProcAddress(dllModule, "libintl_bindtextdomain");
        s_pfnTextDomain = (i18n_textdomain)::GetProcAddress(dllModule, "libintl_textdomain");
        s_pfnGetText = (i18n_gettext)::GetProcAddress(dllModule, "libintl_gettext");

        if (s_pfnBindTextDomain != NULL && s_pfnTextDomain != NULL && s_pfnGetText != NULL)
        {
            s_dllModule = dllModule;
        }
        else
        {
            ::FreeLibrary(dllModule);
        }
    }

    return (s_dllModule != NULL);
}

void InternationalUnloadLibrary()
{
    if (s_dllModule != NULL)
    {
        ::FreeLibrary(s_dllModule);
        s_dllModule = NULL;
        s_pfnBindTextDomain = NULL;
        s_pfnTextDomain = NULL;
        s_pfnGetText = NULL;
    }
}

bool InternationalSetLanguage(const std::string& lang, bool isInit, const std::string& rootPath)
{
    char buf[128] = { 0 };
    sprintf_s(buf, "LC_MESSAGES=%s", lang.c_str());
    _putenv(buf);

    if (isInit)
    {
        if (!rootPath.empty())
        {
            if (s_pfnBindTextDomain != NULL)
            {
                s_pfnBindTextDomain("lang", rootPath.c_str());
            }
        }
    }

    if (s_pfnTextDomain != NULL)
    {
        s_pfnTextDomain("lang");
    }
    return true;
}

std::wstring InternationalGetText(const std::string& key)
{
    if (key.empty())
    {
        return L"";
    }

    static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv_utf8_utf32;

    if (s_pfnGetText == NULL)
    {
        return conv_utf8_utf32.from_bytes(key.c_str());
    }

    return conv_utf8_utf32.from_bytes(s_pfnGetText(key.c_str()));
}

std::wstring InternationalGetTextW(const std::wstring& key)
{
    if (key.empty())
    {
        return L"";
    }

    if (s_pfnGetText == NULL)
    {
        return key;
    }

    static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
    std::string result = convert.to_bytes(key);

    return convert.from_bytes(s_pfnGetText(result.c_str()));
}
