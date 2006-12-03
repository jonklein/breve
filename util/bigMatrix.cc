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
 *  @file bigMatrix.cc
 *  @brief A large matrix manipulation class.
 *  @author Eric DeWitt
 *  Copyright (C) 2005 Eric DeWitt, Jonathan Klein
 */

#include "util.h"
#include "bigMatrix.hh"

/**
 * A custom GSL Error handler to trigger an abort in a breve simulation
 * without exiting the program.
 */

void slGSLErrorHandler( const char * reason, const char * file, int line, int gsl_errno ) {
	throw slException( std::string( "GSL Error" ) );
}

/**
 *  slVectorViewGSL vector length constructor.
 *
 *  @param length the length of the vector.
 */

slVectorViewGSL::slVectorViewGSL( const int length ) :   _vec( gsl_vector_float_alloc( length ) ) {
	_block 	= _vec->block;
	_dim 	= _vec->size;

	gsl_vector_float_set_zero( _vec );
}

/**
 *  slVectorViewGSL copy constructor.
 *
 *  @param other the number of elements in the matrix.
 */

slVectorViewGSL::slVectorViewGSL( const slVectorViewGSL& other ) :   _vec( gsl_vector_float_alloc( other._dim ) ) {
 	_block = _vec->block;
	_dim = other._dim;

	gsl_blas_scopy( other._vec, _vec );
}

/**
 *  slVectorViewGSL sub vector copy constructor.
 *
 *  @param other the number of elements in the matrix.
 */
slVectorViewGSL::slVectorViewGSL( const slVectorViewGSL& other, const int offset,  const int length ) : _vec( gsl_vector_float_alloc( other._dim ) ) {
	_block = _vec->block;
	_dim =  other._dim;
	gsl_blas_scopy( other._vec, _vec );
}

/**
 *  slVectorViewGSL destructor.
 *
 */
slVectorViewGSL::~slVectorViewGSL() {
	gsl_vector_float_free( _vec );
}

// inline
void slVectorViewGSL::copyData( const slVectorView& other ) {
	gsl_blas_scopy( other.getGSLVector(), this->getGSLVector() );
}

//inline
void slVectorViewGSL::setAll( const float value ) {
	gsl_vector_float_set_all( _vec, value );
}

/**
 *
 * clamp(low, tolerance, high)
 *
 * Clamps the values in the matrix to be between low and high.  If the value
 * is below tolerance, then it is set to low.
 * TODO: shouldn't this be if between high and high-tolerance -> high
 * and if between low and low+tolerance -> low?
 */

void slVectorViewGSL::clamp( const float low, const float tolerance, const float high ) {
	unsigned int x;

	for ( x = 0; x < static_cast<unsigned int>( _dim ); x++ ) {
		if ( _vec->data[x] > high ) {
			_vec->data[x] = high;
		} else if ( _vec->data[x] < tolerance ) {
			_vec->data[x] = low;
		}
	}
}


/**
 *  Returns the VectorView's vector elements as a gsl_vector
 *
 */
inline gsl_vector_float* slVectorViewGSL::getGSLVector() const {
	return _vec;
}

/**
 *  Returns the dimensionality of the vector.
 *
 */
inline unsigned int slVectorViewGSL::dim() const {
	return _dim;
}

/**
 *  Returns the sum of the elements in the vector.
 */
inline float slVectorViewGSL::sum() const {
	float result = 0;
	int i;

	for ( i = 0; i < _dim; i++ ) {
		result += _vec->data[i];
	}

	return result;

}

/**
 *  Returns the absolute sum of the elements in the vector.
 */
inline float slVectorViewGSL::absoluteSum() const {
	return gsl_blas_sasum( _vec );
}

/**
 *  Returns the max of the absolute value of the elements in the vector.
 */
inline float slVectorViewGSL::maxAbsolute() const {
	return gsl_vector_float_get( _vec, gsl_blas_isamax( _vec ) );
}

/**
 *  Returns the max of the elements in the vector.
 */
inline float slVectorViewGSL::max() const {
	return gsl_vector_float_max( _vec );
}

/**
 *  Returns the min of the elements in the vector.
 */
inline float slVectorViewGSL::min() const {
	return gsl_vector_float_min( _vec );
}

/**
 *  Multiples the vector with a scalar in place.
 */
slVectorViewGSL& slVectorViewGSL::inPlaceMultiply( const float scalar ) {
	gsl_blas_sscal( scalar, _vec );
	return *this;
}

/**
 *  Multiples the vector with a scalar in place.
 */
slVectorViewGSL& slVectorViewGSL::scale( const float scalar ) {

	this->inPlaceMultiply( scalar );
	return *this;
}

/**
 *  Multiples the vector with another vector in place.
 */
slVectorViewGSL& slVectorViewGSL::inPlaceMultiply( const slVectorView& other ) {
	// error checking needed

	gsl_vector_float_mul( this->getGSLVector(), other.getGSLVector() );
	return *this;
}

/**
 *  Add a scalar to each elements of a matrix in place.
 */
slVectorViewGSL& slVectorViewGSL::inPlaceAdd( const float scalar ) {
	// is there a BLAS version of this?
	gsl_vector_float_add_constant( this->getGSLVector(), scalar );
	return *this;
}

/**
 *  Add two equivilant dimension vectors in place.
 */
slVectorViewGSL& slVectorViewGSL::inPlaceAdd( const slVectorView& other ) {
	gsl_blas_saxpy( 1, other.getGSLVector(), this->getGSLVector() );
	return *this;
}

/**
 *  Add a matrix that has been scaled by a scalar (in-place, same dimensions).
 *
 *  @param scalar the value used to scale each element.
 */
slVectorViewGSL& slVectorViewGSL::inPlaceScaleAndAdd( const float scalar,
        const slVectorView& other ) {
	gsl_blas_saxpy( scalar, other.getGSLVector(), this->getGSLVector() );
	return *this;
}

/**
 *  dot product
 */
float slVectorViewGSL::dotProduct( const slVectorView& other ) const {
	float result = 0;
	gsl_blas_sdot( _vec, other.getGSLVector(), &result );
	return result;
}

/**
 *  outer product
 */
slBigMatrix2DGSL& slVectorViewGSL::outerProduct( const slVectorView& other ) const {
	int i, j;
	float  *v = _vec->data;
	float  *u = other.getGSLVector()->data;
	slBigMatrix2DGSL* result = new slBigMatrix2DGSL( _dim, other.dim() );

	for ( i = 0; i < _dim; i++ ) {
		for ( j = 0; j < other.dim(); j++ ) {
			result->_matrix->data[i * result->_matrix->tda + j] = ( v[i] * u[j] );
		}
	}

	return *result;
}

/**
 *  outer product
 */
slBigMatrix2DGSL& slVectorViewGSL::outerProductInto( const slVectorView& other, slBigMatrix2DGSL& result ) const {
	int i, j;
	float  *v = _vec->data;
	float  *u = other.getGSLVector()->data;

	if ( result._xdim == _dim && result._ydim == other.dim() ) {
		for ( i = 0; i < _dim; i++ ) {
			for ( j = 0; j < other.dim(); j++ ) {
				result._matrix->data[i * result._matrix->tda + j] = ( v[i] * u[j] );
			}
		}
	} else {
		throw slException( "slVectorViewGSL::outerProductInto: Dimension Mismatch" );
	}

	return result;
}

slVectorViewGSL& slVectorViewGSL::inPlaceAddVectorMatrixProduct( const float scalar, const slBigMatrix2DGSL& matrix, const slVectorViewGSL& other ) {

	gsl_blas_sgemv( CblasNoTrans, scalar, matrix._matrix, other.getGSLVector(), 1.0, this->getGSLVector() );
	return *this;

}

/**
 *  Addition operator (must have common dimensions).
 */
slVectorViewGSL& slVectorViewGSL::operator+( const slVectorView& other ) {
	slVectorViewGSL *tmp = new slVectorViewGSL( *this );
	gsl_blas_saxpy( 1, tmp->getGSLVector(), this->getGSLVector() );
	;
	return *tmp;
}

/**
 *  Addition operator (adds scalar to each element).
 */
slVectorViewGSL& slVectorViewGSL::operator+( const float scalar ) {
	slVectorViewGSL *tmp = new slVectorViewGSL( *this );
	gsl_vector_float_add_constant( tmp->getGSLVector(), scalar );
	return *tmp;
}

/**
 *  Vector element multiplication operator (must have identical dimensions).
 */
slVectorViewGSL& slVectorViewGSL::operator*( const slVectorView& other ) {
	slVectorViewGSL *tmp = new slVectorViewGSL( *this );
	gsl_vector_float_mul( other.getGSLVector(), tmp->getGSLVector() );
	return *tmp;
}

/**
 *  Vector element in-place multiplication.
 */
slVectorViewGSL& slVectorViewGSL::operator*( const float scalar ) {
	slVectorViewGSL *tmp = new slVectorViewGSL( *this );
	gsl_blas_sscal( scalar, tmp->getGSLVector() );
	return *tmp;
}

/**
 *  Vector euclidian norm.
 */
float slVectorViewGSL::magnitude() const {
	return gsl_blas_snrm2( _vec );
}

/**
 *  Convolve with general kernel (odd dimension).
 */
// slVectorViewGSL& slVectorViewGSL::convolve(const slVectorView& v) { }

/**
 *  Convolve periodic with general kernel (odd dimension).
*/
// slVectorViewGSL& slVectorViewGSL::convolvePeriodic(const slVectorView& v) { }

/**
 *  Convolve with general kernel via DFT (odd dimension).
 */
// slVectorViewGSL& slVectorViewGSL::convolveFFT(const slVectorView& v) { }

/**
 *  Convolve with 3x3 kernel (odd dimension).
 */
// slVectorViewGSL& slVectorViewGSL::convolve3x3(const slVectorView& v) { }

/**
 *  Base 1D matrix/vector class using GSL
 */
slBigVectorGSL::slBigVectorGSL( const int x )
		: slVectorViewGSL( x ) {
}

/**
 *
 */
slBigVectorGSL::slBigVectorGSL( const slBigVectorGSL& other )
		: slVectorViewGSL( static_cast<const slVectorViewGSL&>( other ) ) {
}

/**
 *
 */
slBigVectorGSL::slBigVectorGSL( const slVectorViewGSL& other, const int offset, const int length )
		: slVectorViewGSL( other, offset, length ) {
}

/**
 *
 *
slBigVectorGSL::slBigVectorGSL(gsl_vector_float* other)
    :   _vec(other),
        _block(_vec->block),
        _dim(other->size)
{

}
*/


slBigVectorGSL::~slBigVectorGSL() {}

/**
 *
 */
//inline
float slBigVectorGSL::get( const int inX ) const {
		if( *(unsigned int*)&_vec->data[ inX ] == 0x55555555 )
			printf(" Got 0x55 in matrix\n" );
		return _vec->data[ inX ];
	}

/**
 *
 */
//inline
void slBigVectorGSL::set( const int inX, const float value ) {
	if ( inX > _dim ) throw slException( "Vector index out of bounds" );

	_vec->data[ inX ] = value;
}


/**
 *  Base 2D matrix class using GSL
 */

slBigMatrix2DGSL::slBigMatrix2DGSL( const int x, const int y ) : slVectorViewGSL( x * y ) {
	_xdim = x;
	_ydim = y;
	_matrix = gsl_matrix_float_alloc_from_block( slVectorViewGSL::_vec->block, 0, _xdim, _ydim, _ydim );
	gsl_matrix_float_set_zero( _matrix );
}

slBigMatrix2DGSL::slBigMatrix2DGSL( const slBigMatrix2DGSL& other ) : slVectorViewGSL( static_cast<slVectorViewGSL>( other ) ) {
	_xdim =  other._xdim;
	_ydim =  other._ydim;

	_matrix = gsl_matrix_float_alloc_from_block( slVectorViewGSL::_vec->block, 0, _xdim, _ydim, _ydim );
	gsl_matrix_float_set_zero( _matrix );

}

slBigMatrix2DGSL::~slBigMatrix2DGSL() {

	gsl_matrix_float_free( _matrix );
}

inline unsigned int slBigMatrix2DGSL::xDim() const {
	return _xdim;
}

inline unsigned int slBigMatrix2DGSL::yDim() const {

	return _ydim;
}

/**
 *  get(x, y) - returns the matrix element at x,y without range checking
 *
 *  The basic slBigMatrix classes do not implement range checking.  When used
 *  in steve code, the range checking should occur there.  In the future a
 *  subclass with range checking can be provided.
 */
// inline
float slBigMatrix2DGSL::get( const int x, const int y ) const {
		if( *(unsigned int*)&_matrix->data[x * _matrix->tda + y]  == 0x55555555 ) 
			printf(" Got 0x55 in matrix\n" );

		return _matrix->data[x * _matrix->tda + y];
	}


/**
 *  set(x, y) - sets the matrix element at x,y without range checking
 *
 *  The basic slBigMatrix classes do not implement range checking.  When used
 *  in steve code, the range checking should occur there.  In the future a
 *  subclass with range checking can be provided.
 */
//inline
void slBigMatrix2DGSL::set( const int x, const int y, const float value ) {
	_matrix->data[x * _matrix->tda + y] = value;
}

slBigVectorGSL& slBigMatrix2DGSL::vectorMultiply( const slVectorViewGSL& vector ) const {

	slBigVectorGSL* result = new slBigVectorGSL( vector.dim() );
	gsl_blas_sgemv( CblasNoTrans, 1.0, _matrix, vector.getGSLVector(), 0.0, result->getGSLVector() );
	return *result;
}

slBigMatrix2DGSL& slBigMatrix2DGSL::vectorMultiplyInto( const slVectorViewGSL& sourceVector, slVectorViewGSL& resultVector ) {

	gsl_blas_sgemv( CblasNoTrans, 1.0, _matrix, sourceVector.getGSLVector(), 0.0, resultVector.getGSLVector() );
	return *this;
}

slBigMatrix2DGSL& slBigMatrix2DGSL::matrixMultiplyInto( const slBigMatrix2DGSL& sourceMatrix, slBigMatrix2DGSL& resultMatrix ) {

	gsl_blas_sgemm( CblasNoTrans, CblasNoTrans, 1.0, _matrix, sourceMatrix._matrix, 0.0, resultMatrix._matrix );
	return *this;
}

slBigMatrix2DGSL& slBigMatrix2DGSL::vectorMultiplyInto( const slVectorViewGSL& sourceVector, const float scalar, slVectorViewGSL& resultVector ) {

	gsl_blas_sgemv( CblasNoTrans, scalar, _matrix, sourceVector.getGSLVector(), 0.0, resultVector.getGSLVector() );
	return *this;
}

/*
slBigVectorGSL& slBigMatrix2DGSL::getRowVector(const int x)
{
    _gsl_vector_float_view *row = &gsl_matrix_float_row(_matrix, x);
    slBigVectorGSL *result = new slBigVectorGSL((gsl_vector_float *)row);
    return *result;
}
*/
float slBigMatrix2DGSL::getRowMagnitude( const int x ) {
	gsl_vector_float *row = &gsl_matrix_float_row( _matrix, x ).vector;
	return gsl_blas_snrm2( row );
}

slBigMatrix2DGSL& slBigMatrix2DGSL::inPlaceRowMultiply( const int x, const float scalar ) {

	gsl_vector_float *row = &gsl_matrix_float_row( _matrix, x ).vector;
	gsl_blas_sscal( scalar, row );
	return *this;
}

/*
slBigVectorGSL& slBigMatrix2DGSL::getColumnVector(const int y)
{
    return new slBigVectorGSL(&gsl_matrix_float_column(_matrix, y));
}
*/
float slBigMatrix2DGSL::getColumnMagnitude( const int y ) {
	gsl_vector_float *column = &gsl_matrix_float_column( _matrix, y ).vector;
	return gsl_blas_snrm2( column );
}

slBigMatrix2DGSL& slBigMatrix2DGSL::inPlaceColumnMultiply( const int y, const float scalar ) {

	gsl_vector_float *column = &gsl_matrix_float_column( _matrix, y ).vector;
	gsl_blas_sscal( scalar, column );
	return *this;

}



/*
slBigMatrix2DGSL& slBigMatrix2DGSL::inPlaceConvolve(const slBigMatrix2D& kernel)
{
    slBigMatrix2DGSL& kernelGSL = static_cast<slBigMatrix2DGSL&>(kernel);
    gsl_vector_float* kernelData = kernelGSL.getGSLVector()->data;
    unsigned int kernelTDA =  kernelGSL.getGSLVector()->tda;
    gsl_vector_float* thisData = this._vec->data;
    unsigned int thisTDA = this._vec->tda;
	unsigned int kernelXDim = kernelGSL->xDim();
	unsigned int kernelYDim = kernelGSL->yDim();
	unsigned int kernelXCenter = 0, kernelYCenter = 0;
    unsigned int x, y, xk, yk;
    float newVal = 0.0f;
    float oldVal = 0.0f;

    if ((kernelXDim % 2) > 0 || (kernelXDim % 2) > 0)
    {
        // throw not odd dimension exception
        return *this;
    }
    else
    {
        kernelXCenter = (kernelXDim / 2) + 1;
        kernelYCenter = (kernelYDim / 2) + 1;
        slBigBatrix2DGSL& convolutedGSL = new slBigMatrix2DGSL(this->_xDim, this->_yDim);
    }

    /////////// CURRENTLY USING INPLACE--SHOULD IT BE COPY?

    /////////// Need to correct multiplication in inner loop

    // loop unrolled to reduce tests for bounds
    // lower y
    for (y = 0; y < kernelYCenter - 1; y++)
    {
        // lower x
        for (x = 0; x < kernelXCenter - 1; x++)
        {
            // kernel mult
            for (xk = (kernelXCenter - x); xk < kernelXDim; xk++)
                for (yk = (kernelYCenter - y); yk < kernelYDim; yk++)
                {
                    newVal += kernelData[xk * kernelTDA + yk] * thisData[(x - xk) * thisTDA + (y - yk)];
                }

            thisData[x*thisTDA + y] = newValue;

        }
        // upper x
        for (x = this->xDim - (kernelXCenter - 1); x < this->xDim; x++)
        {
            // kernel mult
            for (xk = (kernelXCenter - (this->xDim - x)); xk < kernelXDim; xk++)
                for (yk = (kernelYCenter - y); yk < kernelYDim; yk++)
                {
                    newVal += kernelData[xk * kernelTDA + yk] * thisData[(x - (kernelXCenter - xk)) * thisTDA + (y - yk)];
                }

            thisData[x*thisTDA + y] = newValue;
        }
    }

    // upper y
    for (y = this->yDim - (kernelYcenter - 1); y < this->yDim; y++)
    {
        // lower x
        for (x = 0; x < kernelXCenter - 1; x++)
        {
            // kernel mult
            for (xk = (kernelXCenter - x); xk < kernelXDim; xk++)
                for (yk = (kernelYCenter - (this->yDim - y)); yk < kernelYDim; yk++)
                {
                    newVal += kernelData[xk * kernelTDA + yk] * thisData[(x - xk) * thisTDA + (y - (kernelYCenter - yk))];
                }

            thisData[x*thisTDA + y] = newValue;
        }
        // upper x
        for (x = this->xDim - (kernelXCenter - 1); x < this->xDim; x++)
        {
            // kernel mult
            for (xk = (kernelXCenter - (this->xDim - x)); xk < kernelXDim; xk++)
                for (yk = (kernelYCenter - (this->yDim - y)); yk < kernelYDim; yk++)
                {
                    newVal += kernelData[xk * kernelTDA + yk] * thisData[(x - (kernelXCenter - xk)) * thisTDA + (y - (kernelYCenter - yk))];

                }

            thisData[x*thisTDA + y] = newValue;
        }
    }

    for (y = kernelYCenter - 1; y < this->xDim - (kernelYCenter - 1); y++)
        for (x = kernelXCenter - 1; x < this->yDim - (kernelXCenter - 1); x++)
        {
            // kernel mult
            for (xk = 0; xk < kernelXDim; xk++)
                for (yk = (kernelYCenter - y); yk < kernelYDim; yk++)
                {
                    newVal += kernelData[xk * kernelTDA + yk] * thisData[(x - (xk - kernelXCenter)) * thisTDA + (y - (yk - kernelYCenter))];
                }

            thisData[x*thisTDA + y] = newValue;
        }

}
*/

/*
slBigMatrix2DGSL& slBigMatrix2DGSL::convolvePeriodic(const slBigMatrix2D& kernel)
{

}
*/

//slBigMatrix2DGSL& slBigMatrix2DGSL::convolve3x3(const slBigMatrix2D& kernel) {}

/**
 *  Base 3D matrix class using GSL
 */

slBigMatrix3DGSL::slBigMatrix3DGSL( const int x, const int y, const int z )
		: slVectorViewGSL(( x * y * z ) ),
		_xdim( x ),
		_ydim( y ),
		_zdim( z ) {
	unsigned int i = 0;

	_matrix = new gsl_matrix_float*[_zdim];

	for ( i = 0; i < _zdim; i++ ) {
		_matrix[ i ] = gsl_matrix_float_alloc_from_block( slVectorViewGSL::_vec->block, ( i * _xdim * _ydim ), _xdim, _ydim, _ydim );
		gsl_matrix_float_set_zero( _matrix[ i ] );
	}
}

slBigMatrix3DGSL::slBigMatrix3DGSL( const slBigMatrix3DGSL& other )

		: slVectorViewGSL( static_cast<slVectorViewGSL>( other ) ),
		_xdim( other._xdim ),
		_ydim( other._ydim ),
		_zdim( other._zdim ) {
	unsigned int i = 0;

	_matrix = new gsl_matrix_float*[_zdim];

	for ( i = 0; i < _zdim; i++ ) {
		_matrix[i] = gsl_matrix_float_alloc_from_block( slVectorViewGSL::_vec->block, ( i * _xdim * _ydim ), _ydim, _xdim, _xdim );
		gsl_matrix_float_set_zero( _matrix[ i ] );
	}
}

slBigMatrix3DGSL::~slBigMatrix3DGSL() {
	for ( unsigned int i = 0; i < _zdim; i++ ) gsl_matrix_float_free( _matrix[i] );
}

inline unsigned int slBigMatrix3DGSL::xDim() const {
	return _xdim;
}

inline unsigned int slBigMatrix3DGSL::yDim() const {

	return _ydim;
}

inline unsigned int slBigMatrix3DGSL::zDim() const {

	return _zdim;
}

// inline
float slBigMatrix3DGSL::get( const int x, const int y, const int z ) const {
		return ( _matrix[z] )->data[x *( _matrix[z] )->tda + y];
	}

//inline
void slBigMatrix3DGSL::set( const int x, const int y, const int z, const float value ) {
	( _matrix[z] )->data[x *( _matrix[z] )->tda + y] = value;
}

//slBigMatrix3DGSL& slBigMatrix3DGSL::convolve(const slBigMatrix3D& kernel) {}

//slBigMatrix3DGSL& slBigMatrix3DGSL::convolvePeriodic(const slBigMatrix3D& kernel) {}
