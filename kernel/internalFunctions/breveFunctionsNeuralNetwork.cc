/*****************************************************************************
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000-2004 Jonathan Klein, Eric DeWitt                       *
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

/**
 *  @file breveFunctionsNeuralNetwork.cc
 *  @breif A eural network manipulation class.
 *  @author Eric DeWitt
 *  Copyright (C) 2005 Eric DeWitt, Jonathan Klein
 *
 * This library of breve functions provide robust vector routines for
 * use with 2D and 3D matrices of large sizes.  The goal is to provide 
 * hardware acceleration and eventually a full complement of matrix
 * routines.  This may include vector and matrix operations, convolutions,
 * ffts, etc.
 *
 */

#include "kernel.h"
#include "neuralNetwork.hh"
#include "neuralNetworkLearningAlgorithm.hh"

typedef slNeuralNetwork brNeuralNetwork;
typedef slRecurrentNetwork brRecurrentNetwork;
typedef slFeedForwardNetwork brFeedForwardNetwork;
typedef slNNLearningAlgorithm brNNLearningAlgorithm;
typedef slNNHebbianAlgorithm brNNHebbianAlgorithm;
typedef slNNBCMAlgorithm brNNBCMAlgorithm;
typedef slNNSupervisedHebbianAlgorithm brNNSupervisedHebbianAlgorithm;
typedef slNNBackPropagationAlgorithm brNNBackPropagationAlgorithm;
#define BRNEURALNETWORK(ptr) (reinterpret_cast<brNeuralNetwork*>(BRPOINTER(ptr)))
#define BRRECURRENTNETWORK(ptr) (reinterpret_cast<brRecurrentNetwork*>(BRPOINTER(ptr)))
#define BRFEEDFORWARDNETWORK(ptr) (reinterpret_cast<brFeedForwardNetwork*>(BRPOINTER(ptr)))
#define BRLEARNINGALGORITHM(ptr) (reinterpret_cast<brNNLearningAlgorithm*>(BRPOINTER(ptr)))
#define BRHEBBIANALGORITHM(ptr) (reinterpret_cast<brNNHebbianAlgorithm*>(BRPOINTER(ptr)))


int brINeuralNetworkNew(brEval args[], brEval *target, brInstance *i)
{
    target->set( new brNeuralNetwork(BRINT(&args[0])) );
    return EC_OK;
}

int brINeuralNetworkFree(brEval args[], brEval *target, brInstance *i)
{
    if(BRPOINTER(&args[0])) delete BRNEURALNETWORK(&args[0]);
    return EC_OK;
}

int brINeuralNetworkSetTimeStep(brEval args[], brEval *target, brInstance *i)
{
    (BRNEURALNETWORK(&args[0]))->setNeuronTimeStep(BRDOUBLE(&args[1]));
    return EC_OK;
}

int brINeuralNetworkSetTimeConstant(brEval args[], brEval *target, brInstance *i)
{
    (BRNEURALNETWORK(&args[0]))->setNeuronTimeConstant(BRDOUBLE(&args[1]));
    return EC_OK;
}

int brINeuralNetworkSetNeuronStateBounds(brEval args[], brEval *target, brInstance *i)
{
    (BRNEURALNETWORK(&args[0]))->setNeuronStateBounds(BRDOUBLE(&args[1]), BRDOUBLE(&args[2]));
    return EC_OK;
}

int brINeuralNetworkGetNeuronStateVector(brEval args[], brEval *target, brInstance *i)
{
    target->set( (BRNEURALNETWORK(&args[0]))->getNeuronStateVector() );
    return EC_OK;
}

int brINeuralNetworkGetMaxWeight(brEval args[], brEval *target, brInstance *i)
{
    target->set( double(BRNEURALNETWORK(&args[0])->getMaxWeight()) );
    return EC_OK;
}

int brINeuralNetworkGetMinWeight(brEval args[], brEval *target, brInstance *i)
{
    target->set( double(BRNEURALNETWORK(&args[0])->getMinWeight()) );
    return EC_OK;
}

int brINeuralNetworkGetMaxActivity(brEval args[], brEval *target, brInstance *i)
{
    target->set( double(BRNEURALNETWORK(&args[0])->getMaxNeuralActivity()) );
    return EC_OK;
}

int brINeuralNetworkGetMinActivity(brEval args[], brEval *target, brInstance *i)
{
    target->set( double(BRNEURALNETWORK(&args[0])->getMinNeuralActivity()) );
    return EC_OK;
}


int brINeuralNetworkGetNeuronState(brEval args[], brEval *target, brInstance *i)
{
    target->set( (BRNEURALNETWORK(&args[0]))->getNeuronState(BRINT(&args[1])) );
    return EC_OK;
}

int brINeuralNetworkSetNeuronState(brEval args[], brEval *target, brInstance *i)
{
    (BRNEURALNETWORK(&args[0]))->setNeuronState(BRINT(&args[1]), BRDOUBLE(&args[2]));
    return EC_OK;
}

int brINeuralNetworkSetInputNeurons(brEval args[], brEval *target, brInstance *i)
{
    (BRNEURALNETWORK(&args[0]))->setInputNeurons(BRINT(&args[1]), BRINT(&args[2]));
    target->set( (BRNEURALNETWORK(&args[0]))->getInputNeurons() );
    return EC_OK;
}

int brINeuralNetworkSetOutputNeurons(brEval args[], brEval *target, brInstance *i)
{
    (BRNEURALNETWORK(&args[0]))->setOutputNeurons(BRINT(&args[1]), BRINT(&args[2]));
    target->set( (BRNEURALNETWORK(&args[0]))->getOutputNeurons() );
    return EC_OK;
}

int brINeuralNetworkGetConnectionWeightMatrix(brEval args[], brEval *target, brInstance *i)
{
    target->set( (BRNEURALNETWORK(&args[0]))->getConnectionWeightMatrix() );
    return EC_OK;
}

int brINeuralNetworkGetConnectionStructureMatrix(brEval args[], brEval *target, brInstance *i)
{
    target->set( (BRNEURALNETWORK(&args[0]))->getConnectionStructureMatrix() );
    return EC_OK;
}

int brINeuralNetworkApplyStructure(brEval args[], brEval *target, brInstance *i)
{
    (BRNEURALNETWORK(&args[0]))->applyStructureToWeights();
    return EC_OK;
}

int brINeuralNetworkUpdateState(brEval args[], brEval *target, brInstance *i)
{
    (BRNEURALNETWORK(&args[0]))->updateNeuronStates();
    return EC_OK;
}

/****** Recurrent Network Interface *******/
int brIRecurrentNetworkNew(brEval args[], brEval *target, brInstance *i)
{
    target->set( new brRecurrentNetwork(BRINT(&args[0]), BRINT(&args[1])) );
    return EC_OK;
}

int brIRecurrentNetworkFree(brEval args[], brEval *target, brInstance *i)
{
    if(BRPOINTER(target)) delete BRRECURRENTNETWORK(target);
    return EC_OK;
}

int brINeuralRecurrentUpdateState(brEval args[], brEval *target, brInstance *i)
{
    (BRRECURRENTNETWORK(&args[0]))->updateNeuronStates();
    return EC_OK;
}

int brINeuralRecurrentApplyStructure(brEval args[], brEval *target, brInstance *i)
{
    (BRRECURRENTNETWORK(&args[0]))->applyStructureToWeights();
    return EC_OK;
}

/***** Learning Algorithms ******/

int brINNHebbianAlgorithmNNNew(brEval args[], brEval *target, brInstance *i)
{
    target->set( new brNNHebbianAlgorithm(*BRNEURALNETWORK(&args[0])) );
    return EC_OK;
}

int brINNHebbianAlgorithmNNFree(brEval args[], brEval *target, brInstance *i)
{
    if(BRPOINTER(&args[0])) delete BRNEURALNETWORK(&args[0]);
    return EC_OK;
}

int brINNHebbianAlgorithmRNNew(brEval args[], brEval *target, brInstance *i)
{
    target->set( new brNNHebbianAlgorithm(*BRRECURRENTNETWORK(&args[0])) );
    return EC_OK;
}

int brINNHebbianAlgorithmRNFree(brEval args[], brEval *target, brInstance *i)
{
    if(BRPOINTER(&args[0])) delete BRNEURALNETWORK(&args[0]);
    return EC_OK;
}

int brIHebbianAlgorithmNormalizeOff(brEval args[], brEval *target, brInstance *i)
{
    BRHEBBIANALGORITHM(&args[0])->setNormalizeOff();
    return EC_OK;
}

int brIHebbianAlgorithmNormalizeSubtractive(brEval args[], brEval *target, brInstance *i)
{
    BRHEBBIANALGORITHM(&args[0])->setNormalizeSubtractive();
    return EC_OK;
}

int brIHebbianAlgorithmNormalizeMultiplicitive(brEval args[], brEval *target, brInstance *i)
{
    BRHEBBIANALGORITHM(&args[0])->setNormalizeMultiplicitive();
    return EC_OK;
}

int brIHebbianAlgorithmNormalizeVectorMagnitude(brEval args[], brEval *target, brInstance *i)
{
    BRHEBBIANALGORITHM(&args[0])->setNormalizeVectorMagnitude();
    return EC_OK;
}

int brIHebbianAlgorithmNormalizeAlpha(brEval args[], brEval *target, brInstance *i)
{
    BRHEBBIANALGORITHM(&args[0])->setNormalizeAlpha(BRDOUBLE(&args[1]));
    return EC_OK;
}

int brINNBCMAlgorithmNNNew(brEval args[], brEval *target, brInstance *i)
{
    target->set( new brNNBCMAlgorithm(*BRNEURALNETWORK(&args[0])) );
    return EC_OK;
}

int brINNBCMAlgorithmNNFree(brEval args[], brEval *target, brInstance *i)
{
    if(BRPOINTER(&args[0])) delete BRNEURALNETWORK(&args[0]);
    return EC_OK;
}

int brINNBCMAlgorithmRNNew(brEval args[], brEval *target, brInstance *i)
{
    target->set( new brNNBCMAlgorithm(*BRRECURRENTNETWORK(&args[0])) );
    return EC_OK;
}

int brINNBCMAlgorithmRNFree(brEval args[], brEval *target, brInstance *i)
{
    if(BRPOINTER(&args[0])) delete BRNEURALNETWORK(&args[0]);
    return EC_OK;
}

int brINNLearningAlgorithmSetTimeStep(brEval args[], brEval *target, brInstance *i)
{
    (BRLEARNINGALGORITHM(&args[0]))->setSynapseTimeStep(BRDOUBLE(&args[1]));
    return EC_OK;
}

int brINNLearningAlgorithmSetTimeConstant(brEval args[], brEval *target, brInstance *i)
{
    (BRLEARNINGALGORITHM(&args[0]))->setSynapseTimeConstant(BRDOUBLE(&args[1]));
    return EC_OK;
}

int brINNLearningAlgorithmSetWeightBounds(brEval args[], brEval *target, brInstance *i)
{
    (BRLEARNINGALGORITHM(&args[0]))->setWeightBounds(BRDOUBLE(&args[1]), BRDOUBLE(&args[2]));
    return EC_OK;
}

int brINNLearningAlgorithmUpdateConnectionWeights(brEval args[], brEval *target, brInstance *i)
{
    BRLEARNINGALGORITHM(&args[0])->updateNetworkConnectionWeights();
    return EC_OK;
}

/****** namespace work ********/
void breveInitNeuralNetworkFunctions(brNamespace *n)
{

    /* Base NeuralNetwork class calls */
    brNewBreveCall(n, "neuralNetworkNew", brINeuralNetworkNew, AT_POINTER, AT_INT, 0);
    brNewBreveCall(n, "neuralNetworkFree", brINeuralNetworkFree, AT_NULL, AT_POINTER, 0);

    brNewBreveCall(n, "neuralNetworkSetTimeStep", brINeuralNetworkSetTimeStep, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "neuralNetworkSetTimeConstant", brINeuralNetworkSetTimeConstant, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "neuralNetworkSetNeuronStateBounds", brINeuralNetworkSetNeuronStateBounds, AT_NULL, AT_POINTER, AT_DOUBLE, AT_DOUBLE, 0);
    brNewBreveCall(n, "neuralNetworkGetNeuronState", brINeuralNetworkGetNeuronState, AT_DOUBLE, AT_POINTER, AT_INT, 0);
    brNewBreveCall(n, "neuralNetworkSetNeuronState", brINeuralNetworkSetNeuronState, AT_NULL, AT_POINTER, AT_INT, AT_DOUBLE, 0);
    brNewBreveCall(n, "neuralNetworkGetNeuronStateVector", brINeuralNetworkGetNeuronStateVector, AT_POINTER, AT_POINTER, 0);    
    brNewBreveCall(n, "neuralNetworkSetInputNeurons", brINeuralNetworkSetInputNeurons, AT_POINTER, AT_POINTER, AT_INT, AT_INT, 0);
    brNewBreveCall(n, "neuralNetworkSetOutputNeurons", brINeuralNetworkSetOutputNeurons, AT_POINTER, AT_POINTER, AT_INT, AT_INT, 0);
    brNewBreveCall(n, "neuralNetworkGetConnectionWeightMatrix", brINeuralNetworkGetConnectionWeightMatrix, AT_POINTER, AT_POINTER, 0);
    brNewBreveCall(n, "neuralNetworkGetConnectionStructureMatrix", brINeuralNetworkGetConnectionStructureMatrix, AT_POINTER, AT_POINTER, 0);
    brNewBreveCall(n, "neuralNetworkApplyStructure", brINeuralNetworkApplyStructure, AT_NULL, AT_POINTER, 0);
    brNewBreveCall(n, "neuralNetworkGetMaxWeight", brINeuralNetworkGetMaxWeight, AT_DOUBLE, AT_POINTER, 0);
    brNewBreveCall(n, "neuralNetworkGetMinWeight", brINeuralNetworkGetMinWeight, AT_DOUBLE, AT_POINTER, 0);
    brNewBreveCall(n, "neuralNetworkGetMaxActivity", brINeuralNetworkGetMaxActivity, AT_DOUBLE, AT_POINTER, 0);
    brNewBreveCall(n, "neuralNetworkGetMinActivity", brINeuralNetworkGetMinActivity, AT_DOUBLE, AT_POINTER, 0);

    brNewBreveCall(n, "neuralNetworkUpdateState", brINeuralNetworkUpdateState, AT_NULL, AT_POINTER, 0);
            
    /* RecurrentNetwork class calls */
    brNewBreveCall(n, "neuralRecurrentNetworkNew", brIRecurrentNetworkNew, AT_POINTER, AT_INT, AT_INT, 0);
    brNewBreveCall(n, "neuralRecurrentNetworkFree", brIRecurrentNetworkFree, AT_NULL, AT_POINTER, 0);

    brNewBreveCall(n, "neuralRecurrentUpdateState", brINeuralRecurrentUpdateState, AT_NULL, AT_POINTER, 0);
    brNewBreveCall(n, "neuralRecurrentApplyStructure", brINeuralRecurrentApplyStructure, AT_NULL, AT_POINTER, 0);

    /* learning algorithms */
    brNewBreveCall(n, "nnHebbianAlgorithmNNNew", brINNHebbianAlgorithmNNNew, AT_POINTER, AT_POINTER, 0);
    brNewBreveCall(n, "nnHebbianAlgorithmNNFree", brINNHebbianAlgorithmNNFree, AT_NULL, AT_POINTER, 0);
    brNewBreveCall(n, "nnHebbianAlgorithmRNNew", brINNHebbianAlgorithmRNNew, AT_POINTER, AT_POINTER, 0);
    brNewBreveCall(n, "nnHebbianAlgorithmRNFree", brINNHebbianAlgorithmRNFree, AT_NULL, AT_POINTER, 0);
    brNewBreveCall(n, "nnBCMAlgorithmNNNew", brINNBCMAlgorithmNNNew, AT_POINTER, AT_POINTER, 0);
    brNewBreveCall(n, "nnBCMAlgorithmNNFree", brINNBCMAlgorithmNNFree, AT_POINTER, AT_POINTER, 0);
    brNewBreveCall(n, "nnBCMAlgorithmRNNew", brINNBCMAlgorithmRNNew, AT_POINTER, AT_POINTER, 0);
    brNewBreveCall(n, "nnBCMAlgorithmRNFree", brINNBCMAlgorithmRNFree, AT_POINTER, AT_POINTER, 0);

    brNewBreveCall(n, "nnLearningAlgorithmSetTimeStep", brINNLearningAlgorithmSetTimeStep, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "nnLearningAlgorithmSetTimeConstant", brINNLearningAlgorithmSetTimeConstant, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "nnLearningAlgorithmSetWeightBounds", brINNLearningAlgorithmSetWeightBounds, AT_NULL, AT_POINTER, AT_DOUBLE, AT_DOUBLE, 0);
    brNewBreveCall(n, "nnHebbianAlgorithmNormalizeOff", brIHebbianAlgorithmNormalizeOff, AT_NULL, AT_POINTER, 0);
    brNewBreveCall(n, "nnHebbianAlgorithmNormalizeSubtractive", brIHebbianAlgorithmNormalizeSubtractive, AT_NULL, AT_POINTER, 0);
    brNewBreveCall(n, "nnHebbianAlgorithmNormalizeMultiplicitive", brIHebbianAlgorithmNormalizeMultiplicitive, AT_NULL, AT_POINTER, 0);
    brNewBreveCall(n, "nnHebbianAlgorithmNormalizeVectorMagnitude", brIHebbianAlgorithmNormalizeVectorMagnitude, AT_NULL, AT_POINTER, 0);
    brNewBreveCall(n, "nnHebbianAlgorithmNormalizeAlpha", brIHebbianAlgorithmNormalizeAlpha, AT_NULL, AT_POINTER, AT_DOUBLE, 0);

    brNewBreveCall(n, "nnLearningAlgorithmUpdateConnectionWeights", brINNLearningAlgorithmUpdateConnectionWeights, AT_NULL, AT_POINTER, 0);

}
