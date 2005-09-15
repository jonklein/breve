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
 *  @file neuralNetworkLearningAlgorithm.hh
 *  @breif A neural network learning algorithm class.
 *  @author Eric DeWitt
 *  Copyright (C) 2005 Eric DeWitt, Jonathan Klein
 */

#include "neuralNetwork.hh"

class slNNLearningAlgorithm
{
    public:
        virtual ~slNNLearningAlgorithm();

        void setSynapseTimeStep(const float scalar);
        float getSynapseTimeStep() const;

        void setSynapseTimeConstant(const float scalar);
        float getSynapseTimeConstant() const;

        void setWeightBounds(const float lower, const float upper);
        
        float getWeightDeltaMagnitude() const;
        
        virtual void updateNetworkConnectionWeights() = 0;
        
    protected:

        float _deltaTime;
        float _timeConstant;
        float _tauWeight;
        bool _boundWeights;
        float _boundLower;
        float _boundUpper;
        long int _iterationCount;
        
        float _weightDeltaMag;

    private:


};

/**
 *
 *  Unsupervised Hebbian learning
 *
 */
class slNNHebbianAlgorithm : public slNNLearningAlgorithm
{

    public:
        slNNHebbianAlgorithm(slNeuralNetwork& theNetwork);
        slNNHebbianAlgorithm(slRecurrentNetwork& theNetwork);
        ~slNNHebbianAlgorithm();

        void setNormalizeOff();
        void setNormalizeSubtractive();
        void setNormalizeMultiplicitive();
        void setNormalizeVectorMagnitude();
        void setNormalizeAlpha(const float scalar);

        void updateNetworkConnectionWeights();
        
    private:
        
        enum slWeightsNormalizationMethod { VECLENGTH, MULTIPLIC, SUBTRACT };

        slNeuralNetwork* _network;
        slRecurrentNetwork* _recurrentNetwork;

        bool _recurrent;
        
        struct slWeightDeltas {
            slBigMatrix2DGSL* _input;
            slBigMatrix2DGSL* _inputB;
            slBigVectorGSL* _inputUnity;
            slBigMatrix2DGSL* _recurrent;
            slBigMatrix2DGSL* _recurrentB;
            slBigVectorGSL* _recurrentUnity;
        } _connectionWeightsDelta;

        bool _norm;
        slWeightsNormalizationMethod _normMethod;
        float _alpha;

};

class slNNBCMAlgorithm : public slNNHebbianAlgorithm
{
    public:
        slNNBCMAlgorithm(slNeuralNetwork& theNetwork);
        slNNBCMAlgorithm(slRecurrentNetwork& theNetwork);
        ~slNNBCMAlgorithm();
        
        void updateNetworkConnectionWeights();

    private:
        std::vector<float> _thresholds;
};

/**
 *
 *  Supervised Hebbian learning
 *
 */
class slNNSupervisedHebbianAlgorithm : public slNNLearningAlgorithm
{

    public:
        slNNSupervisedHebbianAlgorithm(slNeuralNetwork& theNetwork);
        slNNSupervisedHebbianAlgorithm(slRecurrentNetwork& theNetwork);
        ~slNNSupervisedHebbianAlgorithm();
        
        void updateNetworkConnectionWeights();
        
    private:

        float lastError;
        float leastError;

};

/**
 *
 *  Supervised generalize back-propagation learning
 *
 */
class slNNBackPropagationAlgorithm : public slNNLearningAlgorithm
{

    public:
        slNNBackPropagationAlgorithm(slFeedForwardNetwork& theNetwork);
        ~slNNBackPropagationAlgorithm();
        
        void updateNetworkConnectionWeights();
        
    private:

        float lastError;
        float leastError;

};