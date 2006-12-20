/*MT*
    
    MediaTomb - http://www.mediatomb.org/
    
    filesystem.h - this file is part of MediaTomb.
    
    Copyright (C) 2005 Gena Batyan <bgeradz@mediatomb.org>,
                       Sergey Bostandzhyan <jin@mediatomb.org>
    Copyright (C) 2006 Gena Batyan <bgeradz@mediatomb.org>,
                       Sergey Bostandzhyan <jin@mediatomb.org>,
                       Leonhard Wimmer <leo@mediatomb.org>
    
    MediaTomb is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.
    
    MediaTomb is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    version 2 along with MediaTomb; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    
    $Id$
*/

/// \file filesystem.h

#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "autoconfig.h"
#include "common.h"
#include "rexp.h"

#define FS_MASK_FILES 1
#define FS_MASK_DIRECTORIES 2
#define FS_MASK_HIDDEN 4 // accept hidden files (start with . "dot")

#ifdef __CYGWIN__
#define FS_ROOT_DIRECTORY "/cygdrive/"
#else
#define FS_ROOT_DIRECTORY "/"
#endif

class FsObject : public zmm::Object
{
public:
    inline FsObject() : zmm::Object()
    {
        isDirectory = false;
        hasContent = false;
    }
public:
    zmm::String filename;
    bool isDirectory;
    bool hasContent;
};

class Filesystem : public zmm::Object
{
public:
    Filesystem();

    zmm::Ref<zmm::Array<FsObject> > readDirectory(zmm::String path, int mask,
                                                  int chldMask = 0);
    bool haveFiles(zmm::String dir);
    bool haveDirectories(zmm::String dir);
    bool fileAllowed(zmm::String path);
    
protected:
    zmm::Ref<zmm::Array<RExp> > includeRules;
    bool have(zmm::String dir, int mask);
};

#endif // __FILESYSTEM_H__

