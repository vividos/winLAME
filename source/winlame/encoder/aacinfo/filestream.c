/*
** FAAD - Freeware Advanced Audio Decoder
** Copyright (C) 2002 M. Bakker
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** $Id: filestream.c,v 1.7 2005/02/22 19:14:06 vividos Exp $
**/
/*! \file filestream.c

   \brief contains implementation of the aacinfo functionality

*/

/* Not very portable yet */

#include <winsock2.h> // Note: Must be *before* windows.h
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "filestream.h"
#include "aacinfo.h"

long m_local_buffer_size = 64;
long m_stream_buffer_size = 128;

FILE_STREAM *open_filestream(LPCTSTR filename)
{
    FILE_STREAM *fs;

    {
        fs = (FILE_STREAM*)LocalAlloc(LPTR, sizeof(FILE_STREAM) + m_local_buffer_size * 1024);

        if(fs == NULL)
            return NULL;

        fs->data = (unsigned char *)&fs[1];

        fs->stream = CreateFile(filename, GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
            OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
        if (fs->stream == INVALID_HANDLE_VALUE)
        {
            LocalFree(fs);
            return NULL;
        }
    }

    fs->buffer_length = 0;
    fs->buffer_offset = 0;
    fs->file_offset = 0;

    return fs;
}

int read_byte_filestream(FILE_STREAM *fs)
{
    if(fs->buffer_offset == fs->buffer_length)
    {
        fs->buffer_offset = 0;

        ReadFile(fs->stream, fs->data, m_local_buffer_size * 1024, &fs->buffer_length, 0);

        if(fs->buffer_length <= 0)
        {
            fs->buffer_length = 0;
            return -1;
        }
    }

    fs->file_offset++;

    return fs->data[fs->buffer_offset++];
}

int read_buffer_filestream(FILE_STREAM *fs, void *data, int length)
{
    int i, tmp;
    unsigned char *data2 = (unsigned char *)data;

    for(i=0; i < length; i++)
    {
        if((tmp = read_byte_filestream(fs)) < 0)
        {
            if(i)
            {
                break;
            }
            else
            {
                return -1;
            }
        }
        data2[i] = tmp;
    }

    return i;
}

unsigned long filelength_filestream(FILE_STREAM *fs)
{
    unsigned long fsize;

    fsize = GetFileSize(fs->stream, NULL);

    return fsize;
}

void seek_filestream(FILE_STREAM *fs, unsigned long offset, int mode)
{
    SetFilePointer(fs->stream, offset, NULL, mode);

    if(mode == FILE_CURRENT)
        fs->file_offset += offset;
    else if(mode == FILE_END)
        fs->file_offset = filelength_filestream(fs) + offset;
    else
        fs->file_offset = offset;

    fs->buffer_length = 0;
    fs->buffer_offset = 0;
}

unsigned long tell_filestream(FILE_STREAM *fs)
{
    return fs->file_offset;
}

void close_filestream(FILE_STREAM *fs)
{
    if(fs)
    {
        if(fs->stream)
            CloseHandle(fs->stream);

        LocalFree(fs);
        fs = NULL;
    }
}
