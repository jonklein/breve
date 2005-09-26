/*****************************************************************************
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000-2004 Jonathan Klein                                    *
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
 *  @file slNeuralNetworkLearningAlgorithm.cc
 *  @breif A neural network learning algorithm class.
 *  @author Eric DeWitt
 *  Copyright (C) 2005 Eric DeWitt, Jonathan Klein
 */

#include "util.h"
#include "neuralNetworkLearningAlgorithm.hh"
#include "bigMatrix.hh"

slNNLearningAlgorithm::~slNNLearningAlgorithm()
{

}

void slNNLearningAlgorithm::setSynapseTimeStep(const float scalar)
{
    _deltaTime = scalar;
    // we combine deltaTime and (1 / _timeConstant) for efficiency
    _tauWeight = (_deltaTime * (1.0 / _timeConstant));

}

float slNNLearningAlgorithm::getSynapseTimeStep() const
{
    return _deltaTime;   
}

void slNNLearningAlgorithm::setSynapseTimeConstant(const float scalar)
{
    _timeConstant = scalar;
    // we combine deltaTime and (1 / _timeConstant) for efficiency
    _tauWeight = (_deltaTime * (1.0 / _timeConstant));    
}

float slNNLearningAlgorithm::getSynapseTimeConstant() const
{
    return _timeConstant;
}

void slNNLearningAlgorithm::setWeightBounds(const float lower, const float upper)
{
    if (lower == upper)
    {
        _boundWeights = false;
        _boundLower = 0.0;
        _boundUpper = 0.0;
    }
    else
    {
        _boundWeights = true;
        _boundLower = lower;
        _boundUpper = upper;
    }
}

slNNHebbianAlgorithm::slNNHebbianAlgorithm(slNeuralNetwork& theNetwork)
    :   _network(&theNetwork),
        _recurrentNetwork(0),
        _recurrent(false),
        _norm(false),
        _normMethod(VECLENGTH),
        _alpha(1.0)
{
    slNNLearningAlgorithm::_deltaTime = 1.0;
    slNNLearningAlgorithm::_timeConstant = 1000.0;
    slNNLearningAlgorithm::_tauWeight = (slNNLearningAlgorithm::_deltaTime * (
                                         1.0 / slNNLearningAlgorithm::_timeConstant));    
    slNNLearningAlgorithm::_boundWeights = false;
    slNNLearningAlgorithm::_boundLower = 0.0;
    slNNLearningAlgorithm::_boundUpper = 0.0;
    
    _connectionWeightsDelta._recurrent = new slBigMatrix2DGSL(_network->_connectionWeights.xDim(), _network->_connectionWeights.yDim());
    _connectionWeightsDelta._recurrentB = new slBigMatrix2DGSL(_network->_connectionWeights.xDim(), _network->_connectionWeights.yDim());
    _connectionWeightsDelta._recurrentUnity = new slBigVectorGSL(_network->_currentNeuronStates.dim());
}

slNNHebbianAlgorithm::slNNHebbianAlgorithm(slRecurrentNetwork& theNetwork)
    :   _network(&theNetwork),
        _recurrentNetwork(&theNetwork),
        _recurrent(true),
        _norm(false),
        _normMethod(VECLENGTH),
        _alpha(1.0)
{
    slNNLearningAlgorithm::_deltaTime = 1.0;
    slNNLearningAlgorithm::_timeConstant = 1.0;
    slNNLearningAlgorithm::_tauWeight = (slNNLearningAlgorithm::_deltaTime * (
                                         1.0 / slNNLearningAlgorithm::_timeConstant));
    slNNLearningAlgorithm::_boundWeights = false;
    slNNLearningAlgorithm::_boundLower = 0.0;
    slNNLearningAlgorithm::_boundUpper = 0.0;
    
    _connectionWeightsDelta._input = new slBigMatrix2DGSL(_recurrentNetwork->_connectionWeights.xDim(), _network->_connectionWeights.yDim());
    _connectionWeightsDelta._inputB = new slBigMatrix2DGSL(_recurrentNetwork->_connectionWeights.xDim(), _network->_connectionWeights.yDim());
    _connectionWeightsDelta._inputUnity = new slBigVectorGSL(_recurrentNetwork->_currentNeuronStates.dim());
    _connectionWeightsDelta._recurrent = new slBigMatrix2DGSL(_network->_connectionWeights.xDim(), _recurrentNetwork->_connectionWeights.yDim());
    _connectionWeightsDelta._recurrentB = new slBigMatrix2DGSL(_network->_connectionWeights.xDim(), _recurrentNetwork->_connectionWeights.yDim());
    _connectionWeightsDelta._recurrentUnity = new slBigVectorGSL(_network->_currentNeuronStates.dim());
}

slNNHebbianAlgorithm::~slNNHebbianAlgorithm()
{
    delete _connectionWeightsDelta._recurrent;
    if (_recurrent) delete _connectionWeightsDelta._input;
}

void slNNHebbianAlgorithm::setNormalizeOff()
{
    _norm = false;
}

void slNNHebbianAlgorithm::setNormalizeSubtractive()
{
    _norm = true;
    _normMethod = SUBTRACT;
}
void slNNHebbianAlgorithm::setNormalizeMultiplicitive()
{
    _norm = true;
    _normMethod = MULTIPLIC;
}
void slNNHebbianAlgorithm::setNormalizeVectorMagnitude()
{
    _norm = true;
    _normMethod = VECLENGTH;
}
void slNNHebbianAlgorithm::setNormalizeAlpha(const float scalar)
{
    _alpha = scalar;
}

void slNNHebbianAlgorithm::updateNetworkConnectionWeights()
{
    unsigned int i;
    float norm;
    float tauAlpha  = (_tauWeight * _alpha);
    
    if (_recurrent) // dW & dM
    {
        // calc dW
        // u[].v[] * 1/t
        _recurrentNetwork->_currentNeuronStates.outerProductInto(_recurrentNetwork->_externalInputNeuronStates, *_connectionWeightsDelta._input);
        _connectionWeightsDelta._input->inPlaceMultiply(_tauWeight);
        // norm term or method
        if (_norm)
        {
            switch (_normMethod)
            {
                case VECLENGTH:
                    
                    _connectionWeightsDelta._input->inPlaceMultiply(_recurrentNetwork->_inputConnectionStructure);
                    for (i = 0; i < _connectionWeightsDelta._input->xDim(); i++)
                    {
                        norm = _connectionWeightsDelta._input->getRowMagnitude(i);
                        _connectionWeightsDelta._input->inPlaceRowMultiply(i, norm);
                        if (norm != 0)
                            norm = (tauAlpha / norm);
                    }
                    break;

                case MULTIPLIC:

                    _connectionWeightsDelta._inputB->copyData(_recurrentNetwork->_inputConnectionWeights);
                    for (i = 0; i < _recurrentNetwork->_currentNeuronStates.dim(); i++)
                    {
                        norm = (tauAlpha * _recurrentNetwork->_currentNeuronStates.get(i)
                                     * _recurrentNetwork->_currentNeuronStates.get(i));
                        _connectionWeightsDelta._inputB->inPlaceRowMultiply(i, norm);
                    }
                    _connectionWeightsDelta._input->inPlaceScaleAndAdd(-1.0, *_connectionWeightsDelta._inputB);                    
                    break;

                case SUBTRACT:

                    norm = ((_tauWeight * _recurrentNetwork->_externalInputNeuronStates.sum())
                             / _recurrentNetwork->_externalInputNeuronStates.dim());
                    _connectionWeightsDelta._inputUnity->setAll(norm);
                    _recurrentNetwork->_currentNeuronStates.outerProductInto(*_connectionWeightsDelta._inputUnity,
                                                                             *_connectionWeightsDelta._inputB);
                    _connectionWeightsDelta._input->inPlaceScaleAndAdd(-1.0, *_connectionWeightsDelta._inputB);
                    break;
                    
            }
        }
        _connectionWeightsDelta._input->inPlaceMultiply(_recurrentNetwork->_inputConnectionStructure);
        _recurrentNetwork->_inputConnectionWeights.inPlaceAdd(*_connectionWeightsDelta._input);
        
        if (_boundWeights)
        {
            _recurrentNetwork->_inputConnectionWeights.clamp(_boundLower, _boundLower, _boundUpper);    

        }
    }
    // calc dM
    _network->_currentNeuronStates.outerProductInto(_network->_currentNeuronStates, *_connectionWeightsDelta._recurrent);
    _connectionWeightsDelta._recurrent->inPlaceMultiply(_tauWeight);
    if (_norm)
    {
            switch (_normMethod)
            {
                case VECLENGTH:

                _connectionWeightsDelta._recurrent->inPlaceMultiply(_network->_connectionStructure);
                for (i = 0; i < _connectionWeightsDelta._recurrent->yDim(); i++)
                {
                    norm = _connectionWeightsDelta._recurrent->getRowMagnitude(i);
                    if (norm !=0)
                        norm = (tauAlpha / norm);
                    _connectionWeightsDelta._recurrent->inPlaceRowMultiply(i, norm);
                }
                break;

                case MULTIPLIC:

                _connectionWeightsDelta._recurrentB->copyData(_network->_connectionWeights);
                for (i = 0; i < _network->_connectionWeights.yDim(); i++)
                {
                    norm = (tauAlpha * _network->_currentNeuronStates.get(i)
                                 * _network->_currentNeuronStates.get(i));
                    _connectionWeightsDelta._recurrentB->inPlaceRowMultiply(i, norm);
                }
                _connectionWeightsDelta._recurrent->inPlaceScaleAndAdd(-1.0, *_connectionWeightsDelta._recurrentB);                               
                break;

                case SUBTRACT:
                
                    norm = ((_tauWeight * _network->_currentNeuronStates.sum())
                             / _network->_currentNeuronStates.dim());
                    _connectionWeightsDelta._recurrentUnity->setAll(norm);
                    _network->_currentNeuronStates.outerProductInto(*_connectionWeightsDelta._recurrentUnity,
                                                                    *_connectionWeightsDelta._recurrentB);
                    _connectionWeightsDelta._recurrent->inPlaceScaleAndAdd(-1.0, *_connectionWeightsDelta._recurrentB);
                break;
            }
    }
    _weightDeltaMag = _connectionWeightsDelta._recurrent->magnitude();
//    slMessage(0, "Delta weight magnitude is %e.\n", _weightDeltaMag);
    _connectionWeightsDelta._recurrent->inPlaceMultiply(_network->_connectionStructure);
    _network->_connectionWeights.inPlaceAdd(*_connectionWeightsDelta._recurrent);
    if (_boundWeights)
    {
        _network->_connectionWeights.clamp(_boundLower, _boundLower, _boundUpper);    
    }
    
}

slNNBCMAlgorithm::slNNBCMAlgorithm(slNeuralNetwork& theNetwork)
    : slNNHebbianAlgorithm(theNetwork)
{

}

slNNBCMAlgorithm::slNNBCMAlgorithm(slRecurrentNetwork& theNetwork)
    : slNNHebbianAlgorithm(theNetwork)
{

}

slNNBCMAlgorithm::~slNNBCMAlgorithm()
{

}

void slNNBCMAlgorithm::updateNetworkConnectionWeights()
{

}

slNNSupervisedHebbianAlgorithm::slNNSupervisedHebbianAlgorithm(slNeuralNetwork& theNetwork)
{

}

slNNSupervisedHebbianAlgorithm::slNNSupervisedHebbianAlgorithm(slRecurrentNetwork& theNetwork)
{

}

slNNSupervisedHebbianAlgorithm::~slNNSupervisedHebbianAlgorithm()
{

}

void slNNSupervisedHebbianAlgorithm::updateNetworkConnectionWeights()
{

}

slNNBackPropagationAlgorithm::slNNBackPropagationAlgorithm(slFeedForwardNetwork& theNetwork)
{

}

slNNBackPropagationAlgorithm::~slNNBackPropagationAlgorithm()
{

}

void slNNBackPropagationAlgorithm::updateNetworkConnectionWeights()
{

}
