/////////////////////////////////////////////////////////////////////////////
// Name:        Inspector.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     Mon 14 Mar 2005 07:44:08 PM EST
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "Inspector.h'"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Main.h"
#include "steve.h"
#include "SimInstance.h"
#include "Inspector.h"

#include <wx/confbase.h>

#include "images/expanded.xpm"
#include "images/play.xpm"

IMPLEMENT_CLASS( Inspector, wxFrame )

BEGIN_EVENT_TABLE( Inspector, wxFrame )
    EVT_CLOSE(Inspector::OnClose)
    EVT_LIST_ITEM_SELECTED( ID_INSPECTORCTRL, Inspector::OnInspectorctrlSelected )
    EVT_LIST_ITEM_ACTIVATED( ID_INSPECTORCTRL, Inspector::OnInspectorctrlItemActivated )
    EVT_BUTTON( ID_UPDATE, Inspector::OnUpdateClick )
    EVT_MOVE(Inspector::OnMove)
    EVT_SIZE(Inspector::OnSize)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE( ResizingListCtrl, wxListCtrl )
    EVT_SIZE(ResizingListCtrl::OnSize)
END_EVENT_TABLE()

ResizingListCtrl::ResizingListCtrl(wxWindow * parent, int id, wxPoint p, wxSize s, long st) :
    wxListCtrl(parent, id, p, s, st)
{
}

void ResizingListCtrl::OnSize(wxSizeEvent &event)
{
    int w, h;

    event.Skip();

    GetSize(&w, &h);

    if (GetColumnWidth(0) + GetColumnWidth(1) != w)
	SetColumnWidth(0, w - GetColumnWidth(1));
}

Inspector::Inspector( )
{
}

Inspector::Inspector( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create( parent, id, caption, pos, size, style );
}

bool Inspector::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    bool loaded = FALSE;
    wxSize tsize;
    wxPoint tpoint;

    InspectorCtrl = NULL;
    UpdateButton = NULL;

    tsize = size;
    tpoint = pos;

    wxConfigBase * config = wxConfigBase::Get();

    if (config != NULL)
    {
	int x, y, w, h;

	if (config->Read("InspectorX", &x) && config->Read("InspectorY", &y) &&
	    config->Read("InspectorWidth", &w) && config->Read("InspectorHeight", &h))
	{
	    tpoint.x = x;
	    tpoint.y = y;
	    tsize.SetHeight(h);
	    tsize.SetWidth(w);
	    loaded = TRUE;
	}
    }

    wxFrame::Create( parent, id, caption, tpoint, tsize, style );

#ifdef __WXMSW__
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
#endif

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetRoot(NULL);

    if (loaded)
    {
	SetSize(tsize);
	Move(tpoint);
    }

    Show(TRUE);

    return TRUE;
}

void Inspector::CreateControls()
{    
    Inspector* itemFrame1 = this;

    imagelist = new wxImageList(10, 10, TRUE, 2);

    expandbitmap = wxBitmap(play_xpm);
    expandedbitmap = wxBitmap(expanded_xpm);

    empty.Create(10, 10);

    {
	wxMemoryDC dc;
	wxMask * mask;

	dc.SelectObject(empty);

	dc.SetBackground(*wxWHITE_BRUSH);

	dc.Clear();

	mask = new wxMask(empty, *wxWHITE);

	empty.SetMask(mask);
    }

    imagelist->Add(empty);
    imagelist->Add(expandbitmap);
    imagelist->Add(expandedbitmap);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemFrame1->SetSizer(itemBoxSizer2);

    InspectorCtrl = new ResizingListCtrl( this, ID_INSPECTORCTRL, wxDefaultPosition, wxSize(250, 250), wxLC_REPORT|wxLC_SINGLE_SEL|wxSUNKEN_BORDER);
    itemBoxSizer2->Add(InspectorCtrl, 1, wxGROW|wxALL, 5);

    InspectorCtrl->SetImageList(imagelist, wxIMAGE_LIST_SMALL);

    InspectorCtrl->InsertColumn(0, "Variable");
    InspectorCtrl->InsertColumn(1, "Data");

    UpdateButton = new wxButton( itemFrame1, ID_UPDATE, _("Update"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(UpdateButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
}

void Inspector::Redraw(VarData * root, int loc, int depth)
{
    wxString str;
    VarData * d = root;
    int i = 0;

    for (i = loc; ; i++)
    {
	if (d == NULL)
	    break;

	if (depth > 0)
	    str.Printf("%*c", 3 * depth, ' ');
	else
	    str = "";

	if (!d->IsSuper())
	{
	    str << d->GetName();
	    InspectorCtrl->InsertItem(i, str);
	    InspectorCtrl->SetItem(i, 1, d->String());
	}
	else
	{
	    str << "super";
	    InspectorCtrl->InsertItem(i, str);
	    InspectorCtrl->SetItem(i, 1, d->GetName());
	}

	InspectorCtrl->SetItemData(i, (long)d);

	if (d->IsExpandable())
	    InspectorCtrl->SetItemImage(i, 1, 1);
	else
	    InspectorCtrl->SetItemImage(i, 0, 0);

	d = d->GetNext();
    }

    d = root;

    for (d = root; d != NULL; d = d->GetNext())
    {
	if (d->IsExpandable() && expdata.ShouldExpand(d))
	{
#ifdef __WXMSW__
	    i = InspectorCtrl->FindItem(-1, (long)d);
#else
	    i = InspectorCtrl->FindItem(loc, (long)d);
#endif
	    InspectorCtrl->SetItemImage(i, 2, 2);

	    Redraw(d->GetChildren(), i + 1, depth + 1);
	}
    }
}

void Inspector::Reconstruct(int type)
{
    VarData * d;
    int i = 0;

    InspectorCtrl->DeleteAllItems();

    if (type == 1)
	expdata.RewalkInit(iroot);
    else
	expdata.Walk(iroot);

    lroot = iroot;

    InspectorCtrl->Freeze();
    Redraw(iroot, 0, 0);
    InspectorCtrl->Thaw();
}

bool Inspector::ShowToolTips()
{
    return TRUE;
}

wxBitmap Inspector::GetBitmapResource( const wxString& name )
{
    return wxNullBitmap;
}

wxIcon Inspector::GetIconResource( const wxString& name )
{
    return wxNullIcon;
}

void Inspector::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto())
    {
	Hide();
	return;
    }

    gBreverender->KillInspector();
    event.Skip();
}

void Inspector::OnUpdateClick( wxCommandEvent& event )
{
    if (gBreverender->GetSimulation() != NULL)
	gBreverender->GetSimulation()->SetSelected(gBreverender->GetSimulation()->GetSelected(),1);
}

void Inspector::OnInspectorctrlSelected( wxListEvent& event )
{
    VarData * d;
    int i = 0;

    d = (VarData*)event.GetData();

    if (d == NULL || !d->IsExpandable())
	return;

    InspectorCtrl->Freeze();

    InspectorCtrl->SetItemState(event.GetIndex(), 0, wxLIST_STATE_SELECTED);

    if (event.GetImage() == 1)
    {
	expdata.Expand(d);
	InspectorCtrl->SetItemImage(event.GetIndex(), 2, 2);

	const wxChar * str =  event.GetText();
	int depth = 0;

	while (str[0] == ' ')
	{
	    str += 3;
	    depth++;
	}

	Redraw(d->GetChildren(), event.GetIndex() + 1, depth + 1);
    }
    else
    {
	const wxChar * str;

	str = event.GetText();
	int nd, depth = 0;

	while (str[0] == ' ')
	{
	    str += 3;
	    depth++;
	}

	expdata.Collapse(d);
	InspectorCtrl->SetItemImage(event.GetIndex(), 1, 1);

	for (i = event.GetIndex() + 1; i < InspectorCtrl->GetItemCount(); )
	{
	    nd = 0;

	    str = InspectorCtrl->GetItemText(i);

	    while (str[0] == ' ')
	    {
		str += 3;
		nd++;
	    }

	    if (nd > depth)
		InspectorCtrl->DeleteItem(i);
	    else
		break;
	}
    }

    InspectorCtrl->Thaw();
}

void Inspector::OnInspectorctrlItemActivated( wxListEvent& event )
{
    /*VarData * d;
    int i = 0;

    d = (VarData*)event.GetData();

    if (d == NULL || !d->IsExpandable())
	return;

    if (event.GetImage() == 1)
	expdata.Expand(d);
    else
	expdata.Collapse(d);

    {
	const wxChar * str =  event.GetText();
	int depth = 0;

	while (str[0] == ' ')
	{
	    str += 3;
	    depth++;
	}

	InspectorCtrl->Freeze();
	Redraw(d->GetChildren(), event.GetIndex() + 1, depth + 1);
	InspectorCtrl->Thaw();
    }*/
}

void ExpData::RewalkInit(VarData * root)
{
    VarData * d;
    ExpItem * olist, *scan, *i, *last = NULL;

    if (list == NULL)
    {
	Walk(root);
	return;
    }

    olist = list;

    list = NULL;

    scan = olist;

    for (d = root; d != NULL; d = d->GetNext())
    {
	i = new ExpItem(d);

	if (list == NULL)
	    list = i;

	if (last != NULL)
	    last->SetNext(i);

	last = i;

	if (scan != NULL && scan->IsExpanded() && i->GetD()->IsExpandable())
	{
	    Expand(d);

	    Rewalk(scan->GetChildren(), i->GetChildren());
	}

	if (scan != NULL)
	    scan = scan->GetNext();
    }

    delete olist;
};

void ExpData::Rewalk(ExpItem * old, ExpItem *list)
{
    VarData * d;
    ExpItem * i, *last = NULL;
    ExpItem * scan;

    scan = old;

    for (i = list; i != NULL; i = i->GetNext())
    {
	if (scan != NULL && scan->IsExpanded() && i->GetD()->IsExpandable())
	{
	    Expand(i->GetD());

	    Rewalk(scan->GetChildren(), i->GetChildren());
	}

	if (scan != NULL)
	    scan = scan->GetNext();
    }
}

void ExpData::Walk(VarData *root)
{
    VarData * d;
    ExpItem * i, *last = NULL;

    if (list != NULL)
    {
	delete list;
	list = NULL;
    }

    for (d = root; d != NULL; d = d->GetNext())
    {
	i = new ExpItem(d);

	if (list == NULL)
	    list = i;

	if (last != NULL)
	    last->SetNext(i);

	last = i;
    }
}

ExpItem * ExpData::FindItem(ExpItem * root, VarData * loc)
{
    ExpItem * s, *n;

    for (s = root; s != NULL; s = s->GetNext())
    {
	if (s->GetD() == loc)
	    return s;

	n = FindItem(s->GetChildren(), loc);

	if (n)
	    return n;
    }

    return NULL;
}

void ExpData::Expand(VarData * loc)
{
    ExpItem * s, *n, *last = NULL, *first = NULL;
    VarData * d;

    s = FindItem(list, loc);

    if (s == NULL || s->IsExpanded())
	return;

    s->SetExpanded(1);

    if (loc->IsSuper())
	gBreverender->GetSimulation()->ExpandChild(loc, "");
    else
	gBreverender->GetSimulation()->ExpandChild(loc, loc->GetName().c_str());

    for (d = loc->GetChildren(); d != NULL; d = d->GetNext())
    {
	n = new ExpItem(d);

	if (first == NULL)
	{
	    first = n;
	    s->SetChildren(n);
	}

	if (last != NULL)
	    last->SetNext(n);

	last = n;
    }
}

void ExpData::Collapse(VarData * loc)
{
    ExpItem * s, *n, *last = NULL, *first = NULL;
    VarData * d;

    s = FindItem(list, loc);

    if (s == NULL || !s->IsExpanded())
	return;

    s->SetExpanded(0);

    if (s->GetChildren())
	delete s->GetChildren();

    s->SetChildren(NULL);
}

bool ExpData::ShouldExpand(VarData * loc)
{
    ExpItem * s;

    s = FindItem(list, loc);

    if (s == NULL)
	return FALSE;

    return s->IsExpanded();
}

void Inspector::OnSize(wxSizeEvent &event)
{
    if (!IsMaximized())
    {
	wxConfigBase * config = wxConfigBase::Get();

	if (config != NULL)
	{
	    config->Write("InspectorWidth", event.GetSize().GetWidth());
	    config->Write("InspectorHeight", event.GetSize().GetHeight());
	}
    }

    event.Skip();
}

void Inspector::OnMove(wxMoveEvent &event)
{
    {
	wxConfigBase * config = wxConfigBase::Get();

	if (config != NULL)
	{
	    config->Write("InspectorX", event.GetPosition().x);
	    config->Write("InspectorY", event.GetPosition().y);
	}
    }

    event.Skip();
}

