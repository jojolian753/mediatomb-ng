/*MT*
    
    MediaTomb - http://www.mediatomb.cc/
    
    web_request_handler.cc - this file is part of MediaTomb.
    
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
    
    $Id$
*/

/// \file web_request_handler.cc

#ifdef HAVE_CONFIG_H
    #include "autoconfig.h"
#endif

#include <time.h>
#include "mem_io_handler.h"
#include "web_request_handler.h"
#include "config_manager.h"
#include "content_manager.h"
#include "web/pages.h"
#include "tools.h"
#include "hash.h"

using namespace zmm;
using namespace mxml;

WebRequestHandler::WebRequestHandler() : RequestHandler()
{
    checkRequestCalled = false;
}

int WebRequestHandler::intParam(String name, int invalid)
{
    String value = param(name);
    if (!string_ok(value))
        return invalid;
    else
        return value.toInt();
}

bool WebRequestHandler::boolParam(zmm::String name)
{
    String value = param(name);
    return string_ok(value) && (value == "1" || value == "true");
}

void WebRequestHandler::check_request(bool checkLogin)
{
    // we have a minimum set of parameters that are "must have"
    
    // check if the session parameter was supplied and if we have
    // a session with that id
    
    checkRequestCalled = true;
    
    String sid = param(_("sid"));
    if (sid == nil)
        throw SessionException(_("no session id given"));
    
    if ((session = SessionManager::getInstance()->getSession(sid)) == nil)
        throw SessionException(_("invalid session id"));
    
    if (checkLogin && ! session->isLoggedIn())
        throw SessionException(_("not logged in"));
    session->access();
}

String WebRequestHandler::renderXMLHeader()
{
    return _("<?xml version=\"1.0\" encoding=\"") +
            DEFAULT_INTERNAL_CHARSET +"\"?>\n";
}

void WebRequestHandler::get_info(IN const char *filename, OUT struct File_Info *info)
{
    info->file_length = -1; // length is unknown
    info->last_modified = time(NULL);
    info->is_directory = 0;
    info->is_readable = 1;
    
    String contentType;
    
    contentType = _(MIMETYPE_XML) + "; charset=" + DEFAULT_INTERNAL_CHARSET;
    
    info->content_type = ixmlCloneDOMString(contentType.c_str());
    info->http_header = ixmlCloneDOMString("Cache-Control: no-cache, must-revalidate");
}

Ref<IOHandler> WebRequestHandler::open(Ref<Dictionary> params, IN enum UpnpOpenFileMode mode)
{
    this->params = params;
    
    root = Ref<Element>(new Element(_("root")));
    out = Ref<StringBuffer>(new StringBuffer());
    
    String output;
    // processing page, creating output
    try
    {
        if(!ConfigManager::getInstance()->getBoolOption(CFG_SERVER_UI_ENABLED))
        {
            root->setAttribute(_("ui_disabled"), _("1"));
            log_warning("The UI is disabled in the configuration file. See README.\n");
        }
        else
        {
            process();
            
            if (checkRequestCalled)
            {
                // add current task
                appendTask(root, ContentManager::getInstance()->getCurrentTask());
                
                handleUpdateIDs();
            }
        }
    }
    catch (SessionException se)
    {
        //String url = _("/content/interface?req_type=login&slt=") +
        //                    generate_random_id();
        
        root->appendTextChild(_("redirect"), _("/"));
    }
    catch (StorageException e)
    {
        e.printStackTrace();
        root->appendTextChild(_("error"), e.getUserMessage());
    }
    catch (Exception e)
    {
        e.printStackTrace();
        // Ref<Dictionary> par(new Dictionary());
        // par->put("message", e.getMessage());
        // output = subrequest("error", par);
        String errmsg = _("Error: ") + e.getMessage();
        root->appendTextChild(_("error"), errmsg);
    }
    output = renderXMLHeader() + root->print();
    
    //root = nil;
    
    Ref<MemIOHandler> io_handler(new MemIOHandler(output));
    io_handler->open(mode);
    return RefCast(io_handler, IOHandler);
}

Ref<IOHandler> WebRequestHandler::open(IN const char *filename,
                                       OUT struct File_Info *info,
                                       IN enum UpnpOpenFileMode mode)
{
    log_debug("request: %s\n", filename);
    this->filename = String((char *)filename);
    this->mode = mode;

    String path, parameters;
    split_url(filename, URL_UI_PARAM_SEPARATOR, path, parameters);

    Ref<Dictionary> params = Ref<Dictionary>(new Dictionary());
    params->decode(parameters);
    get_info(NULL, info);
    return open(params, mode);
}

void WebRequestHandler::handleUpdateIDs()
{
    // session will be filled by check_request
    
    String uiUpdate = param(_("get_update_ids"));
    if ((string_ok(uiUpdate) && uiUpdate == _("1")))
    {
//        log_debug("UI wants updates.\n");
        String forceUpdate = param(_("force_update"));
        if ((string_ok(forceUpdate) && forceUpdate ==_("1")))
        {
//            log_debug("UI forces updates.\n");
            addUpdateIDs(root, session);
        }
        else
        {
            Ref<Element> updateIDs(new Element(_("updateIDs")));
            root->appendElementChild(updateIDs);
            if (session->hasUIUpdateIDs())
            {
//                log_debug("UI updates pending...\n");
                updateIDs->setAttribute(_("pending"), _("1"));
            }
            else
            {
//                log_debug("no UI updates.\n");
            }
        }
    }
}

void WebRequestHandler::addUpdateIDs(Ref<Element> root, Ref<Session> session)
{
    Ref<Element> updateIDsEl(new Element(_("updateIDs")));
    root->appendElementChild(updateIDsEl);
    
    String updateIDs = session->getUIUpdateIDs();
    if (string_ok(updateIDs))
    {
        log_debug("UI: sending update ids: %s\n", updateIDs.c_str());
        updateIDsEl->setText(updateIDs);
        updateIDsEl->setAttribute(_("updates"), _("1"));
    }
}

void WebRequestHandler::appendTask(Ref<Element> el, Ref<CMTask> task)
{
    if (task == nil || el == nil)
        return;
    Ref<Element> taskEl (new Element(_("task")));
    taskEl->setAttribute(_("id"), String::from(task->getID()));
    taskEl->setAttribute(_("cancellable"), task->isCancellable() ? _("1") : _("0"));
    taskEl->setText(task->getDescription());
    el->appendElementChild(taskEl);
}
