#pragma once

class DynamicLibrary
{
public:
   DynamicLibrary(LPCTSTR pszFilename) throw()
      :m_hDll(::LoadLibrary(pszFilename))
   {
   }

   ~DynamicLibrary() throw()
   {
      FreeLibrary(m_hDll);
   }

   bool IsLoaded() const throw() { return m_hDll != NULL; }

private:
   HMODULE m_hDll;
};
