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
 *  @file slNeuralNetwork.cc
 *  @breif A neural network manipulation class.
 *  @author Eric DeWitt
 *  Copyright (C) 2005 Eric DeWitt, Jonathan Klein
 */


#include "simulation.h"
#include <string>
#include <iostream>
#include "neuralNetwork.hh"
#include "bigMatrix.hh"

//#include "neuralNetworkLearningAlgorithm.hh"

// using forward declares to avoid dep proliferation

class slNNLearningAlgorithm;

/**
 *  slNeuralNetwork base class constructor
 *
 *
 *  @param size the number of neurons in the network
 */

slNeuralNetwork::slNeuralNetwork( const unsigned size )
		:   _previousNeuronStates( size ),
		_currentNeuronStates( size ),
		_connectionWeights( size, size ),
		_connectionStructure( size, size ),
		_inputOffset( 0 ),
		_inputSize( 0 ),
		_outputOffset(( size - 1 ) ),
		_outputSize( 0 ),
		_deltaTime( 1.0 ),
		_timeConstant( 1.0 ),
		_tauState(( _deltaTime *( 1.0 / _timeConstant ) ) ),
		_boundStates( false ),
		_boundLower( 0.0 ),
		_boundUpper( 0.0 ) {
	_inputNeuronStates = 0;
	_outputNeuronStates = 0;
}

void slNeuralNetwork::setNeuronTimeStep( const float scalar ) {

	_deltaTime = scalar;
	// we combine deltaTime and (1 / _timeConstant) for efficiency
	_tauState = ( _deltaTime * ( 1.0 / _timeConstant ) );
}

float slNeuralNetwork::getNeuronTimeStep() const {

	return _deltaTime;
}

void slNeuralNetwork::setNeuronTimeConstant( const float scalar ) {

	_timeConstant = scalar;
	// we combine deltaTime and (1 / _timeConstant) for efficiency
	_tauState = ( _deltaTime * ( 1.0 / _timeConstant ) );
}

float slNeuralNetwork::getNeuronTimeConstant() const {

	return _timeConstant;
}

void slNeuralNetwork::setNeuronStateBounds( const float lower, const float upper ) {

	if ( lower == upper ) {
		_boundStates = false;
		_boundLower = 0.0;
		_boundUpper = 0.0;
	} else {
		_boundStates = true;
		_boundLower = lower;
		_boundUpper = upper;
	}
}

unsigned int slNeuralNetwork::getNeuronCount() const {

	return _currentNeuronStates.dim();
}

float slNeuralNetwork::getMaxWeight() const {

	return _connectionWeights.max();
}

float slNeuralNetwork::getMinWeight() const {

	return _connectionWeights.min();
}

float slNeuralNetwork::getMaxNeuralActivity() const {

	return _currentNeuronStates.max();
}

float slNeuralNetwork::getMinNeuralActivity() const {

	return _currentNeuronStates.min();
}

float slNeuralNetwork::getNetworkStateDeltaMag() const {

	float result = 0.0;
	result = _currentNeuronStates.magnitude();
	result -= _previousNeuronStates.magnitude();
	return result;
}

void slNeuralNetwork::setNeuronState( const unsigned index, float value ) {

	_currentNeuronStates.set( index, value );
}

float slNeuralNetwork::getNeuronState( const unsigned index ) {

	return _currentNeuronStates.get( index );
}

slBigVectorGSL* slNeuralNetwork::getNeuronStateVector() {

	return &_currentNeuronStates;
}

slBigVectorGSL* slNeuralNetwork::setInputNeurons( const unsigned offset, const unsigned length ) {
	if ((( offset + length ) > this->getNeuronCount() ) || ( offset < 0 ) ) throw slException( std::string( "Input neurons out of bounds." ) );

	if (( offset > ( _outputOffset + _outputSize ) ) || ( offset + length ) <= _outputOffset ) {
		_inputNeuronStates = new slBigVectorGSL( static_cast<slVectorViewGSL>( _currentNeuronStates ), offset, length );
		return _inputNeuronStates;
	} else {
		throw slException( std::string( "Input neurons must not overlap output neurons." ) );
	}
}

slBigVectorGSL* slNeuralNetwork::getInputNeurons() const {

	return _inputNeuronStates;
}

int slNeuralNetwork::getInputNeuronCount() const {

	if ( _inputNeuronStates ) return _inputNeuronStates->dim();
	else return 0;
}

slBigVectorGSL* slNeuralNetwork::setOutputNeurons( const unsigned offset, const unsigned length ) {

	if ((( offset + length ) > this->getNeuronCount() ) || ( offset < ( unsigned )0 ) ) throw slException( std::string( "Output neurons out of bounds." ) );

	if (( offset > ( _inputOffset + _inputSize ) ) || ( offset + length ) <= _inputOffset ) {
		_outputNeuronStates = new slBigVectorGSL( static_cast<slVectorViewGSL>( _currentNeuronStates ), offset, length );
		return _outputNeuronStates;
	} else {
		throw slException( std::string( "Output neurons must no overlap output neurons." ) );
	}
}

slBigVectorGSL* slNeuralNetwork::getOutputNeurons() const {

	return _outputNeuronStates;
}

int slNeuralNetwork::getOutputNeuronCount() const {

	if ( _outputNeuronStates ) return _outputNeuronStates->dim();
	else return 0;
}

slBigMatrix2DGSL* slNeuralNetwork::getConnectionWeightMatrix() {

	return &_connectionWeights;
}

slBigMatrix2DGSL* slNeuralNetwork::getConnectionStructureMatrix() {
	return &_connectionStructure;
}

/* This was for a more complex visitor pattern that might be implemented later
void slNeuralNetwork::attachLearningAlgorithm(slNNLearningAlgorithm* newAlgorithm)
{
    _learningAlgorithm = newAlgorithm;
}
*/

void slNeuralNetwork::applyStructureToWeights() {
	_connectionWeights.inPlaceMultiply( _connectionStructure );
}

void slNeuralNetwork::updateNeuronStates() {
//    int i, j;

//    slMessage(DEBUG_ALL, "_tauState is %g.\n", _tauState);

	_previousNeuronStates.copyData( _currentNeuronStates );

//    slMessage(DEBUG_ALL, "v(t-1) state of neuron 0 is %f.\n", _previousNeuronStates.get(0));
//    slMessage(DEBUG_ALL, "v(t) state of neuron 0 is %f.\n", _currentNeuronStates.get(0));

	// update according to tr * (dV[] / dt) = -V[] + (M[][]V[])
//    slMessage(DEBUG_ALL, "Weights:\n");
//    for (i = 0; i < _connectionWeights.xDim(); i++)
//    {
//        for (j = 0; j < _connectionWeights.yDim(); j++)
//            slMessage(DEBUG_ALL, "%f ", _connectionWeights.get(i, j));
//        slMessage(DEBUG_ALL, "\n");
//    }
	_connectionWeights.vectorMultiplyInto( _previousNeuronStates, _tauState, _currentNeuronStates );

//    slMessage(DEBUG_ALL, "After Mv(t-w), v(t) for neuron 0 is %f.\n", _currentNeuronStates.get(0));

	_currentNeuronStates.inPlaceScaleAndAdd(( -1.0 * _tauState ), _previousNeuronStates );

//    slMessage(DEBUG_ALL, "After -_tauState * v(t-1), v(t) for neuron 0 is %f.\n", _currentNeuronStates.get(0));

	_currentNeuronStates.inPlaceAdd( _previousNeuronStates );

//    slMessage(DEBUG_ALL, "After +v(t-1), v(t) for neuron 0 is %f.\n", _currentNeuronStates.get(0));

	if ( _boundStates ) _currentNeuronStates.clamp( _boundLower, _boundLower, _boundUpper );

}

slNeuralNetwork::~slNeuralNetwork() {
}

/**
 * slRecurrentNetwork
 */

slRecurrentNetwork::slRecurrentNetwork( const unsigned size, const unsigned inputSize )
		:   slNeuralNetwork( size ),
		_externalInputNeuronStates( inputSize ),
		_externalInputDelta( inputSize ),
		_inputConnectionWeights( inputSize, size ),
		_inputConnectionStructure( inputSize, size ) {
	slNeuralNetwork::_inputNeuronStates = &_externalInputNeuronStates;
}

slBigMatrix2DGSL* slRecurrentNetwork::getInputConnectionWeightMatrix() {

	return &_inputConnectionWeights;
}

slBigMatrix2DGSL* slRecurrentNetwork::getInputConnectionStructureMatrix() {
	return &_inputConnectionStructure;
}

void slRecurrentNetwork::applyStructureToWeights() {
	_connectionWeights.inPlaceMultiply( _connectionStructure );
	_inputConnectionWeights.inPlaceMultiply( _inputConnectionStructure );
}

void slRecurrentNetwork::updateNeuronStates() {
	_previousNeuronStates.copyData( _currentNeuronStates );

	// update according to tr * (dV[] / dt) = -V[] + (M[][]V[])

	_inputConnectionWeights.vectorMultiplyInto( _externalInputNeuronStates, _tauState, _externalInputDelta );
	_connectionWeights.vectorMultiplyInto( _previousNeuronStates, _tauState, _currentNeuronStates );
	_currentNeuronStates.inPlaceScaleAndAdd( 1.0, _externalInputDelta );
	_currentNeuronStates.inPlaceScaleAndAdd(( -1.0 * _tauState ), _previousNeuronStates );

	_currentNeuronStates.inPlaceAdd( _previousNeuronStates );

	if ( _boundStates ) _currentNeuronStates.clamp( _boundLower, _boundLower, _boundUpper );

}

slRecurrentNetwork::~slRecurrentNetwork() {
}

/**
 * slFeedForwardNetwork
 */

slFeedForwardNetwork::slFeedForwardNetwork( const unsigned size, const unsigned inputSize, const unsigned outputSize )
		: slNeuralNetwork( size ) {
}

slFeedForwardNetwork::~slFeedForwardNetwork() {

}


