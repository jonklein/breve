#ifndef _INSPECTOR_H_
#define _INSPECTOR_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "Inspector.cpp"
#endif

#include "wx/frame.h"
#include "wx/listctrl.h"

class wxListCtrl;
class VarData;

#define ID_I_FRAME 10000
#define SYMBOL_INSPECTOR_STYLE wxDEFAULT_FRAME_STYLE|wxCLIP_CHILDREN 
#define SYMBOL_INSPECTOR_TITLE _("Object Inspector")
#define SYMBOL_INSPECTOR_IDNAME ID_I_FRAME
#define SYMBOL_INSPECTOR_SIZE wxSize(400, 300)
#define SYMBOL_INSPECTOR_POSITION wxDefaultPosition
#define ID_INSPECTORCTRL 10001
#define ID_UPDATE 10002

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

#include <wx/imaglist.h>

class ExpItem
{
    private:
	ExpItem * next, * children;
	VarData * d;
	int shouldexpand;

    public:
	ExpItem(VarData * d)
	{
	    next = children = NULL;
	    shouldexpand = 0;
	    this->d = d;
	}

	~ExpItem()
	{
	    ExpItem * d,*n;

	    for (d = children; d != NULL; d = n)
	    {
		n = d->GetNext();

		delete d;
	    }
	}

	ExpItem * GetNext()
	{
	    return next;
	}

	void SetNext(ExpItem * n)
	{
	    next = n;
	}

	void SetChildren(ExpItem *c)
	{
	    children = c;
	}

	ExpItem * GetChildren()
	{
	    return children;
	}

	void SetExpanded(int i)
	{
	    if (d->IsExpandable())
		shouldexpand = i;
	}

	int IsExpanded()
	{
	    return shouldexpand;
	}

	VarData * GetD()
	{
	    return d;
	}
};

class ExpData
{
    private:
	ExpItem * list;

	ExpItem * FindItem(ExpItem * root, VarData * loc);

    public:
	ExpData()
	{
	    list = NULL;
	}

	~ExpData()
	{
	    if (list != NULL)
		delete list;
	}

	void Walk(VarData * root);
	void Expand(VarData * loc);
	void Collapse(VarData * loc);
	bool ShouldExpand(VarData * loc);
	void RewalkInit(VarData * root);
	void Rewalk(ExpItem * old, ExpItem *list);
};

class ResizingListCtrl : public wxListCtrl
{
   DECLARE_EVENT_TABLE()

public:
    ResizingListCtrl(wxWindow *, int, wxPoint, wxSize, long);

    ~ResizingListCtrl()
    {
    }

    void OnSize(wxSizeEvent &event);
};

class Inspector: public wxFrame
{    
    DECLARE_CLASS( Inspector )
    DECLARE_EVENT_TABLE()

private:
    wxBitmap expandbitmap;
    wxBitmap expandedbitmap;
    wxBitmap empty;
    wxImageList * imagelist;
    ResizingListCtrl* InspectorCtrl;
    wxButton* UpdateButton;
    VarData * iroot;
    VarData * lroot;
    ExpData expdata;

    void Reconstruct(int type = 0);
    void Redraw(VarData * root, int loc, int depth);

public:
    Inspector( );
    Inspector( wxWindow* parent, wxWindowID id = SYMBOL_INSPECTOR_IDNAME, const wxString& caption = SYMBOL_INSPECTOR_TITLE, const wxPoint& pos = SYMBOL_INSPECTOR_POSITION, const wxSize& size = SYMBOL_INSPECTOR_SIZE, long style = SYMBOL_INSPECTOR_STYLE );
    ~Inspector()
    {
	if (imagelist != NULL)
	    delete imagelist;
    }

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_INSPECTOR_IDNAME, const wxString& caption = SYMBOL_INSPECTOR_TITLE, const wxPoint& pos = SYMBOL_INSPECTOR_POSITION, const wxSize& size = SYMBOL_INSPECTOR_SIZE, long style = SYMBOL_INSPECTOR_STYLE );
    void CreateControls();

    void SetRoot(VarData *r, int type = 0)
    {
	iroot = r;
	Reconstruct(type);
    }

    void OnInspectorctrlSelected( wxListEvent& event );
    void OnInspectorctrlItemActivated( wxListEvent& event );
    void OnUpdateClick( wxCommandEvent& event );
    void OnClose(wxCloseEvent &event);
    void OnSize(wxSizeEvent &event);
    void OnMove(wxMoveEvent &event);

    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
};

#endif
    // _INSPECTOR_H_

