/*MT*
    
    MediaTomb - http://www.mediatomb.cc/
    
    buffered_io_handler.cc - this file is part of MediaTomb.
    
    Copyright (C) 2005 Gena Batyan <bgeradz@mediatomb.cc>,
                       Sergey 'Jin' Bostandzhyan <jin@mediatomb.cc>
    
    Copyright (C) 2006-2007 Gena Batyan <bgeradz@mediatomb.cc>,
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
    
    $Id$
*/

/// \file buffered_io_handler.cc

#ifdef HAVE_CONFIG_H
    #include "autoconfig.h"
#endif

#include "buffered_io_handler.h"
#include "tools.h"

using namespace zmm;

BufferedIOHandler::BufferedIOHandler(Ref<IOHandler> underlyingHandler, size_t bufSize, size_t maxChunkSize, size_t initialFillSize) : IOHandlerBufferHelper(bufSize, initialFillSize)
{
    if (underlyingHandler == nil)
        throw _Exception(_("underlyingHandler must not be nil"));
    if (maxChunkSize <= 0)
        throw _Exception(_("maxChunkSize must be positive"));
    this->underlyingHandler = underlyingHandler;
    this->maxChunkSize = maxChunkSize;
}

void BufferedIOHandler::open(IN enum UpnpOpenFileMode mode)
{
    // do the open here instead of threadProc() because it may throw an exception
    underlyingHandler->open(mode);
    IOHandlerBufferHelper::open(mode);
}

void BufferedIOHandler::close()
{
    IOHandlerBufferHelper::close();
    // do the close here instead of threadProc() because it may throw an exception
    underlyingHandler->close();
}

void BufferedIOHandler::threadProc()
{
    int readBytes;
    size_t maxWrite;
    
#ifdef LOG_TOMBDEBUG
    struct timespec last_log;
    bool first_log = true;
#endif
    
    AUTOLOCK(mutex);
    do
    {
        
#ifdef LOG_TOMBDEBUG
        if (first_log || getDeltaMillis(&last_log) > 1000)
        {
            if (first_log)
                first_log = false;
            getTimespecNow(&last_log);
            float percentFillLevel = 0;
            if (! empty)
            {
                int currentFillSize = b - a;
                if (currentFillSize <= 0)
                    currentFillSize += bufSize;
                percentFillLevel = ((float)currentFillSize / (float)bufSize) * 100;
            }
            log_debug("buffer fill level: %3.2f%%  (bufSize: %d; a: %d; b: %d)\n", percentFillLevel, bufSize, a, b);
        }
#endif
        if (empty)
            a = b = 0;
        maxWrite = (empty ? bufSize: (a < b ? bufSize - b : a - b));
        if (maxWrite == 0)
        {
            cond->wait();
        }
        else
        {
            AUTOUNLOCK();
            size_t chunkSize = (maxChunkSize > maxWrite ? maxWrite : maxChunkSize);
            readBytes = underlyingHandler->read(buffer + b, chunkSize);
            AUTORELOCK();
            if (readBytes > 0)
            {
                b += readBytes;
                assert(b <= bufSize);
                if (b == bufSize)
                    b = 0;
                if (empty)
                {
                    empty = false;
                    cond->signal();
                }
                if (waitForInitialFillSize)
                {
                    size_t currentFillSize = b - a;
                    if (currentFillSize <= 0)
                        currentFillSize += bufSize;
                    if (currentFillSize >= initialFillSize)
                    {
                        log_debug("buffer: initial fillsize reached\n");
                        waitForInitialFillSize = false;
                        cond->signal();
                    }
                }
            }
        }
    }
    while((maxWrite == 0 || readBytes > 0) && ! threadShutdown);
    if (! threadShutdown)
    {
        if (readBytes == 0)
            eof = true;
        if (readBytes < 0)
            readError = true;
    }
    // ensure that read() doesn't wait for me to fill the buffer
    cond->signal();
}
