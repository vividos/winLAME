#include "freedb.hpp"

#ifndef DISABLE_CDROMCODE

//#include <sys/ioctl.h>
#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
	#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
	#include <fcntl.h>
#endif

namespace Freedb
{

using namespace Helpers;

/*!	\brief Reads the CD-TOC and saves track positions in a vector.

	See sourcecode in cdrom.cxx for info on how to calculate the absolute frame
	position of a track.

	\param device The device pathname to open; /dev/cdrom per default

	\todo Only the Linux unified CD-ROM driver interface and the FreeBSD cdio-interface
		are implemented. Access methods for more operating systems must follow; the decision
		on which code to compile must be made via preprocessor-macros.
*/
vector<TrackPosition> getTrackOffsets(const string& device)
{
	/* open device */
	int fd = open(device.c_str(), O_RDONLY | O_NONBLOCK);
	if (fd == (-1))
	{
		throw(Error(errno, _("Error while opening CD-ROM device ") + device));
	}

	/* read TOC to get number of tracks */
	#ifdef __linux__
	struct cdrom_tochdr header;
	int retval = ioctl(fd, CDROMREADTOCHDR, &header);
	#else
	#ifdef __FreeBSD__
	struct ioc_toc_header header;
	int retval = ioctl(fd, CDIOREADTOCHEADER, &header);
	#endif
	#endif

	if (fd == (-1))
	{
		throw(Error(errno, _("Error while reading TOC header on ") + device));
	}

	#ifdef __linux__
	int num_tracks = header.cdth_trk1;
	#else
	#ifdef __FreeBSD__
	int num_tracks = header.ending_track - header.starting_track;
	#endif
	#endif

	/* loop through tracks and save offsets */
	TrackPosition offset;
	vector<TrackPosition> offsets;

	#ifdef __linux__
	struct cdrom_tocentry te;

	for (int i = 0; i <= num_tracks; i++)
	{

		te.cdte_track = (i == num_tracks ? CDROM_LEADOUT : i + 1);
		te.cdte_format = CDROM_MSF;

		retval = ioctl(fd, CDROMREADTOCENTRY, &te);
		if (retval == (-1))
		{
			throw(Error(errno, _("Error while reading TOC entry of track ") + itos(i) + _(" on ") + device));
		}

		offset.min = static_cast<int>(te.cdte_addr.msf.minute);
		offset.sec = static_cast<int>(te.cdte_addr.msf.second);
		offset.frame = static_cast<int>(te.cdte_addr.msf.frame);

		offsets.push_back(offset);
		/*
		 *	To calculate the absolute frame position:
		 *
		 *	pos_abs = offset.frame + offset.second * CD_FRAMES + offset.minute * CD_SECS * CD_FRAMES;
		 *
		 *	Where CD_SECS is defined 60 and CD_FRAMES is defined 75 in linux/cdrom.h.
		 */
	}
	#else
	#ifdef __FreeBSD__
	struct cd_toc_entry toc_buffer[100];
	struct ioc_read_toc_entry t;

	t.address_format = CD_MSF_FORMAT;
	t.starting_track = 0;
	t.data_len = num_tracks * sizeof(struct cd_toc_entry);
	t.data = toc_buffer;

	retval = ioctl(fd, CDIOREADTOCENTRY, &t);
	if (retval == (-1))
	{
		throw(Error(errno, "Error while reading TOC entries on " + device));
	} else {
		for (int i=0; i<num_tracks; i++)
		{
			offset.min = toc_buffer[i].addr.msf.minute;
			offset.sec = toc_buffer[i].addr.msf.second;
			offset.frame = toc_buffer[i].addr.msf.frame;
			offsets.push_back(offset);
		}
	}
	#endif /* __FreeBSD__ */
	#endif

	/* done */
	return(offsets);
}

int generateDiscSum(int n)
{
	int sum = 0;

	while (n > 0)
	{
		sum += (n % 10);
		n = n / 10;
	}

	return(sum);
}

unsigned long generateDiscID(const vector<TrackPosition>& offsets, const int& num_tracks)
{
	vector<TrackPosition> c_offsets(offsets);
	int c_num_tracks = num_tracks - 1;

	c_offsets.pop_back();

	int i = 0, t = 0, n = 0;

	while (i < c_num_tracks)
	{
		n += generateDiscSum((c_offsets[i].min * 60) + c_offsets[i].sec);
		i++;
	}

	t = ((c_offsets[c_num_tracks].min * 60) + c_offsets[c_num_tracks].sec) -
		((c_offsets[0].min * 60) + c_offsets[0].sec);

	return((n % 0xff) << 24 | t << 8 | c_num_tracks);
}


/*! 	\brief A high-level interface for getting the DiscID

	This shows what you have to do in your own code if you want full control over
	the process of getting the DiscID otherwise you are free to use this.

	\param device See getTrackOffsets().
*/
unsigned long getDiscID(const string& device)
{
	vector<TrackPosition> offsets;

	offsets = getTrackOffsets();

	int num_tracks = 0;

	vector<TrackPosition>::const_iterator i = offsets.begin();

	while (i != offsets.end())
	{
		num_tracks++;
		i++;
	}

	return(generateDiscID(offsets, num_tracks));
}

} /* namespace Freedb */


/* //test code
int main()
{
	cout << "freedb-discid: " << hex << getDiscID("/dev/cdrom") << endl ;

	return(0);
}
*/

#endif /* ! DISABLE_CDROMCODE */