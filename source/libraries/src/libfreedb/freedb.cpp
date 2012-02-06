#include "freedb.hpp"

/* TODO: rename createEntry() and parseEntry() functions to more proper names, e.g. makeRaw() and parseRaw() */

namespace Freedb
{

using namespace Helpers;

string separateString(const string& prepend, const string& s)
{
		unsigned int pos = 0, charcount = 0;
		string temp = prepend + s;

		/* replace special chars */
		for (pos = 0; pos < temp.length(); pos++)
		{
			switch (temp[pos])
			{
				case '\n':
					temp.replace(pos, 1, "\\n");
					pos++;
				break;

				case '\t':
					temp.replace(pos, 1, "\\t");
					pos++;
				break;

				case '\\':
					temp.replace(pos, 1, "\\\\");
					pos++;
				break;

				default:
				break;
			}
		}

		pos = 0;

		/* separate at 200-char boundary (cddb-standard says 256 but we leave ample space for the prepend-string "VAR=" */
		while (pos < temp.length())
		{
			if (charcount >= 200)
			{
				temp.insert(pos, "\n" + prepend);
				charcount = 0;
			}
			charcount++;
			pos++;
		}

		return(temp + "\n");
}

/*
	TODO:
	- pad discid with zeros to 8 chars (if applicable, of course)
	- sanity checks (e.g. whether fields contain data at all)
	- ! check for 256-char boundary in lines that do not support multiline content !
*/
string createEntry(const CDInfo& info, unsigned short proto)
{
	CDInfo cinfo = info;

	cinfo.trackoffsets.pop_back();

	string entry;

	entry  = "# xmcd\n";
	entry += "# \n";
	entry += "# Track frame offsets:\n";

	vector<TrackPosition>::const_iterator i = cinfo.trackoffsets.begin();

	for (i = cinfo.trackoffsets.begin(); i != cinfo.trackoffsets.end(); i++)
	{
			entry += "# " + itos(i->frame + i->sec * CD_FRAMES + i->min * CD_SECS * CD_FRAMES) + "\n";
	}

	/* convenience code */
	if (cinfo.trackextinfos.empty())
	{
		for (unsigned int i = 0; i < cinfo.tracktitles.size(); i++)
		{
			cinfo.trackextinfos.push_back("");
		}
	}
	/* TODO: fill-in all other "optional-yet-mandatory" fields */

	entry += "#\n";
	entry += "# Disc length: " + itos(cinfo.dlength) + " seconds\n";
	entry += "#\n";
	entry += "# Revision: " + itos(cinfo.revision) + "\n";

	/* FIXME: check if ClientVersion (cinfo.submitter.version) format is correct */
	entry += "# Submitted via: " + cinfo.submitter.name + " "
		+ cinfo.submitter.version.leading
		+ cinfo.submitter.version.version_number
		+ cinfo.submitter.version.release_type
		+ cinfo.submitter.version.level + " "
		+ cinfo.submitter.comments + "\n";

	entry += "#\n";

	entry += "DISCID=" + convBase(cinfo.discid, 16) + "\n";
	entry += "DTITLE=" + cinfo.dartist + " / " + cinfo.dtitle + "\n";
	if (proto >= 5)
	{
		entry += "DYEAR=" + cinfo.dyear + "\n";
		entry += "DGENRE=" + cinfo.dgenre + "\n";
	}

	int count = 0;

   {
	for (vector<string>::const_iterator i = cinfo.tracktitles.begin(); i != cinfo.tracktitles.end(); i++)
	{
		entry += separateString("TTITLE" + itos(count) + "=", *i);
		count++;
	}
   }


	entry += separateString("EXTD=", cinfo.dextinfo);

	count = 0;

   {
	for (vector<string>::const_iterator i = cinfo.trackextinfos.begin(); i != cinfo.trackextinfos.end(); i++)
	{
		entry += separateString("EXTT" + itos(count) + "=", *i);
		count++;
	}
   }


	string playorder;
   {
	for (vector<int>::const_iterator i = cinfo.playorder.begin(); i != cinfo.playorder.end(); i++)
	{
		playorder += itos(*i) + " ";
	}
   }
	entry += separateString("PLAYORDER=", playorder);


	return(entry);
}

//TODO: trim excess whitespace!
CDInfo parseEntry(const string& entry)
{
	CDInfo info;
	vector<string> lines = splitLines(entry);

	bool reading_trackframes = false;

	map<std::string, std::string> data;

	/* traverse lines */
	for (vector<string>::const_iterator i = lines.begin(); i != lines.end(); i++)
	{
#ifdef _DEBUG
        OutputDebugStringA("FREEDB: ");
        OutputDebugStringA(i->c_str());
        OutputDebugStringA("\n");
#endif

		/* track frame offset list end marker */
		if (reading_trackframes == true && (*i == "#" || *i == "# "))
		{
			reading_trackframes = false;
			continue;
		}

		/* comment or static formal field */
		if (i->substr(0, 5) == "# xmc" || *i == "#" || *i == "# ")
		{
			continue;
		}

		/* disc length */
		if (i->substr(0, 14) == "# Disc length:")
		{
			info.dlength = stoi(i->substr(14, i->length() - 14));
			continue;
		}

		/* track frame offset list start marker */
		if (i->substr(0, 22) == "# Track frame offsets:")
		{
			reading_trackframes = true;
			continue;
		}

		/* a track frame offset */
		if (reading_trackframes == true)
		{
			int frames = stoi(i->substr(2, i->length()-2));

			TrackPosition pos;

			pos.min = frames / (CD_SECS * CD_FRAMES);
			frames %= CD_SECS * CD_FRAMES;

			pos.sec = frames / CD_FRAMES;
			frames %= CD_FRAMES;

			pos.frame = frames;

			info.trackoffsets.push_back(pos);

			continue;
		}

		/* revision of this entry */
		if (i->substr(0, 12) == "# Revision: ")
		{
			info.revision = stoi(i->substr(12, i->length() - 12));
			continue;
		}

		/* submitting program */
		if (i->substr(0, 17) == "# Submitted via: ")
		{
			/* FIXME: do a sub-parse here to get the correct ClientInfo-fields */
			//info.submitter = i->substr(17, i->length() - 17);
			continue;
		}

		/* now it has to be a VAR=VALUE style line, which we parse into a map<std::string, std::string> */
		/* field concatenation is done almost automagically via the += operator */
      std::string part1, part2;
      part1 = i->substr(0, i->find_first_of("="));
      part2 = i->substr(i->find_first_of("=") + 1, i->length() - i->find_first_of("="));

		data[part1] += part2;
   }

	info.discid = strtoul(data["DISCID"].c_str(), NULL, 16); /* TODO: replace NULL by **endptr so we can catch invalid DiscIDs */

   string dtitle = data["DTITLE"];
   if (!dtitle.empty())
   {
      info.dartist = dtitle.substr(0, dtitle.find(" / ", 0));
      info.dtitle = dtitle.substr(dtitle.find(" / ", 0) + 3, dtitle.length() - (dtitle.find(" / ", 0) + 3));
   }
	info.dyear = data["DYEAR"];
	info.dgenre = data["DGENRE"];

	/* FIXME: we might just pack the conversion stuff into an extra function for convenience and readability */
	info.dextinfo = data["EXTD"];
	info.dextinfo = stringReplace("\\n", "\n", info.dextinfo);
	info.dextinfo = stringReplace("\\t", "\t", info.dextinfo);
	info.dextinfo = stringReplace("\\\\", "\\", info.dextinfo);


   {
	for (unsigned int i = 0; i < info.trackoffsets.size(); i++)
	{
		string temptitle = data["TTITLE" + itos(i)];
		string tempext = data["EXTT" + itos(i)];

		temptitle = stringReplace("\\n", "\n", temptitle);
		temptitle = stringReplace("\\t", "\t", temptitle);
		temptitle = stringReplace("\\\\", "\\", temptitle);

		tempext = stringReplace("\\n", "\n", tempext);
		tempext = stringReplace("\\t", "\t", tempext);
		tempext = stringReplace("\\\\", "\\", tempext);

		info.tracktitles.push_back(temptitle);
		info.trackextinfos.push_back(tempext);
	}
   }

	return(info);
}

unsigned int calcDiscLength(const TrackPosition& leadout)
{
	return(static_cast<unsigned int>((leadout.frame + leadout.sec * CD_FRAMES + leadout.min * CD_SECS * CD_FRAMES) / CD_FRAMES));
}

} /* namespace Freedb */

using namespace Freedb;

/* test main() */
/*
int main()
{

	CDInfo info;

	info.discid = strtoul("b610b80c", NULL, 16);

	info.dartist = "Helloween";
	info.dtitle = "Chameleon";
	info.dyear = "1993";
	info.dgenre = "Rock";

	info.revision = 5;

	info.submitter.name = "libfreedb-test";
	info.submitter.version.leading = "alpha:";
	info.submitter.version.version_number = "0.0.8";
	info.submitter.version.release_type = "patchlevel";
	info.submitter.version.level = "8";

	info.trackoffsets = getTrackOffsets();

	info.dlength = calcDiscLength(info.trackoffsets.back());

	info.tracktitles.push_back("First Time");
	info.tracktitles.push_back("When The Sinner");
	info.tracktitles.push_back("I don't Wanna Cry No More");
	info.tracktitles.push_back("Crazy Cat");
	info.tracktitles.push_back("Giants");
	info.tracktitles.push_back("Windmill");
	info.tracktitles.push_back("Revolution Now");
	info.tracktitles.push_back("In The Night");
	info.tracktitles.push_back("Music");
	info.tracktitles.push_back("Step Out Of Hell");
	info.tracktitles.push_back("This is a line containing special characters like the newline,\nthe tab\tand the backslash\\character.");
	info.tracktitles.push_back("This is a quite long line which breaks the 200-char boundary and has to be split into several fields - This is a quite long line which breaks the 200-char boundary and has to be split into several fields - This is a quite long line which breaks the 200-char boundary and has to be split into several fields -");

	string entry = createEntry(info);
	//cout << entry << endl ;

	info = parseEntry(entry);

	entry = createEntry(info);
	cout << entry << endl ;

	return(0);
}*/
