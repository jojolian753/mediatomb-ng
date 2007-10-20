/*MT*
    
    MediaTomb - http://www.mediatomb.cc/
    
    process_io_handler.cc - this file is part of MediaTomb.
    
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

/// \file process_io_handler.cc

#ifdef HAVE_CONFIG_H
    #include "autoconfig.h"
#endif

#ifdef TRANSCODING

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <signal.h>
#include <unistd.h>
#include "common.h"
#include "process_io_handler.h"
#include "process.h"
#include "content_manager.h"

using namespace zmm;

ProcListItem::ProcListItem(Ref<Executor> exec, bool abortOnDeath)
{
    executor = exec;
    abort = abortOnDeath;
}

Ref<Executor> ProcListItem::getExecutor()
{
    return executor;
}

bool ProcListItem::abortOnDeath()
{
    return abort;
}

bool ProcessIOHandler::abort()
{
    bool abort = false;

    if (proclist == nil)
        return abort; 

    for (int i = 0; i < proclist->size(); i++)
    {
        Ref<Executor> exec = proclist->get(i)->getExecutor();
        if ((exec != nil) && (!exec->isAlive()))
        {
            if (proclist->get(i)->abortOnDeath())
                abort = true;
            break;
        }
    }

    return abort;
}

void ProcessIOHandler::killall()
{
    if (proclist == nil)
        return;

    for (int i = 0; i < proclist->size(); i++)
    {
        Ref<Executor> exec = proclist->get(i)->getExecutor();
        if (exec != nil)
            exec->kill();
    }
}

ProcessIOHandler::ProcessIOHandler(String filename, 
                        zmm::Ref<Executor> main_proc,
                        zmm::Ref<zmm::Array<ProcListItem> > proclist) : IOHandler()
{
    this->filename = filename;
    this->proclist = proclist;
    this->main_proc = main_proc;

    if (!main_proc->isAlive() || abort())
    {
        unlink(filename.c_str());
        killall();
        throw _Exception(_("process terminated early"));
    }
//    ContentManager::getInstance()->registerProcess(kill_pid, filename);
}

void ProcessIOHandler::open(IN enum UpnpOpenFileMode mode)
{
    if (!main_proc->isAlive() || abort())
    {
        killall();
        throw _Exception(_("process terminated early"));
    }

    fd = ::open(filename.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd == -1)
    {
        killall();
        main_proc->kill();
        throw _Exception(_("open: failed to open: ") + filename.c_str());
    }
}

int ProcessIOHandler::read(OUT char *buf, IN size_t length)
{
    fd_set readSet;
    struct timeval timeout;
    ssize_t bytes_read = 0;
    int num_bytes = 0;
    char* p_buffer = buf;
    int exit_status = EXIT_SUCCESS;
    int ret = 0;

    while (true)
    {
        FD_ZERO(&readSet);
        FD_SET(fd, &readSet);

        timeout.tv_sec = FIFO_READ_TIMEOUT;
        timeout.tv_usec = 0;

        ret = select(fd + 1, &readSet, NULL, NULL, &timeout);
        if (ret == -1)
        {
            if (errno == EINTR)
                continue;
        }

        // timeout
        if (ret == 0)
        {
            bool main_ok = main_proc->isAlive();
            if (!main_ok || abort())
            {
                if (!main_ok)
                {
                    exit_status = main_proc->getStatus();
                    log_debug("process exited with status %d\n", exit_status);
                    killall();
                    if (exit_status == EXIT_SUCCESS)
                        return 0; 
                    else
                        return -1;
                }
                else
                {
                    main_proc->kill();
                    killall();
                    return -1;
                }
            }
        }

        if (FD_ISSET(fd, &readSet))
        {
            bytes_read = ::read(fd, p_buffer, length);
            if (bytes_read == 0)
                break;

            if (bytes_read < 0)
            {
                log_debug("aborting read!!!\n");
                return -1;
            }

            num_bytes = num_bytes + bytes_read;
            length = length - bytes_read;

            if (length <= 0)
                break;

            p_buffer = buf + num_bytes;
        }
    }

    if (num_bytes < 0)
    {
        // not sure what we return here since no way of knowing about feof
        // actually that will depend on the ret code of the process
        ret = -1;

        if (!main_proc->isAlive())
        {
            if (main_proc->getStatus() == EXIT_SUCCESS)
                ret = 0;

        }
        else
        {
            main_proc->kill();
        }
        killall();
        return ret;
    }

    return num_bytes;
}

void ProcessIOHandler::seek(IN off_t offset, IN int whence)
{   
    // we know we can not seek in a fifo
    throw _Exception(_("fseek failed"));
}

void ProcessIOHandler::close()
{
    bool ret;
   

    log_debug("terminating process, closing %s\n", this->filename.c_str());

//    ContentManager::getInstance()->unregisterProcess(kill_pid);
    ret = main_proc->kill();
    killall();
    
    ::close(fd);
    unlink(filename.c_str());

    if (!ret)
        throw _Exception(_("failed to kill process!"));
}

#endif // TRANSCODING

