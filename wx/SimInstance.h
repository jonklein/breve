#ifndef _SIMINSTANCE_H_
#define _SIMINSTANCE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "SimInstance.cpp"
#endif

#include "BreveInterface.h"

class CodeWindow;

struct brInstance;
struct stInstance;
struct stObject;
struct stRunInstance;
struct brEval;

class VarData
{
    private:
	brEval eval;
	wxString name;
	VarData * next;
	int expandable;
	int issuper;
	int offset;
	int acount;
	VarData * children;
	stInstance * instance;
	stObject * object;

    public:
	VarData(brEval * e, wxString name, stInstance * instance, stObject * object, int issuper = 0);

	wxString String();

	~VarData()
	{
	    VarData * d;
	    VarData * n;

	    for (d = children; d != NULL; d = n)
	    {
		n = d->GetNext();
		delete d;
	    }
	}

	void SetACount(int a)
	{
	    acount = a;
	}

	int GetACount()
	{
	    return acount;
	}

	void SetOffset(int o)
	{
	    offset = o;
	}

	int GetOffset()
	{
	    return offset;
	}

	stObject * GetObject()
	{
	    return object;
	}

	stInstance * GetInstance()
	{
	    return instance;
	}

	int IsSuper()
	{
	    return issuper;
	}

	int IsExpandable()
	{
	    return expandable;
	}

	void SetChildren(VarData * c)
	{
	    children = c;
	}

	VarData * GetChildren()
	{
	    return children;
	}

	void SetNext(VarData * next)
	{
	    this->next = next;
	}

	VarData * GetNext()
	{
	    return next;
	}

	wxString GetName()
	{
	    return name;
	}

	brEval * GetEval()
	{
	    return &eval;
	}
};

class SimInstance
{
    public:
	SimInstance(wxString simdir, wxString simfile, wxString text);
	~SimInstance();

	void UpdateSimCode();
	void RegenSim();
	VarData * ExpandChild(VarData * dlist, wxString named);
	VarData * ProcessObject(stInstance * instance, stObject * object, stRunInstance *, brEval *);
	VarData * ProcessList(stInstance * instance, stObject * object, brEvalListHead * head);
	VarData * ProcessArray(VarData *, stInstance * instance, stObject * object, stRunInstance *ri, brEval *eval);
	//VarData * HandleVar(int type, int arraytype, int arraycount,int offset,
	//    const char * name, stInstance * instance, stRunInstance * ri, brEval * eval);
	    
	void SetSelected(brInstance * selected, int type = 0);
	void tempfunc(VarData *, int level);

	wxMutex * GetMutex()
	{
	    return &mutex;
	}

	SimInstance * GetNext()
	{
	    return next;
	}

	void Rechain(SimInstance * n)
	{
	    next = n;
	}

	BreveInterface * GetInterface()
	{
	    if (!binterface->IsValid())
		RegenSim();

	    return binterface;
	}

	wxString GetOriginCode()
	{
	    return otext;
	}

	wxString GetFilename()
	{
	    return simulationfile;
	}

	wxString GetDir()
	{
	    return simdir;
	}

	void UpdateFile(wxString file, wxString dir, wxString text)
	{
	    this->otext = text;
	    this->simdir = dir;
	    this->simulationfile = file;

	    gBreverender->UpdateChoice(gBreverender->GetSimInt(this));
	}

	brInstance * GetSelected()
	{
	    return selected;
	}

    private:
	SimInstance * next;
	BreveInterface * binterface;
	CodeWindow * codewindow;
	wxString text;
	wxString otext;
	wxString simdir;
	wxString simulationfile;
	brInstance * selected;
	VarData * iroot;
	wxMutex mutex;
};

#endif
