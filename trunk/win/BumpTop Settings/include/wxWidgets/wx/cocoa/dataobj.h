///////////////////////////////////////////////////////////////////////////////
// Name:        wx/cocoa/dataobj.h
// Purpose:     declaration of the wxDataObject
// Author:      David Elliott <dfe@cox.net>
// Modified by:
// Created:     2003/07/23
// RCS-ID:      $Id: dataobj.h 27408 2004-05-23 20:53:33Z JS $
// Copyright:   (c) 2003 David Elliott
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __WX_COCOA_DATAOBJ_H__
#define __WX_COCOA_DATAOBJ_H__

class wxDataObject : public wxDataObjectBase
{
public:
    wxDataObject();
    virtual ~wxDataObject();
    virtual bool IsSupportedFormat(const wxDataFormat& format,
        Direction dir = Get) const;
};

#endif // __WX_COCOA_DATAOBJ_H__
