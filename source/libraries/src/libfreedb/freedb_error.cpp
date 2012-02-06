#include "freedb.hpp"

namespace Freedb
{

using namespace Helpers;

Error::Error(short code, const string& msg, const string& extinfo)
{
	this->code = code;
	this->msg = msg;
	this->extinfo = extinfo;

	return;
}

Error::~Error()
{
	return;
}

} /* namespace Freedb */