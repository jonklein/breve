/******************************************************************
 *  breveSNNS
 *  A fully-featured plugin of the SNNS kernel for breve.
 *
 *  Authors:
 *
 *  Raphael Crawford-Marks (rpc01@hampshire.edu)
 *  David Prude (drp01@hampshire.edu)
 *
 *  2003
 ******************************************************************/

#ifdef DMALLOC
#include "dmalloc.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//Include the Breve API headers
#include "slBrevePluginAPI.h"

//Include SNNS Kernel UI headers
#include "kernel/kr_typ.h"
#include "kernel/kr_ui.h"
#include "kernel/kernel.h"
#include "kernel/glob_typ.h"
#include "kernel/enzo_mem_typ.h"

/*
//Include unitStack
#include "hashtable/hashtable_cwc22.h"
#include "hashtable/hashtable_itr_cwc22.h"
*/

#ifndef __ENZO__
#define __ENZO__
#endif

#define MAX_NETWORKS 20

/*****************************************************************************/
struct key
{
  char *name;
};

memNet networks[MAX_NETWORKS];
struct key keys[MAX_NETWORKS];

/*****************************************************************************/

/********************************************
 *  Wrapper functions for the krui functions.
 ********************************************/

int snns_loadNet(stEval *args, stEval *result, void *instance);
int snns_saveNet(stEval *args, stEval *result, void *instance);
int snns_setInitialisationFunc(stEval *args, stEval *result, void *instance);
int snns_initializeNet(stEval *args, stEval *result, void *instance);
int snns_loadNewPatterns(stEval *args, stEval *result, void *instance);
int snns_saveNewPatterns(stEval *args, stEval *result, void *instance);
int snns_getUnitActivation(stEval *args, stEval *result, void *instance);
int snns_setUnitActivation(stEval *args, stEval *result, void *instance);
int snns_setPatternNo(stEval *args, stEval *result, void *instance);
int snns_showPattern(stEval *args, stEval *result, void *instance);
int snns_updateNet(stEval *args, stEval *result, void *instance);
int snns_getNoOfUnits(stEval *args, stEval *result, void *instance);
int snns_getFirstUnit(stEval *args, stEval *result, void *instance);
int snns_getNextUnit(stEval *args, stEval *result, void *instance);
int snns_setCurrentUnit(stEval *args, stEval *result, void *instance);
int snns_getCurrentUnit(stEval *args, stEval *result, void *instance);
int snns_getUnitName(stEval *args, stEval *result, void *instance);
int snns_setUnitName(stEval *args, stEval *result, void *instance);
int snns_searchUnitName(stEval *args, stEval *result, void *instance);
int snns_deleteNet(stEval *args, stEval *result, void *instance);
int snns_process(stEval *args, stEval *result, void *instance);
int snns_saveState(stEval *args, stEval *result, void *instance);
int snns_loadState(stEval *args, stEval *result, void *instance);

/*********************************************************
 *  breveSNNS - Entry-point function for breveSNNS plugin.
 * 
 *  Tells breve what new functions are available.
 *********************************************************/
int breveSNNSLoadFunctions(void *data) {
  return _breveSNNSLoadFunctions(data);
}

int _breveSNNSLoadFunctions(void *data) {

  initNetStorage();
  stNewSteveCall(data, "snns_loadNet", snns_loadNet, AT_STRING, AT_STRING, 0);
  stNewSteveCall(data, "snns_saveNet", snns_saveNet, AT_NULL, AT_STRING, AT_STRING, 0);
  stNewSteveCall(data, "snns_setInitialisationFunc", snns_setInitialisationFunc, AT_NULL, AT_STRING, 0);
  stNewSteveCall(data, "snns_initializeNet", snns_initializeNet, AT_NULL, AT_LIST, AT_INT, 0);
  stNewSteveCall(data, "snns_loadNewPatterns", snns_loadNewPatterns, AT_NULL, AT_STRING, AT_LIST, 0);
  stNewSteveCall(data, "snns_saveNewPatterns", snns_saveNewPatterns, AT_NULL, AT_STRING, AT_LIST, 0);
  stNewSteveCall(data, "snns_getUnitActivation", snns_getUnitActivation, AT_DOUBLE, AT_INT, 0);
  stNewSteveCall(data, "snns_setUnitActivation", snns_setUnitActivation, AT_NULL, AT_INT, AT_DOUBLE, 0);
  stNewSteveCall(data, "snns_setPatternNo", snns_setPatternNo, AT_NULL, AT_INT, 0);
  stNewSteveCall(data, "snns_showPattern", snns_showPattern, AT_NULL, AT_INT, 0); 
  stNewSteveCall(data, "snns_updateNet", snns_updateNet, AT_NULL, AT_LIST, AT_INT, 0);
  stNewSteveCall(data, "snns_getNoOfUnits", snns_getNoOfUnits, AT_INT, 0);
  stNewSteveCall(data, "snns_getFirstUnit", snns_getFirstUnit, AT_INT, 0);
  stNewSteveCall(data, "snns_getNextUnit", snns_getNextUnit, AT_INT, 0);
  stNewSteveCall(data, "snns_setCurrentUnit", snns_setCurrentUnit, AT_NULL, AT_INT, 0);
  stNewSteveCall(data, "snns_getCurrentUnit", snns_getCurrentUnit, AT_INT, 0);
  stNewSteveCall(data, "snns_getUnitName", snns_getUnitName, AT_STRING, AT_INT, 0);
  stNewSteveCall(data, "snns_setUnitName", snns_setUnitName, AT_NULL, AT_INT, AT_STRING, 0);
  stNewSteveCall(data, "snns_searchUnitName", snns_searchUnitName, AT_INT, AT_STRING, 0);
  stNewSteveCall(data, "snns_deleteNet", snns_deleteNet, AT_NULL, 0);
  stNewSteveCall(data, "snns_process", snns_process, AT_LIST, AT_LIST, AT_LIST, AT_INT, 0);
  stNewSteveCall(data, "snns_saveState", snns_saveState, AT_NULL, AT_STRING, 0);
  stNewSteveCall(data, "snns_loadState", snns_loadState, AT_NULL, AT_STRING, 0);
}

void initNetStorage() {
  int i;
  for(i=0;i<MAX_NETWORKS;i++) {
    (keys+i)->name = NULL;
  }
}

/**************************************************
 *  snns_loadNet
 * 
 *  Loads a network via the SNNS kernel UI function
 *  krui_loadNet().
 **************************************************/
int snns_loadNet(stEval *args, stEval *result, void *instance) {
  char *filename = STSTRING(&args[0]);
  char *netname, *netname1;
  krui_err errCode;
  char *errMsg;

  slMessage(0, "breveSNNS: in loadNet()");
  errCode = krui_loadNet(filename, &netname);
  slMessage(0, "breveSNNS: loadNet(): just finished calling krui_loadNet()");
  fflush(stderr);
  fflush(stdout);
  netname1 = (char *)slMalloc((strlen(netname)+1) * sizeof(char));
  strcpy(netname1, netname);
  if(errCode == KRERR_NO_ERROR) {
    STSTRING(result) = netname1;
    return EC_OK;
  }
  else {
    errMsg = krui_error(errCode);
    slMessage(0, "breveSNNS: loadNet failed with message %s (error code %d).", errMsg, errCode);
    return EC_ERROR;
  }
}

/**************************************************
 *  snns_saveNet
 * 
 *  Saves a network via the SNNS kernel UI function
 *  krui_saveNet().
 **************************************************/
int snns_saveNet(stEval *args, stEval *result, void *instance) {
  char *filename = STSTRING(&args[0]);
  char *netname = STSTRING(&args[1]);
  krui_err errCode;
  char *errMsg;

  errCode = krui_saveNet(filename, netname);
  if(errCode == KRERR_NO_ERROR) {
    return EC_OK;
  }
  else {
    errMsg = krui_error(errCode);
    slMessage(0, "breveSNNS: saveNet failed with message %s (error code %d).", errMsg, errCode);
    return EC_ERROR;
  }
}

/**************************************************
 *  snns_setInitialisationFunc
 * 
 *  Sets the network initialization function using
 *  krui_setInitialisationFunc().
 **************************************************/
int snns_setInitialisationFunc(stEval *args, stEval *result, void *instance) {
  char *initFuncName = STSTRING(&args[0]);
  krui_err errCode;
  char *errMsg;

  errCode = krui_setInitialisationFunc( initFuncName );
  if (errCode == KRERR_NO_ERROR) {
    return EC_OK;
  }
  else {
    errMsg = krui_error(errCode);
    slMessage(0, "breveSNNS: setInitialisationFunc failed with message %s (error code %d).", errMsg, errCode);
    return EC_ERROR;
  }
}

/**************************************************
 *  snns_initializeNet
 * 
 *  Initializes the network using 
 *  krui_initializeNet().
 **************************************************/
int snns_initializeNet(stEval *args, stEval *result, void *instance) {
  float *params = (float*)STLIST(&args[0]);
  int numParams = STINT(&args[1]);
  krui_err errCode;
  char *errMsg;

  errCode = krui_initializeNet(params, numParams);
  if (errCode == KRERR_NO_ERROR) {
    return EC_OK;
  }
  else {
    errMsg = krui_error(errCode);
    slMessage(0, "breveSNNS: initializeNet failed with message %s (error code %d).", errMsg, errCode);
    return EC_ERROR;
  }
}

/**************************************************
 *  snns_loadNewPatterns
 * 
 *  Loads patterns from a file using
 *  krui_loadNewPatterns().
 **************************************************/
int snns_loadNewPatterns(stEval *args, stEval *result, void *instance) {
  char *filename = STSTRING(&args[0]);
  int *set_no = (int*)STLIST(&args[1]);
  krui_err errCode;
  char *errMsg;

  errCode = krui_loadNewPatterns(filename, set_no);
  if (errCode == KRERR_NO_ERROR) {
    return EC_OK;
  }
  else {
    errMsg = krui_error(errCode);
    slMessage(0, "breveSNNS: loadNewPatterns failed with message %s (error code %d).", errMsg, errCode);
    return EC_ERROR;
  }
}

/**************************************************
 *  snns_saveNewPatterns
 * 
 *  Saves patterns from a file using
 *  krui_saveNewPatterns().
 **************************************************/
int snns_saveNewPatterns(stEval *args, stEval *result, void *instance) {
  char *filename = STSTRING(&args[0]);
  int set_no = STINT(&args[1]);
  krui_err errCode;
  char *errMsg;

  errCode = krui_saveNewPatterns(filename, set_no);
  if (errCode == KRERR_NO_ERROR) {
    return EC_OK;
  }
  else {
    errMsg = krui_error(errCode);
    slMessage(0, "breveSNNS: saveNewPatterns failed with message %s (error code %d).", errMsg, errCode);
    return EC_ERROR;
  }
}

/********************************************************************************
 *  FlintType krui_getUnitActivation( int UnitNo )
 *  krui_err krui_setUnitActivation( int UnitNo, FlintTypeParam unit_activation )
 *  
 *  Returns/sets the activation of the unit.
 ********************************************************************************/
int snns_getUnitActivation(stEval *args, stEval *result, void *instance) {
  int unitNo = STINT(&args[0]);
  FlintType unitAct;
  krui_err errCode;
  char *errMsg;

  unitAct = krui_getUnitActivation( unitNo );
  STFLOAT(result) = unitAct;
  return EC_OK;
}

int snns_setUnitActivation(stEval *args, stEval *result, void *instance) {
  int unitNo = STINT(&args[0]);
  FlintTypeParam unitActivation = STDOUBLE(&args[1]);
  krui_err errCode;
  char *errMsg;

  errCode = krui_setUnitActivation(unitNo, unitActivation);
  if (errCode == KRERR_NO_ERROR) {
    return EC_OK;
  }
  else {
    errMsg = krui_error(errCode);
    slMessage(0, "breveSNNS: setUnitActivation failed with message %s (error code %d).", errMsg, errCode);
    return EC_ERROR;
  }
}

/********************************************************************************
 * snns_setPatternNo 
 *
 * Sets the current pattern using 
 * krui_setPatternNo
 ********************************************************************************/
int snns_setPatternNo(stEval *args, stEval *result, void *instance) {
  int pattern_no = STINT(&args[1]);
  krui_err errCode;
  char *errMsg;
  
  errCode = krui_setPatternNo( pattern_no );
  if (errCode == KRERR_NO_ERROR) {
     return EC_OK;
  }
  else {
    errMsg = krui_error(errCode);
    slMessage(0, "breveSNNS: setPatternNo failed with message %s (error code %d).", errMsg, errCode);
    return EC_ERROR;
  }
}

/*******************************************************************************
 *  snns_showPattern
 *
 *  From the SNNS documentation:
 *    "According to the mode [snns_showPattern] stores the current
 *     pattern into the units activation (and/or output) values.
 *     The modes are:
 *       - OUTPUT_NOTHING
 *          store input pattern into input units activations
 *       - OUTPUT_ACT
 *          store input pattern into input units activations and
 *          store output pattern into output units activations
 *       - OUTPUT_OUT
 *          store input pattern into input units activations,
 *          store output pattern into output units activations and
 *          update output units output"
 *
 * i.e. shows a patten to the network using
 * krui_showPattern
 ********************************************************************************/
int snns_showPattern(stEval *args, stEval *result, void *instance) {
  int mode = STINT(&args[1]);
  krui_err errCode;
  char *errMsg;

  errCode = krui_showPattern( mode );
  if (errCode == KRERR_NO_ERROR) {
     return EC_OK;
  }
  else {
    errMsg = krui_error(errCode);
    slMessage(0, "breveSNNS: setPatternNo failed with message %s (error code %d).", errMsg, errCode);
    return EC_ERROR;
  }
}


/********************************************************************************
 *  snns_updateNet
 *
 *  updates the network according to update function using
 *  krui_updateNet
 ********************************************************************************/
int snns_updateNet(stEval *args, stEval *result, void *instance) {
  float *params = (float*)STLIST(&args[0]);
  int	numParams = STINT(&args[1]);
  krui_err errCode;
  char *errMsg;

  errCode = krui_updateNet( params, numParams );
  if (errCode == KRERR_NO_ERROR) {
     return EC_OK;
  }
  else {
    errMsg = krui_error(errCode);
    slMessage(0, "breveSNNS: updateNet failed with message %s (error code %d).", errMsg, errCode);
    return EC_ERROR;
  }
}

int snns_getNoOfUnits(stEval *args, stEval *result, void *instance) {
  int unitQuantity;

  unitQuantity = krui_getNoOfUnits();
  STINT(result) = unitQuantity;
  return EC_OK;
}

int snns_getFirstUnit(stEval *args, stEval *result, void *instance) {
  int firstUnit;

  firstUnit = krui_getFirstUnit();
  STINT(result) = firstUnit;
  return EC_OK;
}

int snns_getNextUnit(stEval *args, stEval *result, void *instance) {
  int nextUnit;

  nextUnit = krui_getNextUnit();
  STINT(result) = nextUnit;
  return EC_OK;
}

int snns_setCurrentUnit(stEval *args, stEval *result, void *instance) {
  int currUnit = STINT(&args[0]);
  krui_err errCode;
  char *errMsg;

  errCode = krui_setCurrentUnit( currUnit );
  if (errCode == KRERR_NO_ERROR) {
     return EC_OK;
  }
  else {
    errMsg = krui_error(errCode);
    slMessage(0, "breveSNNS: setCurrentUnit failed with message %s (error code %d).", errMsg, errCode);
    return EC_ERROR;
  }
}

int snns_getCurrentUnit(stEval *args, stEval *result, void *instance) {
  int currUnit;

  STINT(result) = krui_getCurrentUnit();
  return EC_OK;
}

int snns_getUnitName(stEval *args, stEval *result, void *instance) {
  char* unitName;
  int unitNo = STINT(&args[0]);

  STSTRING(result) = krui_getUnitName( unitNo );
  return EC_OK;
}

int snns_setUnitName(stEval *args, stEval *result, void *instance) {
  char* unitName = STSTRING(&args[1]);
  int unitNo = STINT(&args[0]);
  krui_err errCode;
  char* errMsg;

  errCode = krui_setUnitName( unitNo, unitName );
  if (errCode == KRERR_NO_ERROR) {
     return EC_OK;
  }
  else {
    errMsg = krui_error(errCode);
    slMessage(0, "breveSNNS: setUnitName failed with message %s (error code %d).", errMsg, errCode);
    return EC_ERROR;
  }
}

int snns_searchUnitName(stEval *args, stEval *result, void *instance) {
  char* searchName = STSTRING(&args[0]);
  int unitNo;

  unitNo = krui_searchUnitName( searchName );
  STINT(result) = unitNo;
}

int snns_deleteNet(stEval *args, stEval *result, void *instance){

	krui_deleteNet();

	return EC_OK;
}

/*******************************************************************************
 *  snns_process
 *
 *  takes values for input nodes as input and returns the output values
 *  from the network using krui_setPatternNo, krui_showPattern, and krui_update
 *******************************************************************************/

int snns_process(stEval *args, stEval *result, void *instance) {
  stEvalListHead *head = STLIST(&args[0]);
  float *params = (float*)STLIST(&args[1]);
  int numParams = STINT(&args[2]);
  FlintTypeParam *array_ptr;
  double *returnarray_ptr;
  double *iterator;
  int quant;
  int i;
  int n;
  int *n_ptr;
  krui_err errCode;
  char *errMsg;
  stEvalListHead *result_head = STLIST(result); 
  array_ptr = list_toArray(head);
  iterator = array_ptr;
  quant = krui_getNoOfUnits();
  
  i = 1;
  n = 0;
  
  for(i=1;i <= quant; i++){
	if(krui_getUnitTType(i)==INPUT){
		krui_setUnitActivation(i, *iterator);
		iterator++;
		n++;
	}
  }
  
  krui_updateNet(params, numParams);
  
 free(array_ptr);   //at the moment no space is allocated in list_toArray so commenting out this will result in a delay of the seg fault.
  
  i=0;
  n=0;
  
 
  for(i=1;i <= quant; i++){
	if(krui_getUnitTType(i)==OUTPUT){
		n++;
	}
  }

  returnarray_ptr = slMalloc(sizeof(double)*n);
  iterator = returnarray_ptr;
  i=1;
  
  
  for(i=1;i <= quant; i++){
	  if(krui_getUnitTType(i)==OUTPUT){
		  *iterator = krui_getUnitOutput(i);
		  //		  printf("Got output of %d\n", (*iterator));
		  *(iterator++);
	  }
  }
  
  
  result->values.listValue = array_toList(returnarray_ptr, n);
  result->type = AT_LIST;
  
  free(returnarray_ptr); 

  return EC_OK;

}	    

int getIndexByName(char *name) {
  int i;
  for(i=0;i<MAX_NETWORKS;i++) {
    if((keys+i)->name != NULL) {
      if(0 == strcmp((keys+i)->name, name))
	return i;
    }
  }
  return -1;
}

int getFirstOpenIndex() {
  int i;
  for(i=0;i<MAX_NETWORKS;i++) {
    if((keys+i)->name == NULL)
      return i;
  }
  return -1;
}

/*
simpleSaveState - Saves the state of the network using the ENZO
network management module and some hashtable library I found
on the Internet.
*/
int snns_saveState(stEval *args, stEval *results, void *instance) {
  int i;
  memNet tempnet;
  char *name = (char *)slMalloc(strlen((STSTRING(&args[0]))+1) * sizeof(char));
  name = strcpy(name, STSTRING(&args[0]));
  i = getIndexByName(name);
  if(i<0)
    i = getFirstOpenIndex();
  if((keys+i)->name != NULL)
    free((keys+i)->name);
  krm_getNet(&tempnet);
  networks[i] = tempnet;
  (keys+i)->name = name;
  return EC_OK;
}

int snns_loadState(stEval *args, stEval *results, void *instance) {
  int i;
  char *name = (char *)slMalloc(strlen((STSTRING(&args[0]))+1) * sizeof(char));
  name = strcpy(name, STSTRING(&args[0]));
  i = getIndexByName(name);
  if(i<0) {
    slMessage(0, "Could not find network with the identifier %s\n",name);
    return EC_ERROR;
  }
  krm_putNet(&networks[i]);
  return EC_OK;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------
 * INTERNAL FUNCTIONS
 *___________________________________________________________________________________________________________________________________________*/

/*********************************************************************
 * list_to_Array
 *
 * takes in a pointer to a stEval containing a pointer
 * to a stEvalListHead for a doubly linked list of stEvalList nodes
 * each containing an stEval which holds a double value and an array
 * pointer. It returns a pointer to an array of floats and an integer
 * which reperesents the total number of elements in the array that
 * was created.
 *********************************************************************/


FlintTypeParam* list_toArray(stEvalListHead *head){
	stEvalList *iterator;
	stEval *temp;
	FlintTypeParam *array_working_ptr;
	int n;
	int i;
	FlintTypeParam *array_ptr;
	FlintTypeParam testDouble;	
	iterator = head->start;
	n = head->count;
	array_ptr = malloc(sizeof(FlintTypeParam)*n);
	array_working_ptr = array_ptr;	
	i = 0;
	while(iterator != NULL){
		testDouble = (FlintTypeParam)STDOUBLE(&iterator->eval);
		*array_working_ptr++ = (FlintTypeParam)STDOUBLE(&iterator->eval);
		iterator = iterator->next;
		i++;
	}
	return array_ptr;
}


/************************************************************************
 * array_toList
 *
 * takes a pointer to an array, an integer value for the number of items
 * in the array, and a pointer to an stEval, and creates a doubly linked
 * list of stEval's containing the values in the array, and places the
 * pointer to the stEvalListHead into the stEval that was fed to it.
 ************************************************************************/



stEvalListHead* array_toList(double *array_ptr, int n){
	stEval myEval;
	double temp;
	stEval *myEval_ptr;
	int i;
	stEvalListHead *result_head;
	
	result_head = stNewEvalList();
	myEval_ptr = &myEval;
		
	i=0;
	for(i=0;i<n;i++){
		myEval.values.doubleValue = *(array_ptr++);
		myEval.type = AT_DOUBLE;
		stEvalListAppend(result_head, myEval_ptr);
	
	}
	return result_head;
}
