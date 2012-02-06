#include "freedb.hpp"

namespace Freedb
{

namespace Helpers
{

/*! 	\brief Split a string at its line boundaries

	\param data the string to be split.
	\return A vector<string> containing the lines.

	FIXME: make a more general function - splitString(const string& splitter, const string& data)
*/
vector<string> splitLines(const string& data)
{
	vector<string> lines;
	unsigned int charpos = 0;
	unsigned int lastpos = 0;

	/*
		FIXME: we got a strange bug here: all lines are correct except the first one which yields in "# xmc" instead of "# xmcd"
		Unfortunately I have not been able to figure out the cause yet.
	*/
	while (charpos < data.length())
	{
		if (charpos >= data.length())
			break;

		if (data[charpos] == '\n')
		{
			lines.push_back(data.substr((lastpos == 0 ? lastpos : lastpos + 1), (charpos - lastpos - 1)));
			lastpos = charpos;
		}

		charpos++;
	}

	return(lines);
}

/*! 	\brief Replaces all occurences of a string within another string with a third string.

	\param s1 the string to find.
	\param s2 the string to insert for s1.
	\param replace the string on which the operation should be performed.
*/
string stringReplace(const string& before, const string& after, const string& replace)
{
	string temp = replace;
	unsigned int newpos = 0, pos = 0;

	while (pos < temp.length())
	{
		newpos = temp.find(before, pos);

		if (newpos == string::npos)
		{
			break;
		}

		//cout << "\tfound at " << newpos << endl ;

		temp.replace(newpos, after.length() + 1, after);

		pos = newpos + after.length();
	}

	return(temp);
}

/*! 	\brief Converts an STL string to an integer.

	\param s the string to be converted to an integer.
*/
int stoi(const string s)
{
	istringstream in(s);
	int i;

	in >> i;

	return(i);
}

/*!	\brief Converts an long to an STL string containing the number in hexadecimal notation

	\param l The long to be converted to a string.
*/
string longtohex(long l)
{
	ostringstream out;

	out << hex << l;

	return(out.str());
}

/*!	\brief Converts an integer to an STL string.

	\param i The integer to be converted to a string.
*/
string itos(int i)
{
	ostringstream out;

	out << i;

	return(out.str());
}

/*!	\brief Converts a long to an STL string.

	\param l The long to be converted to a string.
*/
string ltos(long l)
{
	ostringstream out;

	out << l;

	return(out.str());
}

/*!	\brief Converts a long to an STL string.

	\param v The number to be converted.
	\param base The target base.
*/
string convBase(unsigned long v, long base)
{
	string digits = "0123456789abcdef";
	string result;

	if((base < 2) || (base > 16))
	{
		result = "Error: base out of range.";
	}
	else
	{
		do
		{
			result = digits[v % base] + result;
			v /= base;
		} while(v);
	}

	return(result);
}

} /* namespace Helpers */

} /* namespace Freedb */