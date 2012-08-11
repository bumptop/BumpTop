/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk1/tooltip.h
// Purpose:     wxToolTip class
// Author:      Robert Roebling
// Id:          $Id: tooltip.h 37065 2006-01-23 02:28:01Z MR $
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __GTKTOOLTIPH__
#define __GTKTOOLTIPH__

#include "wx/defs.h"
#include "wx/string.h"
#include "wx/object.h"

//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxToolTip;
class WXDLLIMPEXP_CORE wxWindow;

//-----------------------------------------------------------------------------
// wxToolTip
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxToolTip : public wxObject
{
public:
    // globally change the tooltip parameters
    static void Enable( bool flag );
    static void SetDelay( long msecs );

    wxToolTip( const wxString &tip );

    // get/set the tooltip text
    void SetTip( const wxString &tip );
    wxString GetTip() const { return m_text; }

    wxWindow *GetWindow() const { return m_window; }
    bool IsOk() const { return m_window != NULL; }

    // implementation
    void Apply( wxWindow *win );

private:
    wxString     m_text;
    wxWindow    *m_window;

    DECLARE_ABSTRACT_CLASS(wxToolTip)
};

#endif // __GTKTOOLTIPH__
