/*
 * Note by Michael Fink:
 * This is a very hacked-on libfreedb to get it to work under win32
 * use with caution!
 */
/*
	\todo Application layer errors (6xx), e.g. "need connection first"
	\todo Central error handling for general errors
*/

#define DISABLE_CDROMCODE

#ifndef FREEDB_HXX
#define FREEDB_HXX

#pragma warning(disable: 4786)

#include <winsock2.h>
#include <ws2tcpip.h>


//#  include "config.h"
#include <string>


/* TODO: NLS */
#ifdef ENABLE_NLS
#  include <locale.h>
#  include <libintl.h>
#  define _(String) gettext(String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop(String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(Domain)
#  define bindtextdomain(Package, Directory)
#  define gettext(String) (String)
#  define _(String) (String)
#  define N_(String) (String)
#endif


#ifdef HAVE_ASSERT_H
#  include <assert.h>
#else
#  ifndef assert
#    define assert(x)
#  endif
#endif

#include <sys/types.h>

#ifdef HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
#endif

#ifdef HAVE_NETDB_H
#  include <netdb.h>
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h> //close(), unlink()
#endif

#ifdef HAVE_ERRNO_H
#  include <errno.h>
#else
#  ifndef errno
     extern int errno;
#  endif
#endif

#ifdef __linux__
	#include <linux/cdrom.h>
#else
#  ifdef __FreeBSD__
#    include <sys/cdio.h>
#   else
#     ifdef WIN32
//#       error Win32-support is not yet finished.
#     else
#       error No OS or unsupported OS specified, run configure first or define __xxx__ manually.
#     endif /* __WIN32__ */
#   endif /* __FreeBSD__ */
#endif /* __linux__ */

#ifndef CD_FRAMES
#  define CD_FRAMES 75
#endif

#ifndef CD_SECS
#  define CD_SECS 60
#endif

#ifdef HAVE_SIGNAL_H
#  include <signal.h>
#endif

#include <string.h> //strerror() /* TODO: provide fallback! */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

using namespace std;

/*!	\namespace Freedb
	\brief Main namespace of libfreedb
*/
namespace Freedb
{

/*!	\namespace Helpers
	\brief Helper functions
*/
namespace Helpers
{
	using namespace std;

	vector<string> splitLines(const string& data);
	string stringReplace(const string& before, const string& after, const string& replace);
	int stoi(const string s);
	string longtohex(long l);
	string itos(int i);
	string ltos(long l);
	string convBase(unsigned long v, long base);
} /* namespace Helpers */


const unsigned short CDDBP_PORT = 8880;


/*class cVALID_CATEGORIES : public std::vector
{
	VALID_CATEGORIES()
	{
		// do push_back()s here
	}

	~VALID_CATEGORIES();
};*/

/*
	"blues",
	"classical",
	"country",
	"data",
	"folk",
	"jazz",
	"newage",
	"reggae",
	"rock",
	"soundtrack",
	"misc"
*/

//cVALID_CATEGORIES VALID_CATEGORIES();

typedef struct
{
	string leading;
	string version_number;
	string release_type;
	string level;
} ClientVersion;

typedef struct
{
	string name;
	ClientVersion version;
	string comments;
} ClientInfo;

typedef struct
{
	int min;
	int sec;
	int frame;
} TrackPosition;

/*!	\brief The central structure holding all disc information.
*/
typedef struct
{
	unsigned long discid;			/*!< The DiscId, an unique identifier for a compact disc. */
	string category;
	string dartist;					/*!< The name of the artist of the disc */
	string dtitle;					/*!< The title of the disc */
	string dyear;					/*!< The release year of the disc */
	string dgenre;					/*!< The genre (also called category) of the disc */
	string dextinfo;				/*!< Extended information about the disc; a string of arbitrary length */
	unsigned int dlength;			/*!< The length of the disc in seconds */
	ClientInfo submitter;			/*!< The program that submitted the data */
	int revision;					/*!< The revision of the disc entry */
	vector<TrackPosition> trackoffsets;		/*!< The track offsets of the disc; also used to calculate the DiscId */
	vector<string> tracktitles;		/*!< An STL vector containing the track titles in correct order */
	vector<string> trackextinfos;	/*!< An STL vector containing the track extra information in correct order */
	vector<int> playorder;			/*!< The playorder for this disc, can be used in local databases for CD-Players */
} CDInfo;

/*!	\brief A structure representing a search result - a small version of CDInfo.
*/
typedef struct
{
	unsigned long discid;
	string category;
	string dartist;
	string dtitle;
} SearchResult;

/*!	\brief Structure defining a location on the globe
*/
typedef struct
{
	string compassdirection;
	short degrees;
	short minutes;
} GeoCoords;

/*!	\brief Structure holding server information
*/
typedef struct
{
	string site;
	string protocol;
	unsigned short port;
	string address;
	GeoCoords latitude;
	GeoCoords longitude;
	string description;
} ServerInfo;


/* DB file format stuff */
string createEntry(const CDInfo& info, unsigned short proto = 1);
CDInfo parseEntry(const string& entry);
string separateString(const string& prepend, const string& s);
string convSpecialChars(const string& s);

/* DiscID stuff */
vector<TrackPosition> getTrackOffsets(const string & device = "/dev/cdrom");
unsigned int calcDiscLength(const TrackPosition& leadout);
unsigned long generateDiscID(const vector<TrackPosition>& offsets, const int& num_tracks);
unsigned long getDiscID(const string& device = "/dev/cdrom");
int generateDiscSum(int n);

void createDefaultDirectories(); /* FIXME: outstanding */


/*!	\class Error freedb.hxx freedb/freedb.hxx
	\brief Error class used throughout libfreedb
*/
class Error
{
	public:
		Error(short code, const string& msg = "No error msg available", const string& extinfo = "No extended information is available");
		~Error();

		short code;
		string msg, extinfo;
};

/*! 	\class Freedb freedb.hxx freedb/freedb.hxx
	\brief The base class for all freedb operations.
*/
class Freedb
{
	public:
		virtual vector<SearchResult> query(unsigned long discid, vector<TrackPosition>& tracks, unsigned int disclen) const = 0;
		virtual CDInfo read(const string& category, unsigned long discid) const = 0;
		virtual void write(const CDInfo &info) const = 0;
};

/*! 	\class Remote freedb.hxx freedb/freedb.hxx
	\brief Class for remote access to a freedb-server via CDDBP

	Note that the constructor tries to connect to the specified server/the default server,
	as it is vital for an object of this class to have a connection.
*/
class Remote : public Freedb
{
	public:
		Remote(const string& servername = "freedb.freedb.org");
		virtual ~Remote();

		void doHandshake(const string& username, const string& clientname, const string& version) const;
		void setProtoLevel(const unsigned short level);

		vector<ServerInfo> getServerList() const;
		vector<string> getCategories() const;

		vector<SearchResult> query(unsigned long discid, vector<TrackPosition>& tracks, unsigned int disclen) const;
      vector<SearchResult> query_cddb_raw(string raw_query) const;

		CDInfo read(const string& category, unsigned long discid) const;
		void write(const CDInfo& info) const;

		inline string getGreeting() const {	return(_greeting); }

	protected:
		void connect(const string& servername = "freedb.freedb.org");
		void disconnect();

		void sendCommand(const string& command) const;
		void sendDataBlock(const string& data) const;
		string getResponseLine(const bool& stripcr = true) const;
		string getDataBlock() const;
		ServerInfo parseServerInfoLine(const string& line) const;

		string _greeting;

	private:
		int _sockfd;
		unsigned short _proto;
};

/*! 	\class Local freedb.hxx freedb/freedb.hxx
	\brief Class for access to a local freedb-database

	Note that the constructor tries to open the specified file/the default file,
	as it is vital for an object of this class to have an open file.
*/
class Local : public Freedb
{
	public:
		Local(const string& dbdir = "~/.freedb/");
		virtual ~Local();

		vector<SearchResult> query(unsigned long discid, vector<TrackPosition>& tracks, unsigned int disclen) const;
		CDInfo read(const string& category, unsigned long discid) const;
		void write(const CDInfo &entry) const;
		void remove(const string& category, unsigned long discid) const;

	protected:
		string makeFilename(const string& category, unsigned long discid) const;

	private:
		string _dbdir;
		int _dbfd;
};

} /* namespace Freedb */

#endif
