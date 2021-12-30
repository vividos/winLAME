//
// winLAME - a frontend for the LAME encoding engine
// Copyright (c) 2000-2021 Michael Fink
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
/// \file HttpClient.cpp
/// \brief HTTP client
//
#include "stdafx.h"
#include "HttpClient.hpp"
#include <ulib/Exception.hpp>
#include <ulib/thread/Event.hpp>

#pragma comment(lib, "Urlmon.lib")

class HttpCallback : public IBindStatusCallback
{
public:
   HttpCallback()
      :m_event(false)
   {
   }

   HttpResponse Wait()
   {
      m_event.Wait();
      return m_message;
   }

private:
   // Inherited via IBindStatusCallback
   virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override
   {
      return E_NOTIMPL;
   }
   virtual ULONG __stdcall AddRef(void) override
   {
      return 1;
   }
   virtual ULONG __stdcall Release(void) override
   {
      return 1;
   }
   virtual HRESULT __stdcall OnStartBinding(DWORD dwReserved, IBinding* pib) override
   {
      return E_NOTIMPL;
   }
   virtual HRESULT __stdcall GetPriority(LONG* pnPriority) override
   {
      return E_NOTIMPL;
   }
   virtual HRESULT __stdcall OnLowResource(DWORD reserved) override
   {
      return E_NOTIMPL;
   }
   virtual HRESULT __stdcall OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText) override
   {
      return E_NOTIMPL;
   }
   virtual HRESULT __stdcall OnStopBinding(HRESULT hresult, LPCWSTR szError) override
   {
      if (hresult == S_OK)
      {
      }
      else
      {
         //CString errorMessage{ szError };

         //if (!errorMessage.IsEmpty())
         //   m_message.m_errorMessage = errorMessage;
      }

      m_event.Set();

      return S_OK;
   }

   virtual HRESULT __stdcall GetBindInfo(DWORD* grfBINDF, BINDINFO* pbindinfo) override
   {
      return E_NOTIMPL;
   }
   virtual HRESULT __stdcall OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC* pformatetc, STGMEDIUM* pstgmed) override
   {
      if ((grfBSCF & BSCF_LASTDATANOTIFICATION) == 0)
         return S_OK; // wait for the last notification

      // as URLOpenStream was used, an IStream is returned
      if (pstgmed != nullptr &&
         pstgmed->tymed == TYMED_ISTREAM)
      {
         CComPtr<IStream> stream;
         stream.Attach(pstgmed->pstm);

         m_message.m_responseData.resize(dwSize);
         ULONG numReadBytes = 0;
         HRESULT hr = stream->Read(m_message.m_responseData.data(), dwSize, &numReadBytes);
         if (!SUCCEEDED(hr))
         {
            m_message.m_errorText = "Error reading from IStream";
            m_message.m_statusCode = 500;
         }
         else
            m_message.m_statusCode = 200;

         m_event.Set();
      }

      return S_OK;
   }
   virtual HRESULT __stdcall OnObjectAvailable(REFIID riid, IUnknown* punk) override
   {
      return E_NOTIMPL;
   }

private:
   ManualResetEvent m_event;
   HttpResponse m_message;
};

HttpResponse HttpClient::Request(const CString& requestUrl)
{
   HttpCallback callback;

   HRESULT hr = URLOpenStream(
      nullptr,
      requestUrl,
      0,
      &callback);

   ATLVERIFY(SUCCEEDED(hr));

   if (FAILED(hr))
      return HttpResponse(500);

   return callback.Wait();
}
