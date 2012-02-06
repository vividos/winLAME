#pragma once

#include <map>

class CLangCountryMapper
{
public:
   CLangCountryMapper();
   LPCTSTR CountryCodeFromLanguageCode(UINT uiLanguageCode) const;
   int IndexFromLanguageCode(UINT uiLanguageCode) const;

public:
   std::map<CString, int> m_mapCountryCodeToIndex;
};
