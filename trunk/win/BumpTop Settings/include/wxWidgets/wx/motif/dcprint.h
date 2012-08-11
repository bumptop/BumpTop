/////////////////////////////////////////////////////////////////////////////
// Name:        wx/motif/dcprint.h
// Purpose:     wxPrinterDC class
// Author:      Julian Smart
// Modified by:
// Created:     17/09/98
// RCS-ID:      $Id: dcprint.h 42522 2006-10-27 13:07:40Z JS $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DCPRINT_H_
#define _WX_DCPRINT_H_

#include "wx/dc.h"

class WXDLLEXPORT wxPrinterDC: public wxDC
{
public:
    DECLARE_CLASS(wxPrinterDC)

    // Create a printer DC
    wxPrinterDC(const wxString& driver, const wxString& device, const wxString& output, bool interactive = true, int orientation = wxPORTRAIT);

    wxRect GetPaperRect();

    virtual ~wxPrinterDC();
};

#endif
// _WX_DCPRINT_H_
