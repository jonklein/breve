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

/* 
    the ffTrain functions are functions that would be used to train the 
    feed forward network with raw data sets, but are not likely to come
    in useful as part of steve.
*/

/* define the ASEDELTA we should stop at */

#define ASEDELTA .0001

void testNet(snFFLayer *input, snFFLayer *output, double **inputs, double **outputs, int sets, int classes) {
    int n;
    int m;

    double tolerance, diff;

    int passes = 0, pass;

    /* for n classes we can be off by .5/n in either direction for a */
    /* successful guess... */

    tolerance = 1.0/(double)((classes - 1) * 2); 

    printf("tolerance = %f\n", tolerance);

    for(n=0;n<sets;n++) {
        for(m=0;m<input->count;m++) {
            input->values[m] = inputs[n][m];
        }

        snFeedForward(output);

        pass = 1;

        diff = 0;

        for(m=0;m<output->count;m++) {
            diff = output->values[m] - outputs[n][m];
            if(fabs(diff) > tolerance) pass = 0; 
        }

        if(pass) passes++;

        printf("output: %f, desired: %f [%f]", output->values[0], outputs[n][0], diff);

        if(pass) printf("\n");
        else printf(" X\n");
    }

    printf("%f%% pass rate (%d/%d)\n", (passes*100.0/sets), passes, sets);

}

/* 
    given an input and output layer, the network is trained for all the 
    input and output sets.  each input array must contain entries for 
    each input number, and likewise with the output arrays.  'sets' specifies
    the number of arrays to train for.
*/

void backPropSets(snFFLayer *input, snFFLayer *output, double **inputs, double **outputs, int sets, int iterations) {
    int n, m;
    int i, ten;
    double ase = 0, oldAse = 0;

    int *order;

    ten = iterations/10;

    order = slMalloc(sizeof(int) * sets);

    for(i=0;i<iterations;i++) {
        if(!(i % ten)) {
            printf(".");
        }

        /* we'll initialize finished to 1, then change it to 0 */
        /* if any of the training sets aren't finished */

        randomOrder(order, sets);

        ase = 0.0;

        for(n=0;n<sets;n++) {
            for(m=0;m<input->count;m++) {
                input->values[m] = inputs[order[n]][m];
            }

            snFeedForward(output);

            ase += snBackProp(output, outputs[order[n]]);
        }

        ase /= sets;

        if(iterations > 5000 && fabs(oldAse - ase) / ase < ASEDELTA) {
             printf("training complete after %i iterations (%f, %f)\n", i, ase, oldAse);
             return;
        }

        oldAse = ase;
    }

    slFree(order);
}

void randomOrder(int *vector, int count) {
    int n;

    for(n=0;n<count;n++) {
        vector[n] = n;
    } 

    qsort(vector, count, sizeof(int), randomSort);
}

int randomSort(const void *a, const void *b) {
    if(rand() > rand()) return 1;

    return -1;
}
