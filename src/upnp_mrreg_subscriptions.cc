/*MT*
    
    MediaTomb - http://www.mediatomb.org/
    
    upnp_mrreg_subscriptions.cc - this file is part of MediaTomb.
    
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

/// \file upnp_mrreg_subscriptions.cc

#ifdef HAVE_CONFIG_H
    #include "autoconfig.h"
#endif

#include "tools.h"
#include "upnp_mrreg.h"
#include "server.h"
#include "upnp_xml.h"
#include "ixml.h"
#include "storage.h"

using namespace zmm;
using namespace mxml;

void MRRegistrarService::process_subscription_request(zmm::Ref<SubscriptionRequest> request)
{
    int err;
    IXML_Document *event = NULL;

    Ref<Element> propset, property;

    propset = UpnpXML_CreateEventPropertySet();
    property = propset->getFirstChild();
    property->appendTextChild(_("ValidationRevokedUpdateID"), _("0"));
    property->appendTextChild(_("ValidationSucceededUpdateID"), _("0"));
    property->appendTextChild(_("AuthorizationDeniedUpdateID"), _("0"));
    property->appendTextChild(_("AuthorizationGrantedUpdateID"), _("0"));

    String xml = propset->print();
    err = ixmlParseBufferEx(xml.c_str(), &event);
    if (err != IXML_SUCCESS)
    {
        throw UpnpException(UPNP_E_SUBSCRIPTION_FAILED, _("Could not convert property set to ixml"));
    }

    UpnpAcceptSubscriptionExt(Server::getInstance()->getDeviceHandle(),
            ConfigManager::getInstance()->getOption(_("/server/udn")).c_str(),
            serviceID.c_str(), event, request->getSubscriptionID().c_str());

    ixmlDocument_free(event);
}
#if 0
void MRRegistrarService::subscription_update(String sourceProtocol_CSV)
{
    int err;
    IXML_Document *event = NULL;

    Ref<Element> propset, property;

    propset = UpnpXML_CreateEventPropertySet();
    property = propset->getFirstChild();
    property->appendTextChild(_("SourceProtocolInfo"), sourceProtocol_CSV);

    String xml = propset->print();

    err = ixmlParseBufferEx(xml.c_str(), &event);
    if (err != IXML_SUCCESS)
    {
        /// \todo add another error code
        throw UpnpException(UPNP_E_SUBSCRIPTION_FAILED, _("Could not convert property set to ixml"));

    }

    UpnpNotifyExt(Server::getInstance()->getDeviceHandle(),
            ConfigManager::getInstance()->getOption(_("/server/udn")).c_str(),
            serviceID.c_str(), event);

    ixmlDocument_free(event);
}
#endif
