/*!
*	\author	Kjetil Haga (programmer.khaga@start.no)
*/

/*! \ingroup encoder */
/*! @{ */

// prevent multiple including
#ifndef __wlmonkeysaudioinputmodule_h_
#define __wlmonkeysaudioinputmodule_h_

// needed includes
#include "ModuleInterface.h"
#include <string>



// monkey namespace contains internal stuff used in MonkeysAudioInputModule
namespace monkey
{
	#include "mac/MACDll.h"
	#include "mac/APETag.h"

	// typedefs
	typedef int (__stdcall * proc_APEGetID3Tag)(const char*, ID3_TAG*);

	/// internal struct with pointers to mac dll functions
	struct MAC_DLL
	{
		MAC_DLL() : module(0), Create(0), Destroy(0), GetData(0), Seek(0), GetInfo(0)
		{
		}

		~MAC_DLL()
		{
			if(module)
			{
				::FreeLibrary(module);
			}
		}

		// Module
		HMODULE								module;
		
		// APEDecompress functions
		proc_APEDecompress_Create			Create;
		proc_APEDecompress_Destroy			Destroy;
		proc_APEDecompress_GetData			GetData;
		proc_APEDecompress_Seek				Seek;
		proc_APEDecompress_GetInfo			GetInfo;
		proc_APEGetID3Tag					GetID3Tag;
	};
}



//! monkey's audio input module
class MonkeysAudioInputModule : public InputModule
{
public:
	/// ctor
	MonkeysAudioInputModule();

	/// clones input module
   virtual InputModule *cloneModule();

   /// returns the module name
   virtual CString getModuleName(){ return _T("Monkey's Audio File Decoder"); }

   /// returns the last error
   virtual CString getLastError(){ return lasterror; }

   /// returns if the module is available
   virtual bool isAvailable();

   /// returns description of current file
   virtual void getDescription(CString& desc);

   /// returns version string
   virtual void getVersionString(CString& version, int special=0);

   /// returns filter string
   virtual CString getFilterString();

   /// initializes the input module
   virtual int initInput(LPCTSTR infilename, SettingsManager &mgr,
      TrackInfo &trackinfo, SampleContainer &samples);

   /// returns info about the input file
   virtual void getInfo(int &channels, int &bitrate, int &length, int &samplerate);

   /// decodes samples and stores them in the sample container
   virtual int decodeSamples(SampleContainer &samples);

   /// returns the number of percent done
   virtual float percentDone()
   {
		return totalsamples != 0 ? float(samplecount)*100.f/float(totalsamples) : 0.f;
		//return 0;//sfinfo.frames != 0 ? float(samplecount)*100.f/float(sfinfo.frames) : 0.f;
   }

   /// called when done with decoding
   virtual void doneInput();

   /// gets the id3 tag in ape files
   virtual bool getId3Tag(LPCTSTR filepath, TrackInfo &trackinfo);

   /// destructor
   virtual ~MonkeysAudioInputModule();

private:
	//! pointers to functions
	static monkey::MAC_DLL dll;

	//! handle to mac file
	monkey::APE_DECOMPRESS_HANDLE	macfile;
	
	//! counts the samples already decoded
	int samplecount;

	//! number of samples in file
	int totalsamples;
	
	//! last error occured
	CString lasterror;
};


#endif	//__wlmonkeysaudioinputmodule_h_
