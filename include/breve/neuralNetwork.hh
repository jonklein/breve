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
 *  @file neuralNetwork.hh
 *  @breif A neural network manipulation class.
 *  @author Eric DeWitt
 *  Copyright (C) 2005 Eric DeWitt, Jonathan Klein
 */
 
#ifndef _BREVE_NEURAL_NET_H
#define _BREVE_NEURAL_NET_H

#include <vector>

#include "bigMatrix.hh"


/*
 * This will be a generic neural network class which are all intended
 * to be compatible with evolutionary computation.
 *
 */
 
/**
 *  @file neuralNetwork util classes
 *
 *  The goal of this class is a generic neural network interface that
 *  is compatible with evolutionary computation.
 *
 */
 
class slNNLearningAlgorithm;
 
/**
 *  A basic neural network class.  This network has no defined external
 *  inputs or outputs.  A subset of continguous nodes can be defined as
 *  input nodes or output nodes (which must not overlap).
 */
class slNeuralNetwork {

    public:

        slNeuralNetwork(const unsigned size);
        
        void setNeuronTimeStep(const float scalar);
        float getNeuronTimeStep() const;

        void setNeuronTimeConstant(const float scalar);
        float getNeuronTimeConstant() const;
        
        void setNeuronStateBounds(const float lower, const float upper);
        
        unsigned int getNeuronCount() const;
        float getMaxWeight() const;
        float getMinWeight() const;
        float getMaxNeuralActivity() const;
        float getMinNeuralActivity() const;
        
        float getNetworkStateDeltaMag() const;
        
        void setNeuronState(const unsigned int index, float value);
        float getNeuronState(const unsigned int index);
        slBigVectorGSL* getNeuronStateVector();
        
        slBigVectorGSL* setInputNeurons(const unsigned offset, const unsigned length);
        slBigVectorGSL* getInputNeurons() const;
        int getInputNeuronCount() const;
        
        slBigVectorGSL* setOutputNeurons(const unsigned offset, const unsigned length);        
        slBigVectorGSL* getOutputNeurons() const;
        int getOutputNeuronCount() const;

        slBigMatrix2DGSL* getConnectionWeightMatrix();
        slBigMatrix2DGSL* getConnectionStructureMatrix();

        //void attachLearningAlgorithm(slNNLearningAlgorithm* newAlgorithm);
        
        void updateNeuronStates();
        void applyStructureToWeights();
        
        ~slNeuralNetwork();
        
    protected:

        slBigVectorGSL _previousNeuronStates;
        slBigVectorGSL _currentNeuronStates;
        slBigMatrix2DGSL _connectionWeights;
        slBigMatrix2DGSL _connectionStructure;

        slBigVectorGSL* _inputNeuronStates;
        slBigVectorGSL* _outputNeuronStates;
    
        slNNLearningAlgorithm* _learningAlgorithm;
    
        unsigned int _size;
        unsigned int _inputOffset;
        unsigned int _inputSize;
        unsigned int _outputOffset;
        unsigned int _outputSize;
        float _deltaTime;
        float _timeConstant;
        float _tauState;
        bool _boundStates;
        float _boundLower;
        float _boundUpper;
        
        friend class slNNHebbianAlgorithm;
        friend class slNNBCMAlgorithm;
        friend class slNNSupervisedHebbianAlgorithm;
        
    private:
                
};

/**
 *  A recurrent network has external inputs that drive it
 *  but no defined output nodes.  A subset of the networks
 *  nodes can be defined as outputs.
 */
class slRecurrentNetwork : public slNeuralNetwork {

    public:
    
        slRecurrentNetwork(const unsigned size, const unsigned inputSize);
        ~slRecurrentNetwork();

        slBigMatrix2DGSL* getInputConnectionWeightMatrix();
        slBigMatrix2DGSL* getInputConnectionStructureMatrix();

        void updateNeuronStates();
        void applyStructureToWeights();

        
    protected:

        slBigVectorGSL _externalInputNeuronStates;
        slBigVectorGSL _externalInputDelta;
        slBigMatrix2DGSL _inputConnectionWeights;
        slBigMatrix2DGSL _inputConnectionStructure;
        
        friend class slNNHebbianAlgorithm;
        friend class slNNBCMAlgorithm;
        friend class slNNSupervisedHebbianAlgorithm;

   private:
 
};

/**
 *  A feedforward network is one that can be defined to have input nodes
 *  layers and output nodes.  Note that the network is defined to have
 *  layers, so that the network can use layer based learning algorithms.
 *  NOTE: This implementation does not provide a complete implementation
 *  of the superclass NeuralNetwork because the full weight matrix is not
 *  normally initialized and used. *** this could change without warning ***
 */
class slFeedForwardNetwork: public slNeuralNetwork {

    public:

        slFeedForwardNetwork(const unsigned int size, const unsigned int inputSize, const unsigned int outputSize);

        unsigned int getInputNeuronCount();
        unsigned int getOutputNeuronCount();

        slBigVectorGSL* getInputNeurons();
        slBigVectorGSL* getOutputNeurons();
    
        ~slFeedForwardNetwork();
        
    protected:
        
        /**
         *  The vector of weight matrices for layer to layer connections.
         */
        std::vector<slBigMatrix2DGSL> _layerWeights;
    
    private:
        
        
};

 
#endif /* _BREVE_NEURAL_NET_H */
