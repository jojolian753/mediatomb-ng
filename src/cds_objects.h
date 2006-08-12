/*  cds_objects.h - this file is part of MediaTomb.

    Copyright (C) 2005 Gena Batyan <bgeradz@deadlock.dhs.org>,
                       Sergey Bostandzhyan <jin@deadlock.dhs.org>

    MediaTomb is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    MediaTomb is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MediaTomb; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/// \file cds_objects.h
/// \brief Definition for the CdsObject, CdsItem, CdsActiveItem and CdsContainer classes.
#ifndef __CDS_OBJECTS_H__
#define __CDS_OBJECTS_H__

#include "common.h"
#include "dictionary.h"
#include "cds_resource.h"

// ATTENTION: These values need to be changed in web/js/items.js too.
#define OBJECT_TYPE_CONTAINER 1
#define OBJECT_TYPE_ITEM 2
#define OBJECT_TYPE_ACTIVE_ITEM 4
#define OBJECT_TYPE_ITEM_EXTERNAL_URL 8
#define OBJECT_TYPE_ITEM_INTERNAL_URL 16

#define IS_CDS_CONTAINER(type) (type & OBJECT_TYPE_CONTAINER)
#define IS_CDS_ITEM(type) (type & OBJECT_TYPE_ITEM)
#define IS_CDS_ACTIVE_ITEM(type) (type & OBJECT_TYPE_ACTIVE_ITEM)
#define IS_CDS_ITEM_EXTERNAL_URL(type) (type & OBJECT_TYPE_ITEM_EXTERNAL_URL)
#define IS_CDS_ITEM_INTERNAL_URL(type) (type & OBJECT_TYPE_ITEM_INTERNAL_URL)

#define INVALID_OBJECT_ID (-333)

int CdsObjectTitleComparator(void *arg1, void *arg2);

/// \brief Generic object in the Content Directory.
class CdsObject : public zmm::Object
{
protected:
    /// \brief ID of the object in the content directory
    int id;

    /// \brief ID of the referenced object
    int refID;

    /// \brief ID of the object's parent
    int parentID;

    /// \brief restricted flag
    bool restricted;

    /// \brief dc:title
    zmm::String title;

    /// \brief upnp:class
    zmm::String upnpClass;

    /// \brief Physical location of the media.
    zmm::String location;

    /// \brief virtual object flag
    int virt;

    /// \brief type of the object: item, container, etc.
    int objectType;

    zmm::Ref<Dictionary> metadata;
    zmm::Ref<Dictionary> auxdata;
    zmm::Ref<zmm::Array<CdsResource> > resources;

public:
    /// \brief Constructor, currently only sets the restricted flag to 1
    CdsObject();

    /// \brief Set the object ID.
    inline void setID(int id) { this->id = id; }

    /// \brief Retrieve the object ID.
    inline int getID() { return id; }

    /// \brief Set the object ID.
    inline void setRefID(int refID) { this->refID = refID; }

    /// \brief Retrieve the object ID.
    inline int getRefID() { return refID; }

    /// \brief Set the parent ID of the object.
    inline void setParentID(int parentID) { this->parentID = parentID; }

    /// \brief Retrieve the objects parent ID.
    inline int getParentID() { return parentID; }

    /// \brief Set the restricted flag.
    inline void setRestricted(int restricted) { this->restricted = restricted; }

    /// \brief Query the restricted flag.
    inline bool isRestricted() { return restricted; }

    /// \brief Set the object title (dc:title)
    inline void setTitle(zmm::String title) { this->title = title; }

    /// \brief Retrieve the title.
    inline zmm::String getTitle() { return title; }

    /// \brief set the upnp:class
    inline void setClass(zmm::String upnpClass) { this->upnpClass = upnpClass; }

    /// \brief Retrieve class
    inline zmm::String getClass() { return upnpClass; }

    /// \brief Set the physical location of the media (usually an absolute path)
    inline void setLocation(zmm::String location) { this->location = location; }

    /// \brief Retrieve media location.
    inline zmm::String getLocation() { return location; }

    /// \brief Set the virtual flag.
    inline void setVirtual(bool virt) { this->virt = virt; }

    /// \brief Query the virtual flag.
    inline bool isVirtual() { return virt; }

    /// \brief Query information on object type: item, container, etc.
    inline int getObjectType() { return objectType; }

    /// \brief Query single metadata value.
    inline zmm::String getMetadata(zmm::String key)
    { return metadata->get(key); }

    /// \brief Query entire metadata dictionary.
    inline zmm::Ref<Dictionary> getMetadata() { return metadata; }
    
    /// \brief Set entire metadata dictionary.
    inline void setMetadata(zmm::Ref<Dictionary> metadata)
    { this->metadata = metadata; }

    /// \brief Set a single metadata value.
    inline void setMetadata(zmm::String key, zmm::String value)
    { metadata->put(key, value); }

    /// \brief Removes metadata with the given key
    inline void removeMetadata(zmm::String key)
    { metadata->remove(key); }
    

    /// \brief Query single auxdata value.
    inline zmm::String getAuxData(zmm::String key)
    { return auxdata->get(key); }

    /// \brief Query entire auxdata dictionary.
    inline zmm::Ref<Dictionary> getAuxData() { return auxdata; }

    /// \brief Set a single auxdata value.
    inline void setAuxData(zmm::String key, zmm::String value)
    { auxdata->put(key, value); }
    
    /// \brief Set entire auxdata dictionary.
    inline void setAuxData(zmm::Ref<Dictionary> auxdata)
    { this->auxdata = auxdata; }

    /// \brief Removes auxdata with the given key
    inline void removeAuxData(zmm::String key)
    { auxdata->remove(key); }
    


    /// \brief Get number of resource tags
    inline int getResourceCount() { return resources->size(); }

    /// \brief Query resources
    inline zmm::Ref<zmm::Array<CdsResource> > getResources()
    { return resources; }
    
    /// \brief Query resource tag with the given index
    inline zmm::Ref<CdsResource> getResource(int index)
    { return resources->get(index); }
    
    /// \brief Add resource tag
    inline void addResource(zmm::Ref<CdsResource> resource)
    { resources->append(resource); } 
    
    /// \brief Copies all object properties to another object.
    /// \param obj target object (clone)
    virtual void copyTo(zmm::Ref<CdsObject> obj);

    /// \brief Checks if current object is equal to obj.
    /// \param obj object to check against
    /// \param exactly tells to check really everything or only the "public" version
    ///
    /// The difference between setting this flag to true or false is following:
    /// exactly=true checks all fields, also internal ones, exactly=false checks
    /// only the fields that will be visible in DIDL-Lite
    virtual int equals(zmm::Ref<CdsObject> obj, bool exactly=false);

    /// \brief Checks if the minimum required parameters for the object have been set and are valid.
    virtual void validate();

    /// \brief Frees unnecessary memory
    void optimize();

    static zmm::Ref<CdsObject> createObject(int objectType);

	friend int CdsObjectTitleComparator(void *arg1, void *arg2);
};

/// \brief An Item in the content directory.
class CdsItem : public CdsObject
{
protected:
    /// \brief mime-type of the media.
    zmm::String mimeType;

public:
    /// \brief Constructor, sets the object type and default upnp:class (object.item)
    CdsItem();

    /// \brief Set mime-type information of the media.
    inline void setMimeType(zmm::String mimeType) { this->mimeType = mimeType; }

    /// \brief Query mime-type information.
    inline zmm::String getMimeType() { return mimeType; }

    /// \brief Copies all object properties to another object.
    /// \param obj target object (clone)
    virtual void copyTo(zmm::Ref<CdsObject> obj);

    /// \brief Checks if current object is equal to obj.
    ///
    /// See description for CdsObject::equals() for details.
    virtual int equals(zmm::Ref<CdsObject> obj, bool exactly=false);

    /// \brief Checks if the minimum required parameters for the object have been set and are valid.
    virtual void validate();
};

/// \brief An Active Item in the content directory.
///
/// An active item is something very special, and it is not defined within UPnP.
/// From the UPnP point of view it is a normal Item, but internally it does
/// a little more.
/// When an ActiveItem is played back (HTTP GET request for it's URL), a
/// script is executed on the server.
/// The script gets an XML representation of the item (actually a DIDL-Lite render)
/// to the standard input, and has to return an appropriate XML to the standard
/// output. The script may change most of the values of the Item. The only
/// protected fields are object ID and parent ID. In case changes have taken
/// place, a container update is issued.
///
/// You could use ActiveItems for a variety of things, implementing
/// "toggle" items (ones that change between "on" and "off" with each activation)
/// or just "trigger" items that do not change visible but trigger events on the
/// server. For example, you could write a script and create an item to
/// shutdown your PC when this item is played.
///
/// We plan to extend the ActiveItem functionality, allowing to control
/// various server settings, and also being more flexible on the time
/// of script execution (execute script before or after serving the media, etc.)
class CdsActiveItem : public CdsItem
{
protected:
    /// \brief action to be executed (an absolute path to a script that will process the XML)
    zmm::String action;

    /// \brief a field where you can save any string you wnat.
    zmm::String state;
public:

    /// \brief Constructor, sets the object type.
    CdsActiveItem();

    /// \brief Sets the action for the item.
    /// \param action absolute path to the script that will process the XML data.
    inline void setAction(zmm::String action) { this->action = action; }

    /// \brief Get the path of the action script.
    inline zmm::String getAction() { return action; }

    /// \brief Set action state.
    /// \param state any string you want.
    ///
    /// This is quite useful to let the script identify what state the item is in.
    /// Think of it as a cookie (did I already mention that I hate web cookies?)
    inline void setState(zmm::String state) { this->state = state; }

    /// \brief Retrieve the item state.
    inline zmm::String getState() { return state; }

    /// \brief Copies all object properties to another object.
    /// \param obj target object (clone)
    virtual void copyTo(zmm::Ref<CdsObject> obj);

    /// \brief Checks if current object is equal to obj.
    ///
    /// See description for CdsObject::equals() for details.
    virtual int equals(zmm::Ref<CdsObject> obj, bool exactly=false);

    /// \brief Checks if the minimum required parameters for the object have been set and are valid.
    virtual void validate();
};

/// \brief An item that is accessible via a URL.
class CdsItemExternalURL : public CdsItem
{
public:

    /// \brief Constructor, sets the object type.
    CdsItemExternalURL();

    /// \brief Sets the URL for the item.
    /// \param URL full url to the item: http://somewhere.com/something.mpg
    inline void setURL(zmm::String URL) { this->location = URL; }

    /// \brief Get the URL of the item.
    inline zmm::String getURL() { return location; }
    /// \brief Copies all object properties to another object.
    /// \param obj target object (clone)
    //virtual void copyTo(zmm::Ref<CdsObject> obj);

    /// \brief Checks if current object is equal to obj.
    ///
    /// See description for CdsObject::equals() for details.
    //virtual int equals(zmm::Ref<CdsObject> obj, bool exactly=false);

    /// \brief Checks if the minimum required parameters for the object have been set and are valid.
    virtual void validate();
};

/// \brief An item that is pointing to a file located in the "servedir"
/// directory.
///
/// This implementation will alow to easily launch Java games on the
/// Streamium media renderer. Why "internal URL"? The port of the server
/// can change upon restarts, I have seen that the SDK often binds to 
/// a new port (no matter what is configured). The location of an
/// internal URL will be specified as /mystuff/myfile.txt and will
/// resolve to http://serverip:serverport/content/serve/mystuff/myfile.txt
class CdsItemInternalURL : public CdsItemExternalURL
{
public:

    /// \brief Constructor, sets the object type.
    CdsItemInternalURL();

    /// \brief Checks if the minimum required parameters for the object have been set and are valid.
    virtual void validate();
};

/*
/// \brief A Dynamic Item in the content directory.
///
/// A Dynamic Item differs from a regular Item in the way that
/// it's content is entirely dynamic and is being built on the fly
/// during request
class CdsDynamicItem : public CdsItem
{
protected:
    /// \brief action to be executed (an absolute path to a script that will process the XML)
    zmm::String ;

    /// \brief a field where you can save any string you wnat.
    zmm::String properties;
public:

    /// \brief Constructor, sets the object type.
    CdsActiveItem();

    /// \brief Sets the action for the item.
    /// \param action absolute path to the script that will process the XML data.
    void setAction(zmm::String action);

    /// \brief Get the path of the action script.
    zmm::String getAction();

    /// \brief Set action state.
    /// \param state any string you want.
    ///
    /// This is quite useful to let the script identify what state the item is in.
    /// Think of it as a cookie (did I already mention that I hate web cookies?)
    void setState(zmm::String state);

    /// \brief Retrieve the item state.
    zmm::String getState();

    /// \brief Copies all object properties to another object.
    /// \param obj target object (clone)
    virtual void copyTo(zmm::Ref<CdsObject> obj);

    /// \brief Checks if current object is equal to obj.
    ///
    /// See description for CdsObject::equals() for details.
    virtual int equals(zmm::Ref<CdsObject> obj, bool exactly=false);

    /// \brief Checks if the minimum required parameters for the object have been set and are valid.
    virtual void validate();
};

*/

/// \brief A container in the content directory.
class CdsContainer : public CdsObject
{
protected:
    /// \brief searchable flag.
    bool searchable;

    /// \brief container update id.
    int updateID;

    /// \brief childCount attribute
    int childCount;

public:
    /// \brief Constructor, initializes default values for the flags and sets the object type.
    CdsContainer();

    /// \brief Set the searchable flag.
    inline void setSearchable(bool searchable) { this->searchable = searchable; }

    /// \brief Query searchable flag.
    inline int isSearchable() { return searchable; }

    /// \brief Set the container update ID value.
    inline void setUpdateID(int updateID) { this->updateID = updateID; }

    /// \brief Query container update ID value.
    inline int getUpdateID() { return updateID; }

    /// \brief Set container childCount attribute.
    inline void setChildCount(int childCount) { this->childCount = childCount; }

    /// \briefe Retrieve number of children
    inline int getChildCount() { return childCount; }

    /// \brief Copies all object properties to another object.
    /// \param obj target object (clone)
    virtual void copyTo(zmm::Ref<CdsObject> obj);

    /// \brief Checks if current object is equal to obj.
    ///
    /// See description for CdsObject::equals() for details.
    virtual int equals(zmm::Ref<CdsObject> obj, bool exactly=false);

    /// \brief Checks if the minimum required parameters for the object have been set and are valid.
    virtual void validate();
};

#endif // __CDS_OBJECTS_H__

