/*
	BASSCD 2.4 C/C++ header file
	Copyright (c) 2003-2009 Un4seen Developments Ltd.

	See the BASSCD.CHM file for more detailed documentation
*/

#ifndef BASSCD_H
#define BASSCD_H

#include "bass.h"

#if BASSVERSION!=0x204
#error conflicting BASS and BASSCD versions
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BASSCDDEF
#define BASSCDDEF(f) WINAPI f
#endif

// Additional error codes returned by BASS_ErrorGetCode
#define BASS_ERROR_NOCD		12	// no CD in drive
#define BASS_ERROR_CDTRACK	13	// invalid track number
#define BASS_ERROR_NOTAUDIO	17	// not an audio track

// Additional BASS_SetConfig options
#define BASS_CONFIG_CD_FREEOLD		0x10200
#define BASS_CONFIG_CD_RETRY		0x10201
#define BASS_CONFIG_CD_AUTOSPEED	0x10202
#define BASS_CONFIG_CD_SKIPERROR	0x10203

// BASS_CD_SetInterface options
#define BASS_CD_IF_AUTO	0
#define BASS_CD_IF_SPTI	1
#define BASS_CD_IF_ASPI	2
#define BASS_CD_IF_WIO	3

typedef struct {
	const char *vendor; // manufacturer
	const char *product; // model
	const char *rev; // revision
	int letter;		// drive letter
	DWORD rwflags;	// read/write capability flags
	BOOL canopen;	// BASS_CD_DOOR_OPEN/CLOSE is supported?
	BOOL canlock;	// BASS_CD_DOOR_LOCK/UNLOCK is supported?
	DWORD maxspeed;	// max read speed (KB/s)
	DWORD cache;	// cache size (KB)
	BOOL cdtext;	// can read CD-TEXT
} BASS_CD_INFO;

// "rwflag" read capability flags
#define BASS_CD_RWFLAG_READCDR		1
#define BASS_CD_RWFLAG_READCDRW		2
#define BASS_CD_RWFLAG_READCDRW2	4
#define BASS_CD_RWFLAG_READDVD		8
#define BASS_CD_RWFLAG_READDVDR		16
#define BASS_CD_RWFLAG_READDVDRAM	32
#define BASS_CD_RWFLAG_READANALOG	0x10000
#define BASS_CD_RWFLAG_READM2F1		0x100000
#define BASS_CD_RWFLAG_READM2F2		0x200000
#define BASS_CD_RWFLAG_READMULTI	0x400000
#define BASS_CD_RWFLAG_READCDDA		0x1000000
#define BASS_CD_RWFLAG_READCDDASIA	0x2000000
#define BASS_CD_RWFLAG_READSUBCHAN	0x4000000
#define BASS_CD_RWFLAG_READSUBCHANDI 0x8000000
#define BASS_CD_RWFLAG_READC2		0x10000000
#define BASS_CD_RWFLAG_READISRC		0x20000000
#define BASS_CD_RWFLAG_READUPC		0x40000000

// additional BASS_CD_StreamCreate/File flags
#define BASS_CD_SUBCHANNEL		0x200
#define BASS_CD_SUBCHANNEL_NOHW	0x400
#define BASS_CD_C2ERRORS		0x800

// additional CD sync types
#define BASS_SYNC_CD_ERROR		1000
#define BASS_SYNC_CD_SPEED		1002

// BASS_CD_Door actions
#define BASS_CD_DOOR_CLOSE		0
#define BASS_CD_DOOR_OPEN		1
#define BASS_CD_DOOR_LOCK		2
#define BASS_CD_DOOR_UNLOCK		3

// BASS_CD_GetID flags
#define BASS_CDID_UPC		1
#define BASS_CDID_CDDB		2
#define BASS_CDID_CDDB2		3
#define BASS_CDID_TEXT		4
#define BASS_CDID_CDPLAYER	5
#define BASS_CDID_MUSICBRAINZ 6
#define BASS_CDID_ISRC		0x100 // + track #

// BASS_CHANNELINFO type
#define BASS_CTYPE_STREAM_CD	0x10200

DWORD BASSCDDEF(BASS_CD_SetInterface)(DWORD iface);

BOOL BASSCDDEF(BASS_CD_GetInfo)(DWORD drive, BASS_CD_INFO *info);
BOOL BASSCDDEF(BASS_CD_Door)(DWORD drive, DWORD action);
BOOL BASSCDDEF(BASS_CD_DoorIsOpen)(DWORD drive);
BOOL BASSCDDEF(BASS_CD_DoorIsLocked)(DWORD drive);
BOOL BASSCDDEF(BASS_CD_IsReady)(DWORD drive);
DWORD BASSCDDEF(BASS_CD_GetTracks)(DWORD drive);
DWORD BASSCDDEF(BASS_CD_GetTrackLength)(DWORD drive, DWORD track);
DWORD BASSCDDEF(BASS_CD_GetTrackPregap)(DWORD drive, DWORD track);
const char *BASSCDDEF(BASS_CD_GetID)(DWORD drive, DWORD id);
DWORD BASSCDDEF(BASS_CD_GetSpeed)(DWORD drive);
BOOL BASSCDDEF(BASS_CD_SetSpeed)(DWORD drive, DWORD speed);
DWORD BASSCDDEF(BASS_CD_GetCache)(DWORD drive);
BOOL BASSCDDEF(BASS_CD_SetCache)(DWORD drive, BOOL enable);
BOOL BASSCDDEF(BASS_CD_Release)(DWORD drive);

HSTREAM BASSCDDEF(BASS_CD_StreamCreate)(DWORD drive, DWORD track, DWORD flags);
HSTREAM BASSCDDEF(BASS_CD_StreamCreateFile)(const char *file, DWORD flags);
DWORD BASSCDDEF(BASS_CD_StreamGetTrack)(HSTREAM handle);
BOOL BASSCDDEF(BASS_CD_StreamSetTrack)(HSTREAM handle, DWORD track);

BOOL BASSCDDEF(BASS_CD_Analog_Play)(DWORD drive, DWORD track, DWORD pos);
DWORD BASSCDDEF(BASS_CD_Analog_PlayFile)(const char *file, DWORD pos);
BOOL BASSCDDEF(BASS_CD_Analog_Stop)(DWORD drive);
DWORD BASSCDDEF(BASS_CD_Analog_IsActive)(DWORD drive);
DWORD BASSCDDEF(BASS_CD_Analog_GetPosition)(DWORD drive);

#ifdef __cplusplus
}
#endif

#endif
