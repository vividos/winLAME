#pragma once

#include <map>

class LangCountryMapper
{
public:
   LangCountryMapper();
   LPCTSTR CountryCodeFromLanguageCode(UINT uiLanguageCode) const;
   int IndexFromLanguageCode(UINT uiLanguageCode) const;

public:
   std::map<CString, int> m_mapCountryCodeToIndex;
};
