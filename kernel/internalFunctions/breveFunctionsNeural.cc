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

/*! \addtogroup InternalFunctions */
/*@{*/

#include "kernel.h"
#include "neural.h"

/*!
	\brief Fress a neural network structure.

	void freeNetwork(snFFLayer pointer outputLayer).
*/

int brIFreeNetwork(brEval args[], brEval *target, brInstance *i) {
    snFreeNetwork(BRPOINTER(&args[0]));

    return EC_OK;
}

/*!
	\brief Given a layer of a neural network, adds another layer of a given size.

	void newLayer(int size, snFFLayer pointer layer).
*/

int brINewFFLayer(brEval args[], brEval *target, brInstance *i) {
    BRPOINTER(target) = snNewLayer(BRINT(&args[0]), BRPOINTER(&args[1]));

    if(!BRPOINTER(target)) return EC_ERROR;

    return EC_OK;
}

/*!
	\brief Randomize the weights of a network.

	void randomizeWeights(snFFLayer pointer layer).
*/

int brIRandomizeWeights(brEval args[], brEval *target, brInstance *i) {
    snRandomizeNetworkWeights(BRPOINTER(&args[0]), 8);

    return EC_OK;
}

/*!
	\brief Sets a weight in a network layer.

	void randomizeWeights(snFFLayer pointer layer, int toNode, int fromNode, double weight).
*/

int brISetWeight(brEval args[], brEval *target, brInstance *i) {
	snFFLayer *layer = BRPOINTER(&args[0]);
	int toNode = BRINT(&args[1]);
	int fromNode = BRINT(&args[2]);
	double weight = BRDOUBLE(&args[3]);

	if(!layer->previous) {
        slMessage(DEBUG_ALL, "setWeight called on input layer (no weights to be set)");
        return EC_ERROR;
	}

	if(toNode < 0 || toNode >= layer->count) {
        slMessage(DEBUG_ALL, "setWeight called with out of bounds node number");
        return EC_ERROR;
	}

	if(fromNode < 0 || fromNode >= layer->previous->count) {
        slMessage(DEBUG_ALL, "setWeight called with out of bounds node number from previous layer");
        return EC_ERROR;
	}

	layer->weights[toNode][fromNode] = weight;

	return EC_OK;
}

/*!
	\brief Gets a weight from a network layer.

	double getWeight(snFFLayer pointer network, int toNode, int fromNode).
*/

int brIGetWeight(brEval args[], brEval *target, brInstance *i) {
	snFFLayer *layer = BRPOINTER(&args[0]);
	int toNode = BRINT(&args[1]);
	int fromNode= BRINT(&args[2]);

	if(!layer->previous) {
        slMessage(DEBUG_ALL, "getWeight called on input layer (no weights to be retrieved)");
        return EC_ERROR;
	}

	if(toNode < 0 || toNode >= layer->count) {
        slMessage(DEBUG_ALL, "getWeight called with out of bounds node number");
        return EC_ERROR;
	}

	if(fromNode < 0 || fromNode >= layer->previous->count) {
        slMessage(DEBUG_ALL, "getWeight called with out of bounds node number from previous layer");
        return EC_ERROR;
	}

	BRDOUBLE(target) = layer->weights[toNode][fromNode];

	return EC_OK;
}

/*!
	\brief Gets the output value of the specified node in a given layer.

	double getValue(snFFLayer pointer layer, int node).
*/

int brIGetValue(brEval args[], brEval *target, brInstance *i) {
	snFFLayer *layer = BRPOINTER(&args[0]);
	int node = BRINT(&args[1]);

	if(node < 0 || node >= layer->count) {
        slMessage(DEBUG_ALL, "getValue called with out of bounds node number");
        return EC_ERROR;
	}

	BRDOUBLE(target) = layer->values[node];

	return EC_OK;
}

/*!
	\brief Runs a feed-forward network through one iteration.

	void feedForward(snFFLayer pointer network, list inputs, list outputs).

	Stores the output values in the output list.
*/

int brIFeedForward(brEval args[], brEval *target, brInstance *i) {
    brEvalListHead *inputs, *outputs;
    brEvalList *list;
    snFFLayer *outputLayer, *inputLayer;
    int count;

    outputLayer = BRPOINTER(&args[0]);
    inputLayer = outputLayer->input;

    inputs = BRPOINTER(&args[1]);
    outputs = BRPOINTER(&args[2]);

    if(inputs->count != inputLayer->count) {
        slMessage(DEBUG_ALL, "feedForward expects list matching size of network input layer (%d inputs, list is size %d)", inputLayer->count, inputs->count);
        return EC_ERROR;
    }

    list = inputs->start;
    count = 0;

    while(list) {
        inputLayer->values[count] = BRDOUBLE(&list->eval);

        count++;
        list = list->next; 
    }

    snFeedForward(outputLayer);

    return EC_OK;
}
/*@}*/

void breveInitNeuralFunctions(brNamespace *n) {
    brNewBreveCall(n, "newFFLayer", brINewFFLayer, AT_POINTER, AT_INT, AT_POINTER, 0);
    brNewBreveCall(n, "freeNetwork", brIFreeNetwork, AT_NULL, AT_POINTER, 0);
    brNewBreveCall(n, "randomizeFFWeights", brIRandomizeWeights, AT_NULL, AT_POINTER, 0);
    brNewBreveCall(n, "getValue", brIGetValue, AT_DOUBLE, AT_POINTER, AT_INT, 0);
    brNewBreveCall(n, "setWeight", brISetWeight, AT_NULL, AT_POINTER, AT_INT, AT_INT, AT_DOUBLE, 0);
    brNewBreveCall(n, "getWeight", brIGetWeight, AT_DOUBLE, AT_POINTER, AT_INT, AT_INT, 0);
    brNewBreveCall(n, "feedForward", brIFeedForward, AT_NULL, AT_POINTER, AT_LIST, 0);
}
