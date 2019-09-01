/*****************************************************************************************
Monkey's Audio MACDll.h (include for using MACDll.dll in your projects)
Copyright (C) 2000-2019 by Matthew T. Ashland   All Rights Reserved.

Overview:

Basically all this dll does is wrap MACLib.lib, so browse through MACLib.h for documentation
on how to use the interfaces.
*****************************************************************************************/

#pragma once

/*****************************************************************************************
Includes
*****************************************************************************************/
#include "All.h"
#include "MACLib.h"

/*****************************************************************************************
Defines (implemented elsewhere)
*****************************************************************************************/
namespace APE
{
	struct ID3_TAG;
}

/*****************************************************************************************
Helper functions
*****************************************************************************************/
extern "C"
{
    DLLEXPORT int __stdcall GetVersionNumber();
#ifdef PLATFORM_WINDOWS
    DLLEXPORT int __stdcall GetInterfaceCompatibility(int nVersion, BOOL bDisplayWarningsOnFailure = TRUE, HWND hwndParent = NULL);
    DLLEXPORT int __stdcall ShowFileInfoDialog(const APE::str_ansi * pFilename, HWND hwndWindow);
#endif
    DLLEXPORT int __stdcall TagFileSimple(const APE::str_ansi * pFilename, const char * pArtist, const char * pAlbum, const char * pTitle, const char * pComment, const char * pGenre, const char * pYear, const char * pTrack, BOOL bClearFirst, BOOL bUseOldID3);
	DLLEXPORT int __stdcall GetID3Tag(const APE::str_ansi * pFilename, APE::ID3_TAG * pID3Tag);
    DLLEXPORT int __stdcall RemoveTag(const APE::str_ansi * pFilename);
}

typedef int (__stdcall * proc_GetVersionNumber)();
#ifdef PLATFORM_WINDOWS
typedef int (__stdcall * proc_GetInterfaceCompatibility)(int, BOOL, HWND);
#endif

/*****************************************************************************************
IAPECompress wrapper(s)
*****************************************************************************************/
typedef void * APE_COMPRESS_HANDLE;

typedef APE_COMPRESS_HANDLE (__stdcall * proc_APECompress_Create)(int *); 
typedef void (__stdcall * proc_APECompress_Destroy)(APE_COMPRESS_HANDLE); 
typedef int (__stdcall * proc_APECompress_Start)(APE_COMPRESS_HANDLE, const char *, const APE::WAVEFORMATEX *, int, int, const void *, int);
typedef int (__stdcall * proc_APECompress_StartW)(APE_COMPRESS_HANDLE, const APE::str_utfn *, const APE::WAVEFORMATEX *, int, int, const void *, int);
typedef APE::int64 (__stdcall * proc_APECompress_AddData)(APE_COMPRESS_HANDLE, unsigned char *, int);
typedef int (__stdcall * proc_APECompress_GetBufferBytesAvailable)(APE_COMPRESS_HANDLE);
typedef unsigned char * (__stdcall * proc_APECompress_LockBuffer)(APE_COMPRESS_HANDLE, APE::int64 *);
typedef int (__stdcall * proc_APECompress_UnlockBuffer)(APE_COMPRESS_HANDLE, int, BOOL);
typedef int (__stdcall * proc_APECompress_Finish)(APE_COMPRESS_HANDLE, unsigned char *, int, int);
typedef int (__stdcall * proc_APECompress_Kill)(APE_COMPRESS_HANDLE);

extern "C"
{
    DLLEXPORT APE_COMPRESS_HANDLE __stdcall c_APECompress_Create(int * pErrorCode = NULL);
    DLLEXPORT void __stdcall c_APECompress_Destroy(APE_COMPRESS_HANDLE hAPECompress);
    DLLEXPORT int __stdcall c_APECompress_Start(APE_COMPRESS_HANDLE hAPECompress, const char * pOutputFilename, const APE::WAVEFORMATEX * pwfeInput, APE::int64 nMaxAudioBytes = MAX_AUDIO_BYTES_UNKNOWN, int nCompressionLevel = COMPRESSION_LEVEL_NORMAL, const void * pHeaderData = NULL, int nHeaderBytes = CREATE_WAV_HEADER_ON_DECOMPRESSION);
    DLLEXPORT int __stdcall c_APECompress_StartW(APE_COMPRESS_HANDLE hAPECompress, const APE::str_utfn * pOutputFilename, const APE::WAVEFORMATEX * pwfeInput, APE::int64 nMaxAudioBytes = MAX_AUDIO_BYTES_UNKNOWN, int nCompressionLevel = COMPRESSION_LEVEL_NORMAL, const void * pHeaderData = NULL, int nHeaderBytes = CREATE_WAV_HEADER_ON_DECOMPRESSION);
    DLLEXPORT APE::int64 __stdcall c_APECompress_AddData(APE_COMPRESS_HANDLE hAPECompress, unsigned char * pData, int nBytes);
    DLLEXPORT int __stdcall c_APECompress_GetBufferBytesAvailable(APE_COMPRESS_HANDLE hAPECompress);
    DLLEXPORT unsigned char * __stdcall c_APECompress_LockBuffer(APE_COMPRESS_HANDLE hAPECompress, APE::int64 * pBytesAvailable);
    DLLEXPORT int __stdcall c_APECompress_UnlockBuffer(APE_COMPRESS_HANDLE hAPECompress, int nBytesAdded, BOOL bProcess = true);
    DLLEXPORT int __stdcall c_APECompress_Finish(APE_COMPRESS_HANDLE hAPECompress, unsigned char * pTerminatingData, int nTerminatingBytes, int nWAVTerminatingBytes);
    DLLEXPORT int __stdcall c_APECompress_Kill(APE_COMPRESS_HANDLE hAPECompress);
}

/*****************************************************************************************
IAPEDecompress wrapper(s)
*****************************************************************************************/
typedef void * APE_DECOMPRESS_HANDLE;

typedef APE_DECOMPRESS_HANDLE (__stdcall * proc_APEDecompress_Create)(const char *, int *); 
typedef APE_DECOMPRESS_HANDLE (__stdcall * proc_APEDecompress_CreateW)(const char *, int *); 
typedef void (__stdcall * proc_APEDecompress_Destroy)(APE_DECOMPRESS_HANDLE); 
typedef int (__stdcall * proc_APEDecompress_GetData)(APE_DECOMPRESS_HANDLE, char *, APE::intn, APE::intn *);
typedef int (__stdcall * proc_APEDecompress_Seek)(APE_DECOMPRESS_HANDLE, int); 
typedef int (__stdcall * proc_APEDecompress_GetInfo)(APE_DECOMPRESS_HANDLE, APE::APE_DECOMPRESS_FIELDS, int, int);

extern "C"
{
    DLLEXPORT APE_DECOMPRESS_HANDLE __stdcall c_APEDecompress_Create(const APE::str_ansi * pFilename, int * pErrorCode = NULL);
    DLLEXPORT APE_DECOMPRESS_HANDLE __stdcall c_APEDecompress_CreateW(const APE::str_utfn * pFilename, int * pErrorCode = NULL);
    DLLEXPORT void __stdcall c_APEDecompress_Destroy(APE_DECOMPRESS_HANDLE hAPEDecompress);
    DLLEXPORT int __stdcall c_APEDecompress_GetData(APE_DECOMPRESS_HANDLE hAPEDecompress, char * pBuffer, APE::intn nBlocks, APE::intn * pBlocksRetrieved);
    DLLEXPORT int __stdcall c_APEDecompress_Seek(APE_DECOMPRESS_HANDLE hAPEDecompress, int nBlockOffset);
    DLLEXPORT int __stdcall c_APEDecompress_GetInfo(APE_DECOMPRESS_HANDLE hAPEDecompress, APE::APE_DECOMPRESS_FIELDS Field, int nParam1 = 0, int nParam2 = 0);
}
