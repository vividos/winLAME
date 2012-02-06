#include "freedb.hpp"
#include <fstream>
#include <cstdio>

/*
	- known limitation: the alternate format for FAT file system is not (yet? drop me an email) supported
	- FIXME: do error checking after reading/writing
	- TODO: revise error codes
*/

namespace Freedb
{

using namespace Helpers;


Local::Local(const string& dbdir)
	:	_dbdir(dbdir)
{
	return;
}


Local::~Local()
{
	return;
}


CDInfo Local::read(const string& category, unsigned long discid) const
{
	ifstream in;
	string rawdata;
	char linebuf[256];

	in.open((makeFilename(category, discid)).c_str(), ios::in);

	if (!in.is_open())
	{
		throw(Error(631, "Couldn't read from file \"" + makeFilename(category, discid) + "\"", itos(errno) + " - " + strerror(errno)));
	}

	while (!in.eof())
	{
		in.getline(linebuf, 256);
		rawdata += linebuf;
	}

	in.close();

	return(parseEntry(rawdata));
}


void Local::write(const CDInfo& entry) const
{
	ofstream out;
	string rawdata;

	out.open((makeFilename(entry.category, entry.discid)).c_str(), ios::out);

	if (!out.is_open())
	{
		throw(Error(632, "Couldn't write file \"" + makeFilename(entry.category, entry.discid) + "\"", itos(errno) + " - " + strerror(errno)));
	}

	rawdata = createEntry(entry);

	out.write(rawdata.c_str(), rawdata.length());

	out.close();

	return;
}


vector<SearchResult> Local::query(unsigned long discid, vector<TrackPosition>& tracks, unsigned int disclen) const
{
	vector<SearchResult> results;

	/* fuzzy hash matching here */

	return(results);
}


void Local::remove(const string& category, unsigned long discid) const
{
   if (unlink((makeFilename(category, discid)).c_str()) == -1)
	{
		throw(Error(633, "Couldn't delete file \"" + makeFilename(category, discid) + "\"", itos(errno) + " - " + strerror(errno)));
	}

	return;
}


string Local::makeFilename(const string& category, unsigned long discid) const
{
	return((_dbdir + category + "/" + longtohex(discid)).c_str());
}


} /* namespace Freedb */
