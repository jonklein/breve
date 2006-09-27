#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "SimInstance.h"
#endif

#include "wx/wxprec.h"  

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Main.h"
#include "BreveRender.h"
#include "BreveInterface.h"
#include "BreveCanvas.h"
#include "CodeWindow.h"
#include "steve.h"
#include "evaluation.h"
#include "expression.h"
#include "SimInstance.h"
#include "Inspector.h"

SimInstance::SimInstance(wxString simdir, wxString simfile, wxString text)
{
    this->otext = text;
    this->simdir = simdir;
    this->text = text;
    this->simulationfile = simfile;
    next = NULL;
    selected = NULL;
    iroot = NULL;
    binterface = new BreveInterface(strdup(simulationfile), simdir, strdup(text));
    codewindow = new CodeWindow(this, simfile, simdir, text);
}

SimInstance::~SimInstance()
{
    codewindow->Destroy();

    VarData * n, * d;

    for (d = iroot; d != NULL; d = n)
    {
	n = d->GetNext();

	delete d;
    }

    delete binterface;

    iroot = NULL;
}

void SimInstance::UpdateSimCode()
{
    text = codewindow->GetCode();

    binterface->UpdateCode(strdup(text));
}

void SimInstance::RegenSim() {
    int i = -1;

    if ( gBreverender->GetSimulation() == this ) {
	i = gBreverender->GetSimInt(this);
	gBreverender->ResetSim(-1);
    }
    
    delete binterface;

    binterface = new BreveInterface(strdup(simulationfile), simdir, strdup(text));

    if (i > -1)
	gBreverender->ResetSim(i);
}

void SimInstance::SetSelected(brInstance*i, int type)
{
    VarData * d;
    stInstance * instance = NULL;

    if (i != selected || selected == NULL || type == 1)
    {
	if (iroot != NULL)
	{
	    VarData * n;

	    for (d = iroot; d != NULL; d = n)
	    {
		n = d->GetNext();

		delete d;
	    }

	    iroot = NULL;
	}

	if (selected != i)
	   type = 0;
    }

    selected = i;

    if (selected == NULL && gBreverender->GetSimulation() != NULL && gBreverender->GetSimulation()->GetInterface()->Initialized())
	instance = (stInstance*)gBreverender->GetSimulation()->GetInterface()->GetFrontend()->engine->controller->userData;
    else if (selected != NULL)
	instance = (stInstance*)(selected->userData);

    if (instance != NULL)
    {
	stRunInstance ri;
	brEval eval;

	ri.instance = instance;
	ri.type = instance->type;

	iroot = ProcessObject(instance, instance->type, &ri, &eval);

	//if (iroot != NULL)
	//    tempfunc(iroot, 0);
    }

    if (gBreverender->GetInspector() != NULL)
	gBreverender->GetInspector()->SetRoot(iroot, type);
}

void SimInstance::tempfunc(VarData * d, int level)
{
    if (level > 10)
	return;

    for (; d != NULL; d = d->GetNext())
    {
	if (!d->IsSuper())
	{
	    printf("%*c%-10s - %s\n\r", level * 2, ' ', d->GetName().c_str(), d->String().c_str());

	    if (d->IsExpandable())
		tempfunc(ExpandChild(d, d->GetName().c_str()), level + 1);
	}
	else
	{
	    printf("%*c%-10s - %s\n\r", level * 2, ' ', "super", d->GetName().c_str());

	    tempfunc(ExpandChild(d, ""), level + 1);
	}
    }
}

wxString VarData::String()
{
    wxString str;

    switch ( eval.type() )
    {
	case AT_INT:
	    str << eval.getInt();
	    break;

	case AT_DOUBLE:
	    str << eval.getDouble();
	    break;

	case AT_STRING:
	    str << eval.getString();
	    break;

	case AT_POINTER:
	    str.Printf( "%p", eval.getPointer() );
	    break;

	case AT_INSTANCE:
	    if (eval.getInstance() == NULL)
		str << "(null)";
	    else
		str << ((stInstance*)(eval.getInstance()->userData))->type->name.c_str();
	    break;

	case AT_DATA:
	    str << "Data";
	    break;

	case AT_VECTOR:
	    str << "(" << eval.getVector().x << ", " <<
		eval.getVector().y << ", " <<
		eval.getVector().z << ")";
	    break;

	case AT_LIST:
	    str << "List (" << eval.getList()->_vector.size() << ")";
	    break;

	case AT_HASH:
	    str << "Hash (" << eval.getHash()->table->size << ")";
	    break;

	case AT_MATRIX:
	    {
		int i = 0;

		for (i = 0; i < 3; i++)
		{
		    str << "(" <<
			eval.getMatrix()[i][0] <<
			eval.getMatrix()[i][1] <<
			eval.getMatrix()[i][2] <<
			")";
		}

		break;
	    }

	case AT_ARRAY:
	    str << "Array";
	    break;

	default:
	    str << "?Unknown?";
	    break;
    }

    return str;
}

VarData * SimInstance::ExpandChild(VarData * dlist, wxString named)
{
    VarData * d;
    stRunInstance ri;
    brEval eval;

    for (d = dlist; d != NULL; d = d->GetNext())
    {
	if ((named.IsEmpty() && d->IsSuper()) || (!named.IsEmpty() && !d->IsSuper() && d->IsExpandable() && !named.Cmp(d->GetName())))
	{
	    if (d->GetChildren())
		return d->GetChildren();

	    if (d->IsSuper())
	    {
		ri.instance = d->GetInstance();
		ri.type = d->GetInstance()->type;

		d->SetChildren(ProcessObject(d->GetInstance(), d->GetObject()->super, &ri, &eval));
	    }
	    else if (d->GetEval()->type() == AT_INSTANCE)
	    {
		if (d->GetEval()->getInstance() == NULL)
		    return NULL;

		ri.instance = (stInstance*)d->GetEval()->getInstance()->userData;
		ri.type = ((stInstance*)(d->GetEval()->getInstance()->userData))->type;

		d->SetChildren(ProcessObject((stInstance*)d->GetEval()->getInstance()->userData, ((stInstance*)(d->GetEval()->getInstance()->userData))->type, &ri, &eval));
	    }
	    else if (d->GetEval()->type() == AT_LIST)
	    {
		d->SetChildren(ProcessList(d->GetInstance(), d->GetObject(), d->GetEval()->getList() ));
	    }
	    else if (d->GetEval()->type() == AT_ARRAY)
	    {
		d->SetChildren(ProcessArray(d, d->GetInstance(), d->GetObject(), &ri, &eval));
	    }
	    else
	    {
		wxMessageBox("Cannot expand value");
	    }

	    return d->GetChildren();
	}
    }
}

VarData * SimInstance::ProcessList(stInstance * instance, stObject * object, brEvalListHead * head)
{
    VarData * first = NULL;
    VarData * d = NULL;
    VarData * last = NULL;
    wxString str;
    int count = 0;

    if (head == NULL || head->_vector.size() < 1)
	return NULL;

	for( int c = 0; c < head->_vector.size(); c++ ) {
		str = "Item ";
		str << count++;

		d = new VarData( head->_vector[ c ], str, instance, object);

		if (first == NULL) first = d;

		if (last != NULL)
		    last->SetNext(d);

		last = d;
    }

    return first;
}

VarData * SimInstance::ProcessArray(VarData * top, stInstance * instance, stObject * object, stRunInstance *ri, brEval * eval)
{
    VarData * first = NULL;
    VarData * d = NULL;
    VarData * last = NULL;
    wxString str;
    int i = 0;
    int off;

    if (top->GetACount() < 1 || top->GetEval()->getInt() == AT_ARRAY)
	return NULL;

    off = top->GetOffset();

    for (i = 0; i < top->GetACount(); i++)
    {
	str = "Item ";
	str << i;

	mutex.Lock();
	stLoadVariable(&instance->variables[off], top->GetEval()->getInt(), eval, ri);
	mutex.Unlock();

	off += stSizeofAtomic( top->GetEval()->getInt() );

	d = new VarData(eval, str, instance, object);

	if (first == NULL)
	    first = d;

	if (last != NULL)
	    last->SetNext(d);

	last = d;
    }

    return first;
}

VarData * SimInstance::ProcessObject(stInstance * instance, stObject * object, stRunInstance*ri, brEval*eval)
{
    VarData * first = NULL;
    VarData * d = NULL;
    VarData * last = NULL;
    stVar * var;
	std::map< std::string, stVar* >::iterator vi;

    if ( (int)instance->status != (int)AS_ACTIVE ) return NULL;

	for ( vi = object->variables.begin(); vi != object->variables.end(); vi++ )
    {
	var = vi->second;

	if (var->type->_type != AT_ARRAY)
	{
	    mutex.Lock();
	    stLoadVariable(&instance->variables[var->offset], var->type->_type, eval, ri);
	    mutex.Unlock();
	}
	else
	{
	    // eval->set( var->type->arrayType );
	    // eval->setType( AT_ARRAY );
		eval->set( 0 );
	}

	d = new VarData( eval, var->name.c_str(), instance, object );

	if ( eval->type() == AT_ARRAY )
	{
	    d->SetOffset(var->offset);
	    d->SetACount(var->type->_arrayCount);
	}

	if (first == NULL)
	    first = d;

	if (last != NULL)
	    last->SetNext(d);

	last = d;
    }

    if (object->super != NULL)
    {
	d = new VarData( eval, object->super->name.c_str(), instance, object, 1 );

	if (first == NULL)
	    first = d;

	if (last != NULL)
	    last->SetNext(d);

	last = d;
    }

    return first;
}

VarData::VarData(brEval * e, wxString name, stInstance * instance, stObject * object, int issuper)
{
	brEvalCopy( e, &this->eval );
    this->name = name;
    next = NULL;
    children = NULL;
    this->issuper = issuper;
    this->instance = instance;
    this->object = object;

    switch ( eval.type() )
    {
	default:
	    expandable = 0;
	    break;

	//case AT_HASH:
	case AT_ARRAY:
	case AT_LIST:
	case AT_INSTANCE:
	    expandable = 1;
	    break;
    }

    if (issuper)
	expandable = 1;
}
