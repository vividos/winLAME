#include "freedb.hpp"

namespace Freedb
{

using namespace Helpers;

/*! 	\brief The class constructor.

	You have to construct a FreedbRemote object to utilize a remote freedb server.
	All low-level C chores like socket initializiation, dns querying
	are done by this function as it calls Remote::connect().

	Clean C++ exception throwing is	employed to guarantee easy
	error handling. It is imperative that you enclose a call to this
	function in a try-block with a catch(const Error& err) following
	it. The reserved error codes 600-699 may be returned by this function.

	\param servername The server to connect to, may be a hostname or
		an IP address. If no server is given, the main relay server is used.

	\exception Error Throws the Error struct if a part of the connection
		process fails.

	\todo Set default proto level to 5 as soon as all protocol levels are supported.
*/
Remote::Remote(const string& servername)
	:	_sockfd(-1),
		_proto(1)
{
#ifdef HAVE_SIGNAL
	signal(SIGPIPE, SIG_IGN);
#endif

	connect(servername);

	return;
}

/*! 	\brief The class destructor.

	The destructor will attempt to disconnect.
*/
Remote::~Remote()
{
	disconnect();

	return;
}

/*!	\brief Helper function to send a CDDBP command to the server.

	\note You cannot (and should not) use this function directly.
	If you really need to send a custom command to the server,
	derive your own class from class CDDB.

	\param command The command to send to the server without terminating newline.

	\todo implement error checking.
*/
void Remote::sendCommand(const string& command) const
{
	int retval = send(_sockfd, (command + "\n").data(), (command + "\n").length(), 0);

	if (retval < 0)
	{
		throw(Error(errno, strerror(errno)));
	}

	return;
}

/*!	\brief Helper function to send a CDDBP data block to the server.

	\note You cannot (and should not) use this function directly.
	If you really need to send a custom command to the server,
	derive your own class from class CDDB.

	\param command The data to send to the server without terminating "\n.".

	\todo implement error checking.
*/
void Remote::sendDataBlock(const string& data) const
{
	cout << "sending this block: " << endl ;
	cout << (data + ".\n").data() << endl ;
	int retval = send(_sockfd, (data + ".\n").data(), (data + ".\n").length(), 0);

	if (retval < 0)
	{
		throw(Error(errno, strerror(errno)));
	}

	return;
}

string Remote::getDataBlock() const
{
	int retval;
	char buf;
	bool newline;
	string response;

	while (1)
	{
		retval = recv(_sockfd, &buf, 1, 0);

		if (buf == '.' && newline == true)
		{
			break;
		}

		if (buf == '\n')
		{
			newline = true;
		}
		else
		{
			newline = false;
		}

		if (buf != '\r')
		{
			response += buf;
		}
	}

	return(response);
}

/*!	\brief Helper function to get a line of response from the server.

	A line of response is a string of arbitrary data that is terminated
	by a newline, often consisting of a response code (e.g. 200), a space
	and additional information.

	You cannot (and should not) use this function directly.
	If you really need to send a custom command to the server,
	derive your own class from class CDDB.

	\param stripcr Denotes whether carriage returns (\r) should be filtered
		out.
	\return A string containing the response line.
*/
string Remote::getResponseLine(const bool& stripcr) const
{
	string response;
	char buf = 0;
	int retval;

	while (buf != '\n')
	{
		retval = recv(_sockfd, &buf, 1, 0);

		if (buf != '\r' || stripcr == false)
		{
			response += buf;
		}
	}

	return(response);
}

/*!	\brief Attempts to connect to a freedb server.

	Utilized by the constructor.
*/
void Remote::connect(const string& servername)
{
   hostent* host = gethostbyname(servername.c_str());
   if (host == NULL)
      throw(Error(610, _("Unable to get address of ") + servername, strerror(errno)));

   sockaddr_in addr;
   addr.sin_addr.s_addr = *(u_long*) host->h_addr_list[0];
   addr.sin_port = htons(CDDBP_PORT);
   addr.sin_family = AF_INET;
/*
	struct addrinfo hints, *res;

	int pf = PF_UNSPEC;

	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = pf;
	hints.ai_socktype = SOCK_STREAM;

	string port = itos(CDDBP_PORT);

	if (getaddrinfo(servername.c_str(), port.c_str(), &hints, &res) != 0)
	{
		throw(Error(610, _("Unable to get address of ") + servername, strerror(errno)));
	}
*/
	//fprintf(stderr, "got: len(addr) = %d, family = %d\n", res->ai_addrlen, res->ai_family==PF_INET6?"PF_INET" ? (res->ai_family==PF_INET?"PF_INET":"unknown"));

//	_sockfd = socket(res->ai_family, SOCK_STREAM, 0);
   _sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (_sockfd == (-1))
	{
		throw(Error(611, _("Unable to create socket"), strerror(errno)));
	}

//	int retval = ::connect(_sockfd, res->ai_addr, res->ai_addrlen);
   int retval = ::connect(_sockfd, (struct sockaddr*)&addr, sizeof(addr));

//	freeaddrinfo(res);

	if (retval == (-1))
	{
		throw(Error(612, _("Unable to connect"), strerror(errno)));
	}

	/* socket is connected */
	string response = getResponseLine();

	if (response.substr(0, 2) != "20")
	{
		string errmsg;

		switch (stoi(response.substr(0, 3)))
		{
			case 432:
				errmsg = _("Permission denied");
			break;

			case 433:
				errmsg = _("Too many users");
			break;

			case 434:
				errmsg = _("System load too high");
			break;
		}

		throw(Error(613, _("Server rejected connection"), response.substr(0, 3) + " - " + errmsg));
	}

	_greeting = response;
	return;
}

/*!	\brief Disconnects a connection to a remote server if one exists.
*/
void Remote::disconnect()
{
	sendCommand("quit");
	closesocket(_sockfd);

	return;
}

/*!	\brief Sends the handshake to the server.

	A valid CDDBP handshake is created from the given parameters
	and sent to the server. The handshake must be done after connecting
	before other commands are issued. If a handshake was already performed,
	error code 613 "Already shook hands" is set and an exception thrown.

	\param username The name of the user; application developers should offer
		the user an opportunity to set this.
	\param clientname The name of the client software.
	\param version The version of the client software.
*/
void Remote::doHandshake(const string& username, const string& clientname, const string& version) const
{
	char* hn = new char[256];
	int retval = gethostname(hn, 256);
	if (retval == (-1))
	{
		throw(Error(614, _("Unable to look up local hostname"), itos(errno) + " - " + strerror(errno)));
	}

	/* send handshake */
	string hostname = hn;
	string handshake = "cddb hello " + username + " " + hostname + " " + clientname + " " + version;

	sendCommand(handshake);

	/* get response */
	string response = getResponseLine();

	if (response.substr(0, 2) != "20")
	{
		string errmsg;

		switch (stoi(response.substr(0, 3)))
		{
			case 431:
				errmsg = _("Handshake not successful");
			break;

			case 402:
				errmsg = _("Already shook hands");
			break;

         default:
            errmsg = response;
         break;
		}

		throw(Error(stoi(response.substr(0, 3)), errmsg));
	}
}

/*!	\brief Parses a line returned by the 'sites'-command.

	This function is used internally by getServerList() and shouldn't be used.
*/
ServerInfo Remote::parseServerInfoLine(const string& line) const
{
	ServerInfo info;
	int lastpos = 0, currentpos;

	string c_line = line;

	if (c_line[0] == '\n')
	{
		c_line = c_line.substr(1, c_line.length() - 1);
	}

	/* hostname */
	currentpos = c_line.find(" ", lastpos);
	info.site = c_line.substr(lastpos, currentpos);
	lastpos = currentpos + 1;

	/* protocol */
	if (_proto >= 3)
	{
		currentpos = c_line.find(" ", lastpos);
		info.protocol = stoi(c_line.substr(lastpos, currentpos));
		lastpos = currentpos + 1 ;
	}

	/* port */
	currentpos = c_line.find(" ", lastpos);
	info.port = stoi(c_line.substr(lastpos, currentpos));
	lastpos = currentpos + 1 ;

	/* address */
	if (_proto >= 3)
	{
		currentpos = c_line.find(" ", lastpos);
		info.address = stoi(c_line.substr(lastpos, currentpos));
		lastpos = currentpos + 1 ;
	}

	/* latitude */
	currentpos = c_line.find(" ", lastpos);
	string latitude = c_line.substr(lastpos, currentpos);
	lastpos = currentpos + 1 ;
	info.latitude.compassdirection = latitude.substr(0, 1);
	info.latitude.degrees = stoi(latitude.substr(1, 3));
	info.latitude.minutes = stoi(latitude.substr(5, 2));

	/* longitude */
	currentpos = c_line.find(" ", lastpos);
	string longitude = c_line.substr(lastpos, currentpos);
	lastpos = currentpos + 1 ;
	info.longitude.compassdirection = longitude.substr(0, 1);
	info.longitude.degrees = stoi(longitude.substr(1, 3));
	info.longitude.minutes = stoi(longitude.substr(5, 2));

	/* description */
	info.description = c_line.substr(lastpos, c_line.length() - lastpos);

	return(info);
}

/*!	\brief Gets the list of master freedb servers
*/
vector<ServerInfo> Remote::getServerList() const
{
	vector<ServerInfo> servers;

	sendCommand("sites");

	string response = getResponseLine();

	if (response.substr(0, 1) != "2")
	{
		string errmsg;

		switch (stoi(response.substr(0, 3)))
		{
			case 401:
				errmsg = _("No site information available");
			break;
		}

		throw(stoi(response.substr(0, 3)), errmsg);
	}

	vector<string> lines = splitLines(getDataBlock());

	for (vector<string>::const_iterator line = lines.begin(); line != lines.end(); line++)
	{
		servers.push_back(parseServerInfoLine(*line));
	}

	return(servers);
}

/*!	\brief Gets the list of valid categories from the server.
*/
vector<string> Remote::getCategories() const
{
	sendCommand("lscat");

	string response = getResponseLine();

	if (response.substr(0, 3) != "210")
	{
		throw(Error(stoi(response.substr(0, 3))));
	}

	return(splitLines(getDataBlock()));
}

/*!	\brief Request a single database entry by category and DiscID from the server.

	\note A query with Freedb::query() \em{must} be performed before this function is called.

	\param category The category (as returned by Freedb::search()).
	\param discid The DiscID (as returned by Freedb::search()).
*/
CDInfo Remote::read(const string& category, unsigned long discid) const
{
	sendCommand("cddb read " + category + " " + longtohex(discid));

	string response = getResponseLine();
   if (response == "\n")
      response = getResponseLine();

	if (response.substr(0, 3) != "210")
	{
		string errmsg;

		switch (stoi(response.substr(0, 3)))
		{
			case 401:
				errmsg = _("Specified CDDB entry not found");
			break;

			case 402:
				errmsg = _("Server error");
			break;

			case 403:
				errmsg = _("Database entry is corrupt");
			break;

			case 409:
				errmsg = _("No handshake");
			break;
		}

		throw(Error(stoi(response.substr(0, 3)), errmsg));
	}

	return(parseEntry(getDataBlock()));
}

/*!	\brief Submits an entry to the server for inclusion in the freedb database.
*/
void Remote::write(const CDInfo& info) const
{
	/* TODO: check if category is valid! */
	sendCommand("cddb write " + info.category + " " + longtohex(info.discid));

	string response = getResponseLine();

	if (response.substr(0, 3) != "320")
	{
		string errmsg;

		switch (stoi(response.substr(0, 3)))
		{
			case 401:
				errmsg = _("Permission denied");
			break;

			case 402:
				errmsg = _("File access failed");
			break;

			case 501:
				errmsg = _("Write rejected, extended information available");
			break;

			case 409:
				errmsg = _("No handshake");
			break;
		}

		throw(Error(stoi(response.substr(0, 3)), errmsg, response));
	}

	string block = createEntry(info);

	sendDataBlock(block);

	response = getResponseLine();

	if (response.substr(0, 3) != "200")
	{
		string errmsg;

		switch (stoi(response.substr(0, 3)))
		{
			case 501:
				errmsg = _("Entry rejected, extended information available");
			break;
		}

		throw(Error(stoi(response.substr(0, 3)), errmsg, response));
	}

	return;
}

/*!	\brief Queries the server for an entry by discid, track offsets and disc length.

	The returned vector contains the matches returned by the server and is therefore empty
	if no matches were returned.

	\param discid The DiscID (as returned by Freedb::getDiscID()).
	\param tracks The track offsets including the leadout track (as returned by Freedb::getTrackOffsets()).
	\param disclen The length of the disc in seconds.
*/
vector<SearchResult> Remote::query(unsigned long discid, vector<TrackPosition>& tracks, unsigned int disclen) const
{
	string offsets = "";
	int num_tracks = 0;

	cout << "Remote::search: performing search." << endl ;
	cout << "\tdiscid: " << hex << discid << endl ;

	tracks.pop_back();

	for (vector<TrackPosition>::const_iterator track = tracks.begin(); track != tracks.end(); track++)
	{
		offsets += itos(track->min * CD_SECS * CD_FRAMES + track->sec * CD_FRAMES + track->frame) + " ";
		num_tracks++;
	}
	cout << "\tnumber of tracks: " << itos(num_tracks) << endl ;

	cout << "\ntrack offsets: " << endl ;
	cout << offsets << endl ;

	cout << "sending command: " << "cddb query " << longtohex(discid) << " " << itos(tracks.size()) << " " << offsets << itos(disclen) << endl ;

   return query_cddb_raw(longtohex(discid) + " " + itos(tracks.size()) + " " + offsets + itos(disclen));
}

vector<SearchResult> Remote::query_cddb_raw(string raw_query) const
{
	sendCommand("cddb query " + raw_query);

	cout << "getting response...";
	string response = getResponseLine();
	cout << " -> " << response << endl ;

	if (response.substr(0, 1) != "2")
	{
		string errmsg;

		switch (stoi(response.substr(0, 3)))
		{
			case 403:
				errmsg = _("Database entry corrupt");
			break;

			case 409:
				errmsg = _("No handshake");
			break;
		}

		throw(Error(stoi(response.substr(0, 3)), errmsg));
	}

	SearchResult result;
	vector<SearchResult> results;
	vector<string> lines;
	string temptitle;
	int pos_discid, pos_temptitle;

	if (response.substr(0, 3) == "202") /* no match */
	{
		cout << "no match" << endl ;
		return(results);
	}

	if (response.substr(0, 3) == "200") /* exact match */
	{
		cout << "we got an exact match: " << response.substr(3, response.length() - 3) << endl ;
		lines.push_back(response.substr(4, response.length() - 3));
	}
	else
	{
		cout << "we got more than one match." << endl ;
		lines = splitLines(getDataBlock());
	}

	for (vector<string>::const_iterator line = lines.begin(); line != lines.end(); line++)
	{
		cout << "iterating lines." << endl ;

		pos_discid = line->find(" ", 0) + 1;
		result.category = line->substr(0, pos_discid - 1);

		pos_temptitle = line->find(" ", pos_discid) + 1;
		result.discid = strtoul((line->substr(pos_discid, pos_temptitle - pos_discid)).c_str(), NULL, 16);

		temptitle = line->substr(pos_temptitle, line->length() - pos_temptitle);

		result.dartist = temptitle.substr(0, temptitle.find(" / "));
		result.dtitle = temptitle.substr(temptitle.find(" / ") + 3, temptitle.length() - (temptitle.find(" / ") + 3));

		results.push_back(result);
	}

	return(results);
}

} /* namespace Freedb */
