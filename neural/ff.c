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

#include "neural.h"

#define AC 1
#define LC .45
#define MC .45

snFFLayer *snNewLayer(int count, snFFLayer *previous) {
    snFFLayer *l;
    int n, m;

    if(count < 1) return NULL;

    l = slMalloc(sizeof(snFFLayer));

    l->count = count;

    /* we'll need space for both the sums and the activation */
    /* values in order to run the back propogation algorithm */

    l->values = slMalloc(sizeof(double)*count);
    l->sums = slMalloc(sizeof(double)*count);
    l->deltas = slMalloc(sizeof(double)*count);

    for(n=0;n<count;n++) {
        l->values[n] = 0.0;
        l->sums[n] = 0.0;
        l->deltas[n] = 0.0;
    }

    l->previous = previous;
    l->next = NULL;


    /* if there's a previous layer, allocate space for the weights */

    if(previous) {
        l->input = previous->input;
        previous->next = l;

        /* we'll need 'count' sets of weights... */

        l->weights = slMalloc(sizeof(double*)*count);
        l->previousDeltas = slMalloc(sizeof(double*)*count);

        /* and each one goes back to everything on the last layer */

        for(n=0;n<count;n++) {
            /* + 1 for good luck, or for threshold, if you wish */

            l->weights[n] = slMalloc(sizeof(double)*previous->count + 1);
            l->previousDeltas[n] = slMalloc(sizeof(double)*previous->count + 1);

            for(m=0;m<previous->count + 1;m++) {
                l->weights[n][m] = 1.5;
                l->previousDeltas[n][m] = 0.0;
            }
        }
    } else { /* no previous layer */
        l->input = l;
    }

    return l;
}

void snFreeNetwork(snFFLayer *output) {
	int n;

    if(!output) return;

    slFree(output->sums);
    slFree(output->values);
    slFree(output->deltas);

	if(output->previous) {
		for(n=0;n<output->count;n++) {
	    	slFree(output->weights[n]);
    		slFree(output->previousDeltas[n]);
		}

    	slFree(output->weights);
    	slFree(output->previousDeltas);
	}

    snFreeNetwork(output->previous);

    slFree(output);
}

void snFeedForward(snFFLayer *output) {
    int node, input;
 
    if(!output || !output->previous) return;

    snFeedForward(output->previous);

    for(node=0;node<output->count;node++) {
        output->sums[node] = 0;

        for(input=0;input<output->previous->count;input++) {
            output->sums[node] += output->weights[node][input] * output->previous->values[input];
        }

        /* subtract the threshold */
     
        output->sums[node] -= output->weights[node][input];

        output->values[node] = snFFActivation(output->sums[node]);
    }
}

double snFFActivation(double input) {
    return 1.0 / (1.0 + exp(-AC * input));
}

double snFFActivationDeriv(double input) {
    return AC * exp(-AC * input)/pow(1.0 + exp(-AC * input), 2.0);
}

double snBackProp(snFFLayer *output, double *desiredOutputs) {
    int n, m;
    double deriv, deltaSum, deltaWeight, ase = 0, ej;
    snFFLayer *next;

    if(!output->previous) return 0;

    /* first find the deltas--if there is no next layer, then use the */
    /* output layer delta calculation, otherwise use the back prop    */
    /* delta calculation. */
 
    if(!output->next) {
        for(n=0;n<output->count;n++) {
            deriv = snFFActivationDeriv(output->sums[n]);
            ej = desiredOutputs[n] - output->values[n];

            output->deltas[n] = deriv * ej;
            ase += .5 * ej * ej;
        }
    } else {
        next = output->next;

        for(n=0;n<output->count;n++) {
            deriv = snFFActivationDeriv(output->sums[n]);
            deltaSum = 0.0;

            for(m=0;m<next->count;m++) deltaSum += next->weights[m][n] * next->deltas[m];

            output->deltas[n] = deriv * deltaSum;
        }
    }

    /* recurse back through the network computing the rest of the deltas */

    snBackProp(output->previous, NULL);

    /* compute and activate the weight changes */

    for(n=0;n<output->count;n++) {
        for(m=0;m<output->previous->count;m++) {
            deltaWeight = LC * output->deltas[n] * output->previous->values[m];
            deltaWeight += MC * (output->previousDeltas[n][m]);

            output->previousDeltas[n][m] = deltaWeight;
            output->weights[n][m] += deltaWeight;
        }
    }

    return ase;
}

void snRandomizeNetworkWeights(snFFLayer *output, int c) {
    int n, m;

    double range, halfrange;

    range = 1.0/sqrt(c);
    halfrange = range/2;
  
    if(output->previous) {
        for(n=0;n<output->count;n++) {
            for(m=0;m<output->previous->count;m++) {
                output->weights[n][m] = range*(((double)random())/0x7fffffff) - halfrange;
            }
        }

        snRandomizeNetworkWeights(output->previous, c);
    }
}

void snPrintWeights(snFFLayer *output) {
    int n, m;

    if(!output || !output->previous) return;

    slMessage(DEBUG_ALL, "snFFLayer %p\n", output);

    for(n=0;n<output->count;n++) {
        for(m=0;m<output->previous->count;m++) {
            slMessage(DEBUG_ALL, "%d-%d: %f\n", n, m, output->weights[n][m]);
        }
    }

    snPrintWeights(output->previous);
}
