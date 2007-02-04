/*MT*
    
    MediaTomb - http://www.mediatomb.org/
    
    tools.js - this file is part of MediaTomb.
    
    Copyright (C) 2005 Gena Batyan <bgeradz@mediatomb.org>,
                       Sergey 'Jin' Bostandzhyan <jin@mediatomb.org>
    
    Copyright (C) 2006-2007 Gena Batyan <bgeradz@mediatomb.org>,
                            Sergey 'Jin' Bostandzhyan <jin@mediatomb.org>,
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

var INACTIVITY_TIMEOUT = 5000;

function link(req_type, param, get_update_ids)
{
    var url = "/content/interface?req_type="+ encodeURI(req_type) +"&sid="+ SID;
    if (param)
        for (var key in param)
        {
            url += "&" + encodeURI(key) +"="+ encodeURI(param[key]);
        }
    if (get_update_ids && isTypeDb() && dbStuff.treeShown)
        url += "&get_update_ids=1";
    return url;
}

function isTypeDb()
{
    return (TYPE == "db");
}

function xmlGetElement(parent, name)
{
    var returnVal = parent.getElementsByTagName(name)
    
    if (!returnVal || !returnVal.length)
        return null;
    else
        return returnVal[0];
}

function xmlGetElementText(parent, name)
{
    var el = xmlGetElement(parent, name);
    return xmlGetText(el);
}
function xmlGetText(el)
{
    if (!el || el.childNodes.length != 1)
        return null;
    return el.firstChild.nodeValue;
}

function xmlGetAttribute(parent, name)
{
    var a = parent.attributes;
    if (! a)
        return null;
    for (var i = 0; i < a.length; i++)
    {
        if (a[i].name == name)
            return a[i].value;
    }
    return null;
}

function appendImgNode(document, node, alt, icon, width, height)
{
    var img = document.createElement("img");
    img.setAttribute("src", icon.src);
    img.setAttribute("alt", alt);
    img.setAttribute("border", "0");
    img.setAttribute("width", icon.width);
    img.setAttribute("height", icon.height);
    node.appendChild(img);
}

var timer;

function errorCheck(xml, noredirect)
{
    if (!xml)
    {
        //alert ("could not fetch XML");
        return false;
    }
    var rootEl = xmlGetElement(xml, 'root');
    
    var uiDisabled = xmlGetAttribute(rootEl, 'ui_disabled');
    if (uiDisabled)
    {
        window.location = "/disabled.html";
        return false;
    }
    
    var redirect = xmlGetElementText(xml, 'redirect');
    if (redirect)
    {
        var now = new Date();
        var expire = new Date();
        expire.setTime(now.getTime() - 3600000 * 24 * 360);
        setCookie('SID', null, expire);
        if (SID && ! noredirect)
        {
            SID = null;
            window.location = redirect;
        }
        return false;
    }
    
    // clears current task if no task element
    updateCurrentTask(xmlGetElement(xml, 'task'));
    
    var updateIDsEl = xmlGetElement(xml, 'updateIDs');
    if (updateIDsEl)
        handleUIUpdates(updateIDsEl);
    
    var error = xmlGetElementText(xml, 'error');
    if (error)
    {
        alert(error);
        return false;
    }
    return true;
}

function handleUIUpdates(updateIDsEl)
{
    if (updateIDsEl.getAttribute("pending") == '1')
    {
        setStatus("updates_pending");
        if (! timer)
            timer = window.setTimeout("getUpdates(true)", INACTIVITY_TIMEOUT);
        last_update = new Date().getTime();
    }
    else if (updateIDsEl.getAttribute("updates") != '1')
    {
        setStatus("no_updates");
        if (timer)
        {
            window.clearTimeout(timer);
            timer = false;
        }
        last_update = new Date().getTime();
    }
    else
    {
        var updateIDStr = xmlGetText(updateIDsEl);
        var savedlastNodeDbID = lastNodeDb;
        var savedlastNodeDbIDParent;
        if (savedlastNodeDbID != 'd0')
            savedlastNodeDbIDParent = getTreeNode(savedlastNodeDbID).getParent().getID();
        //alert('before: lastid: ' + savedlastNodeDbID);
        //alert('before: node: ' + getTreeNode(savedlastNodeDbID));
        selectNodeIfVisible('d0');
        var updateAll = false;
        if (updateIDStr != 'all')
        {
            var updateIDsArr = updateIDStr.split(",");
            for (var i = 0; i < updateIDsArr.length; i++)
            {
                if (updateIDsArr[i] == 0)
                    updateAll = true;
            }
        }
        else
            updateAll = true;
        if (! updateAll)
        {
            for (var i = 0; i < updateIDsArr.length; i++)
            {
                var node = getTreeNode('d' + updateIDsArr[i]);
                if (node)
                {
                    var parNode = node.getParent();
                    if (parNode)
                    {
                        parNode.childrenHaveBeenFetched=false;
                        parNode.resetChildren();
                        fetchChildren(parNode, true);
                    }
                }
            }
        }
        else
        {
            var node = getTreeNode('d0');
            node.childrenHaveBeenFetched=false;
            node.resetChildren();
            fetchChildren(node, true);
        }
        var savedlastNodeDb = getTreeNode(savedlastNodeDbID);
        if (savedlastNodeDb)
            selectNodeIfVisible(savedlastNodeDbID);
        else if (savedlastNodeDbIDParent)
        {
            savedlastNodeDb = getTreeNode(savedlastNodeDbIDParent);
            if (savedlastNodeDb)
                selectNodeIfVisible(savedlastNodeDbIDParent);
        }
        setStatus("no_updates");
        if (timer)
        {
            window.clearTimeout(timer);
            timer = false;
        }
        last_update = new Date();
    }
}

function addBr(useDocument, element)
{
    var newBr = useDocument.createElement('br');
    element.appendChild(newBr);
}

function getTopOffset(el)
{
    var offset = 0;
    while (el != null)
    {
        offset += el.offsetTop;
        el = el.offsetParent;
    }
    return offset;
}

function ensureElementVisibility(win, doc, el, bottomAdd)
{
    var viewheight;
    if (win.innerHeight) // all except IE
    {
        viewheight = win.innerHeight;
    }
    else if (doc.documentElement && doc.documentElement.clientHeight)
        // IE 6 Strict Mode
    {
        viewheight = doc.documentElement.clientHeight;
    }
    else if (doc.body) // other IEs
    {
        viewheight = doc.body.clientHeight;
    }
    
    var scrollpos;
    var xoffset;
    if (win.pageYOffset) // all except IE
    {
        xoffset = win.pageXOffset;
        scrollpos = win.pageYOffset;
    }
    else if (doc.documentElement && doc.documentElement.scrollTop)
        // IE 6 Strict
    {
        xoffset = doc.documentElement.scrollLeft;
        scrollpos = doc.documentElement.scrollTop;
    }
    else if (document.body) // all other IEs
    {
        xoffset = doc.body.scrollLeft;
        scrollpos = doc.body.scrollTop;
    }
    
    var elpos = getTopOffset(el);
    
    if (elpos<scrollpos)
    {
        win.scrollTo(xoffset, elpos);
    }
    else if (elpos>scrollpos+viewheight-bottomAdd)
    {
        win.scrollTo(xoffset, elpos-viewheight+bottomAdd);
    }
}

function formToArray(form, args)
{
    for (var i = 0; i < form.length; ++i)
    {
        var element = form.elements[i];
        
        if (element.type && ((element.type != 'submit' && element.type != "radio" && element.type != "checkbox") || element.checked) && ! element.disabled)
            args[element.name] = element.value;
    }
}

var status_updates_pending = false;

function setStatus(status)
{
    var topDocument = frames["topF"].document;
    
    if (status == "idle")
    {
        Element.hide(topDocument.getElementById("statusWorking"));
        Element.show(topDocument.getElementById("statusIdle"));
    }
    else if (status == "loading")
    {
        Element.hide(topDocument.getElementById("statusIdle"));
        Element.show(topDocument.getElementById("statusWorking"));
    }
    else if (status == "updates_pending" && ! status_updates_pending)
    {
        Element.hide(topDocument.getElementById("statusNoUpdates"));
        Element.show(topDocument.getElementById("statusUpdatesPending"));
        status_updates_pending = true;
    }
    else if (status == "no_updates" && status_updates_pending)
    {
        Element.hide(topDocument.getElementById("statusUpdatesPending"));
        Element.show(topDocument.getElementById("statusNoUpdates"));
        status_updates_pending = false;
    }
}

function getUpdates(force)
{
    if (loggedIn)
    {
        var url = link('update', {force_update: (force ? '1' : '0')}, true);
        var myAjax = new Ajax.Request(
            url,
            {
                method: 'get',
                asynchronous: false,
                onComplete: getUpdatesCallback
            });
    }
}

function getUpdatesCallback(ajaxRequest)
{
    var xml = ajaxRequest.responseXML;
    if (! errorCheck(xml))
        return;
    last_update = new Date().getTime();
}

function userActivity(event)
{
    if (timer)
    {
        window.clearTimeout(timer);
        timer = window.setTimeout("getUpdates(true)", INACTIVITY_TIMEOUT);
    }
    return true;
}

var last_update = new Date().getTime();

function mouseDownHandler(event)
{
    userActivity();
    var now = new Date().getTime();
    if (last_update + 3000 < now)
    {
        getUpdates(false);
        last_update = now;
    }
}

