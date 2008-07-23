/*MT*
    
    MediaTomb - http://www.mediatomb.cc/
    
    dvd_read.h - this file is part of MediaTomb.
    
    Copyright (C) 2005 Gena Batyan <bgeradz@mediatomb.cc>,
                       Sergey 'Jin' Bostandzhyan <jin@mediatomb.cc>
    
    Copyright (C) 2006-2008 Gena Batyan <bgeradz@mediatomb.cc>,
                            Sergey 'Jin' Bostandzhyan <jin@mediatomb.cc>,
                            Leonhard Wimmer <leo@mediatomb.cc>
    
    MediaTomb is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.
    
    MediaTomb is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    version 2 along with MediaTomb; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
    
    $Id: file_io_handler.h 1698 2008-02-23 20:48:30Z lww $
*/

/// \file dvd_read.h
/// \brief Definition of the FileIOHandler class.
#ifndef __DVD_READ_H__
#define __DVD_READ_H__

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include <sys/types.h>
#include <dvdread/dvd_reader.h>
#include <dvdread/ifo_read.h>
#include <dvdread/nav_types.h>
#include "common.h"
#include "sync.h"

/// \brief Allows to read selected streams from a DVD image.
///
/// There are some constraints on the usage of this class:
/// First of all - you *must* call the selectPGC() function before you 
/// attempt to read data or get the stream length.
///
/// You must call selectPGC() each time when you want to reset read; calling
/// read consequently will return the stream data and advance further in the
/// stream.
/// 
/// The class is thread safe, meaning that locks are in place, however it's
/// design does not suggest multithreaded usage. selectPGC(), read(), 
/// getLength() will not work in parallel but block if one of the functions
/// is running.
class DVDReader : public zmm::Object
{
public:
    /// \brief Sets the filename to work with. Can be an ISO, device or
    /// directory.
    DVDReader(zmm::String path);
    ~DVDReader();

    /// \brief returns the number of titles on the DVD
    int titleCount();

    /// \brief returns the number of chapters for a given title
    /// \param title_idx index of the title
    int chapterCount(int title_idx);

    /// \brief returns the number of angles for a given title
    int angleCount(int title_idx);

    /// \brief Returns the number of audio streams for the selected PGC
    int audioTrackCount();

    void dumpVMG();

    /// \brief Selects the title, chapter and angle to play, returns the
    /// number of bytes for the selection.
    ///
    /// The DVD is divided into titles, each title is didvided into chapters
    /// and can have one or more angles. This function selects what data we
    /// want to retrieve from the DVD (i.e. what stream we want to watch),
    /// and returns the size of the stream in bytes. 
    /// Note, that we will always treat the chapter as "starting point", i.e.
    /// we will play from the given chapter to the very end, we will not stop
    /// at the end of the chapter.
    ///
    /// \param title_idx index of the title
    /// \param chapter_idx index of the chapter from where we start
    /// \param angle_idx index of the angle
    void selectPGC(int title_idx, int chapter_idx, int angle_idx);

    /// \brief Reads the stream, specified by selectPGC from the DVD.
    ///
    /// \param buffer buffer to store the data
    /// \param length length of the buffer
    /// \return number of bytes read (can be shorter than buffer length), value
    ///  of 0 indicates end of stream.
    size_t readSector(unsigned char *buffer, size_t length);  

    /// \brief Calculates the length of the currently selected PGC
    /// 
    /// In order to get the length we have to parse the DVD meaning that 
    /// this call may take a little while.
    /// 
    /// There are two approaches, one determines the exact size of the stream,
    /// however it comes at a high cost. We have to simulate reading and parse
    /// the NAV packets so we can determine the correct angle. This is what
    /// the precise parameter is doing - expect delays up to 10 seconds and
    /// more.
    /// Setting precise to false will deliver an approximate length - without
    /// angle calculations, meaning that the length can be off by some amount.
    /// We will try to make sure that this amount is still greater than the
    /// actual length, the worst that can happen will be a seek beyond end of
    /// stream which should not be too bad. The approximate length calculation
    /// is quite fast.
    ///
    /// \param precise if true turns on precise length calculation which is slow
    off_t length(bool precise = true);

    /// \brief Retrieves the length of a given chapter.
    off_t chapterLength(int chapter_idx, bool precise);

    /// \brief Returns the duration in seconds for the currently selected PGC.
    int duration();

    /// \brief Returns the duration in seconds of the entire title
    int titleDuration();

    /// \brief Returns the duration in seconds of the selected chapter
    int chapterDuration(int chapter_idx);

    /// \brief Returns a human readable language string of the audio stream
    zmm::String audioLanguage(int stream_idx);

    /// \brief Returns the sampling frequency of the given audio stream
    int audioSampleFrequency(int stream_idx);

    /// \brief Returns the number of channels for the given audio stream
    int audioChannels(int stream_idx);

    /// \brief Returns a human readable name of the audio format
    zmm::String audioFormat(int stream_idx);

protected:
    /// \brief Name of the DVD file.
    zmm::String dvd_path;

    /// \brief DVD handle
    dvd_reader_t *dvd;

    /// \brief Video Manager handle
    ifo_handle_t *vmg;

    /// \brief Currently selected Program Chain
    pgc_t *pgc;

    /// \brief Currently selected Video Title Set
    ifo_handle_t *vts;

    /// \brief Currently selected Title
    dvd_file_t *title;

    zmm::Ref<Mutex> mutex;

    int title_index;
    int chapter_index;
    int angle_index;
    int start_cell;
    int next_cell;
    int current_cell;
    int current_pack;
    //int last_pack;

    /// \brief flag that indicates that we continue with the current sector
    /// when reading.
    bool cont_sector;


    /// \brief selectPGC will set these variables to the initial values,
    /// they will then be used by the length calculation function which is
    /// independant of the read function.
    int nc_len;
    int cc_len;
    int cp_len;
    bool cs_len;

    /// \brief Internal read function which is also used for calculating the
    /// length of the selected stream.
    ///
    /// \param read when set to false only packets which are needed to
    /// follow the stream are read, the contents of the buffer are unusable
    /// on return, however the length of the simulated read is returned
    /// correctly.
    ///
    /// \param chapter_idx if not -1 then we will only read the data till the
    /// end of the given chapter
    /// \return number of bytes read, 0 for EOF
    size_t readSectorInternal(unsigned char *buffer, size_t length, bool read,
                              bool till_eof = true);

    /// \brief Calculates the length of the stream, expect delays up to 10
    /// seconds and more.
    off_t calculateExactLength();

    /// \brief Calculates the length of the stream but ignores angles and
    /// does not follow the cells correctly, this fast and the result is
    /// good enough, but the returned length will not be the exact value.
    off_t calculateApproxLength();

    long dvdtime2msec(dvd_time_t *dt);

    zmm::String getLanguage(char *code);

#ifdef TOMBDEBUG
    void dumpGlobalVars();
#endif
};


#endif // __DVD_READ_H__
