/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

/* 
    + steveFuncsDataObject.c
    = defines various internal functions relating the "Data" object.
    =
    = functions in this file must be implemented to read arguments from an
    = an array of brEvals and "return" values by filling in the brEval
    = result pointer.
    =
    = functions should return (the real C return) EC_OK or EC_ERROR depending
    = on whether an error occured.  EC_ERROR will kill the simulation, 
    = so on a non fatal error it is preferable to simply print an error
    = message.
    =
    = see the file internal.c for more information on the implementation 
    = of internal methods.
*/ 

#include "steve.h"
#include "xml.h"
#include "breveFunctionsSteveDataObject.h"
    
/* check the variables of this object to make sure that it contains */
/* only types valid for the Data object */

int stDDataCheckVariables(brEval *args, brEval *result, brInstance *bi) {
    stVar *v;
	int type = 0;
	stObject *o;
	std::map< std::string, stVar* >::iterator vi;

	stInstance *i = (stInstance*)bi->userData;

	o = i->type;

	// check all subinstances of i for the proper types

	while(o && strcmp(o->name, "Data")) {
		for(vi = o->variables.begin(); vi != o->variables.end(); vi++ ) {
			v = vi->second;

			if(v->type->type == AT_ARRAY) type = v->type->arrayType;
			else type = v->type->type;

        	if(type == AT_INSTANCE || type == AT_POINTER) {
				slMessage(DEBUG_ALL, "variable \"%s\" is not allowed in Data subclass class \"%s\"\n", v->name, i->type->name);
				result->set( -1 );
				return EC_OK;
			}
		}

		o = o->super;
	}

    result->set( 0 );

    return EC_OK;
}

int stDDataCopyObject(brEval *args, brEval *result, brInstance *bi) {
	stInstance *i = (stInstance*)bi->userData;
    stInstance *otherObject = (stInstance*)BRINSTANCE(&args[0])->userData;

	if(!otherObject) {
        result->set( -1 );
        slMessage(DEBUG_ALL, "cannot copy instance of type \"%s\" to uninitialized instance\n", i->type->name, otherObject->type->name);
        return EC_OK;
	}

    if(otherObject->type != i->type) {
        result->set( -1 );
        slMessage(DEBUG_ALL, "cannot copy instance of type \"%s\" to instance of class \"%s\"\n", i->type->name, otherObject->type->name);
        return EC_OK;
    }

	brDataCopyObject(i, otherObject);

    result->set( 0 );

    return EC_OK;
}

int stDDataWriteObject(brEval *args, brEval *result, brInstance *bi) {
    char *filename = BRSTRING(&args[0]);
	char *path;
	stInstance *i = (stInstance*)bi->userData;

    if(!filename) {
        slMessage(DEBUG_ALL, "NULL string passed to dataWriteObject.  Write cancelled.\n");
        result->set( -1 );
        return EC_OK;
    }

	path = brOutputPath(i->type->engine, filename);	
	result->set( stWriteObject(i, path) );
	slFree(path);

    return EC_OK;
}


int stWriteObject(stInstance *i, char *filename) {
    FILE *fp;
    int result, length;
    char *package;

    if(!(fp = fopen(filename, "w"))) {
        slMessage(DEBUG_ALL, "error opening file \"%s\" for writing\n", filename);
        return -1;
    }

    slMessage(DEBUG_ALL, "saving instance of %s to \"%s\"\n", i->type->name, filename);

    package = stPackObject(i, &length);
    result = slUtilFwrite(package, 1, length, fp);
    delete package;

    fclose(fp);

    return result;
}

char *stPackObject(stInstance *i, int *length) {
    brDataObjectHeader *header;
    char *package;

    if(!i) return NULL;

    *length = sizeof(brDataObjectHeader) + (i->type->varSize - i->type->varOffset) + strlen(i->type->name);

    package = new char[*length];
    header = (brDataObjectHeader*)&package[0];
  
    header->varSize = i->type->varSize - i->type->varOffset;
    header->nameLength = strlen(i->type->name);
    header->magic = SDO_MAGIC_NUMBER;
    header->version = SDO_VERSION;

    strcpy(&package[sizeof(brDataObjectHeader)], i->type->name);
    memcpy(&package[sizeof(brDataObjectHeader) + header->nameLength], &i->variables[i->type->varOffset], header->varSize);

    return package;
}

int stUnpackObject(stInstance *i, char *buffer, int length) {
    brDataObjectHeader *header;
    char name[1024];

    header = (brDataObjectHeader*)&buffer[0];

    if(header->magic != SDO_MAGIC_NUMBER) {
        slMessage(DEBUG_ALL, "load failed: corrupt class input file -- invalid identification number\n");
        return -1;
    }

    if(header->version != SDO_VERSION) {
        slMessage(DEBUG_ALL, "load failed: class data file saved with incompatible format\n");
        return -1;
    }

    if(header->varSize != i->type->varSize - i->type->varOffset) {
        slMessage(DEBUG_ALL, "load failed: data size mismatch between saved object and target object\n");
        return -1;
    }

    memcpy(&name[0], &buffer[sizeof(brDataObjectHeader)], header->nameLength);

    name[header->nameLength] = 0;

    if(strcmp(name, i->type->name)) {
        slMessage(DEBUG_ALL, "load failed: cannot load data from class \"%s\" into class \"%s\"\n", name, i->type->name);
        return -1;
    }

    memcpy(&i->variables[i->type->varOffset],
	&buffer[header->nameLength + sizeof(brDataObjectHeader)],
	header->varSize);

    return 0;
}

int stDDataWriteObjectWithDialog(brEval *args, brEval *result, brInstance *bi) {
    char *filename = NULL;
	stInstance *i = (stInstance*)bi->userData;
    
    result->set( 1 );
        
	filename = brEngineRunSaveDialog(i->type->engine);

    if(!filename) {  
        slMessage(DEBUG_ALL, "Could not retreive filename from user dialog: write cancelled.\n");
        return EC_OK;
    }
    
	result->set( stXMLWriteObjectToFile(i, filename, 1) );
    
    slFree(filename);

    return EC_OK;
}

int stDDataReadXMLObject(brEval args[], brEval *target, brInstance *bi) {
	stInstance *i = (stInstance*)bi->userData;

    char *filename = brFindFile(bi->engine, BRSTRING(&args[0]), NULL);

    target->set( stXMLReadObjectFromFile(i, filename) );

    return EC_OK;
}

int stDWriteXMLObject(brEval args[], brEval *target, brInstance *bi) {
    char *filename = BRSTRING(&args[0]);
    char *path;
	stInstance *i = (stInstance*)bi->userData;

	path = brOutputPath(i->type->engine, filename);
   	target->set( stXMLWriteObjectToFile(i, path, 1) );
	slFree(path);

    return EC_OK;
}

int stDDataReadObject(brEval *args, brEval *result, brInstance *bi) {
    char *filename = BRSTRING(&args[0]);
	stInstance *i = (stInstance*)bi->userData;

    if(!filename) {
        slMessage(DEBUG_ALL, "NULL string passed to dataReadObject.  Read cancelled.\n");
        result->set( -1 );
        return EC_OK;
    }

	result->set( stReadObject(i, filename) );

	return EC_OK;
}

int stDDataReadObjectWithDialog(brEval *args, brEval *result, brInstance *bi) {
    char *filename = NULL;
	stInstance *i = (stInstance*)bi->userData;

    result->set( 1 );

	filename = brEngineRunLoadDialog(i->type->engine);

    if(!filename) {
        slMessage(DEBUG_ALL, "Could not retrieve filename from load dialog: load cancelled.\n");
        return EC_OK;
    }

    result->set( stXMLReadObjectFromFile(i, filename) );

    if(BRINT(result) < 0) {
        slMessage(DEBUG_ALL, "load failed for file \"%s\"\n", filename);
    }

    slFree(filename);

    return EC_OK;
}

int stReadObject(stInstance *i, char *filename) {
    FILE *fp;
    int result;
    int length;
    char *package;
    char *wholeFile;
    
    brDataObjectHeader header;

    wholeFile = brFindFile(i->type->engine, filename, NULL);

    if(!(fp = fopen(wholeFile, "r"))) {
        slMessage(DEBUG_ALL, "error opening file \"%s\" for reading\n", filename);
        return -1;
    }

    slFree(wholeFile);

    result = slUtilFread(&header, sizeof(brDataObjectHeader), 1, fp);

    if(header.magic != SDO_MAGIC_NUMBER) {
        slMessage(DEBUG_ALL, "load failed: corrupt class input file -- invalid identification number\n");
        return -1;
    }

    length = sizeof(brDataObjectHeader) + header.nameLength + header.varSize;

    package = new char[length];
    memcpy(package, &header, sizeof(brDataObjectHeader));
    
    result = slUtilFread(&package[sizeof(brDataObjectHeader)], 1, header.nameLength + header.varSize, fp);

    fclose(fp);

    result = stUnpackObject(i, package, length);

    delete[] package;

    return result;
}

int stCSimpleCrossover(brEval *args, brEval *target, brInstance *i) {
    stInstance *a = (stInstance*)BRINSTANCE(&args[0])->userData;
    stInstance *b = (stInstance*)BRINSTANCE(&args[1])->userData;
    stInstance *child = (stInstance*)BRINSTANCE(&args[2])->userData;
    
    if(!a || !b) {
        slMessage(DEBUG_ALL, "uninitialized object passed to simpleCrossover\n");
        return EC_ERROR;
    }
    
    if(stObjectSimpleCrossover(a, b, child)) return EC_ERROR;

    return EC_OK;
}   

void breveInitSteveDataObjectFuncs(brNamespace *n) {
    brNewBreveCall(n, "dataCheckVariables", stDDataCheckVariables, AT_INT, 0);
    brNewBreveCall(n, "dataCopyObject", stDDataCopyObject, AT_INT, AT_INSTANCE, 0);
    brNewBreveCall(n, "dataWriteObject", stDDataWriteObject, AT_INT, AT_STRING, 0);
    brNewBreveCall(n, "dataWriteObjectWithDialog", stDDataWriteObjectWithDialog, AT_INT, 0);
    brNewBreveCall(n, "dataReadObject", stDDataReadObject, AT_INT, AT_STRING, 0);
    brNewBreveCall(n, "dataReadXMLObject", stDDataReadXMLObject, AT_INT, AT_STRING, 0);
    brNewBreveCall(n, "dataWriteXMLObject", stDWriteXMLObject, AT_INT, AT_STRING, 0);
    brNewBreveCall(n, "dataReadObjectWithDialog", stDDataReadObjectWithDialog, AT_INT, 0);
    brNewBreveCall(n, "simpleCrossover", stCSimpleCrossover, AT_NULL, AT_INSTANCE, AT_INSTANCE, AT_INSTANCE, 0);
}
