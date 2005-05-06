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
 *  @breif A large matrix manipulation class.
 *  @author Eric DeWitt
 *  Copyright (C) 2005 Eric DeWitt, Jonathan Klein
 */

#include "bigMatrix.hh"

/**
 *  slVectorViewGSL vector length constructor.
 *
 * 
 *  @param length the length of the vector.
 */
slVectorViewGSL::slVectorViewGSL(const int length)
    :   _vec(gsl_vector_float_alloc (length)),
        _block(_vec->block),
        _dim(_vec->size)
{
    gsl_vector_float_set_zero(_vec);
}

/**
 *  slVectorViewGSL copy constructor.
 * 
 *  @param source the number of elements in the matrix.
 */
slVectorViewGSL::slVectorViewGSL(const slVectorViewGSL& source)
    :   _vec(gsl_vector_float_alloc(source._dim)),
        _block(_vec->block),
        _dim(source._dim)
{
    gsl_blas_scopy(source._vec, _vec);
}

/**
 *  slVectorViewGSL destructor.
 * 
 */
slVectorViewGSL::~slVectorViewGSL()
{
    gsl_vector_float_free(_vec);
}

// inline 
void slVectorViewGSL::copy(const slVectorViewGSL& source)
{
    gsl_blas_scopy(source._vec, _vec);
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
 *  Returns the absolute sum of the elements in the vector.
 */
inline float slVectorViewGSL::absoluteSum() const
{
    return gsl_blas_sasum(_vec);
}

/**
 *  Returns the max of the elements in the vector.
 */
inline float slVectorViewGSL::max() const
{
    return gsl_blas_isamax(_vec);
}

/**
 *  Returns the min of the elements in the vector.
 */
inline float slVectorViewGSL::min() const
{
    return gsl_vector_float_min(_vec);
}

/**
 *  Multiples the vector with a scalar in place.
 */
slVectorViewGSL& slVectorViewGSL::inPlaceMultiply(const float scalar)
{ 
    gsl_blas_sscal(scalar, _vec);
    return *this;
}

/**
 *  Multiples the vector with a scalar in place.
 */
slVectorViewGSL& slVectorViewGSL::scale(const float scalar)
{
    
    this->inPlaceMultiply(scalar);
    return *this;
}

/**
 *  Multiples the vector with another vector in place.
 */
slVectorViewGSL& slVectorViewGSL::inPlaceMultiply(const slVectorView& other)
{
	// error checking needed
	
    gsl_vector_float_mul(this->getGSLVector(), other.getGSLVector());
    return *this;
}

/**
 *  Add a scalar to each elements of a matrix in place.
 */
slVectorViewGSL& slVectorViewGSL::inPlaceAdd(const float scalar)
{
    // is there a BLAS version of this?
    gsl_vector_float_add_constant(this->getGSLVector(), scalar);
    return *this;
}

/**
 *  Add two equivilant dimension vectors in place.
 */
slVectorViewGSL& slVectorViewGSL::inPlaceAdd(const slVectorView& other)
{
    gsl_blas_saxpy(1, other.getGSLVector(), this->getGSLVector());
    return *this;
}

/**
 *  Add a matrix that has been scaled by a scalar (in-place, same dimensions).
 * 
 *  @param scalar the value used to scale each element.
 */
slVectorViewGSL& slVectorViewGSL::scaleAndAdd(const float scalar, 
                                              const slVectorView& other)
{
    gsl_blas_saxpy(scalar, other.getGSLVector(), this->getGSLVector());
    return *this;
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
 *  Addition operator (must have common dimensions).
 */
slVectorViewGSL& slVectorViewGSL::operator+(const slVectorView& other)
{
    slVectorViewGSL *tmp = new slVectorViewGSL(*this);
    gsl_blas_saxpy(1, tmp->getGSLVector(), this->getGSLVector());;
    return *tmp;
}

/**
 *  Addition operator (adds scalar to each element).
 */
slVectorViewGSL& slVectorViewGSL::operator+(const float scalar)
{
    slVectorViewGSL *tmp = new slVectorViewGSL(*this);
    gsl_vector_float_add_constant(tmp->getGSLVector(), scalar);
    return *tmp;
}

/**
 *  Vector element multiplication operator (must have identical dimensions).
 */
slVectorViewGSL& slVectorViewGSL::operator*(const slVectorView& other)
{
    slVectorViewGSL *tmp = new slVectorViewGSL(*this);
    gsl_vector_float_mul(other.getGSLVector(), tmp->getGSLVector());
    return *tmp;
}

/**
 *  Vector element in-place multiplication.
 */
slVectorViewGSL& slVectorViewGSL::operator*(const float scalar)
{
    slVectorViewGSL *tmp = new slVectorViewGSL(*this);
    gsl_blas_sscal(scalar, tmp->getGSLVector());
    return *tmp;
}


/**
 *  Base 1D matrix/vector class using GSL
 */

slBigVectorGSL::slBigVectorGSL(const int x)
    : slVectorViewGSL(x)
{

}

slBigVectorGSL::slBigVectorGSL(const slBigVectorGSL& source)
    : slVectorViewGSL(static_cast<slVectorViewGSL>(source))
{

}

slBigVectorGSL::~slBigVectorGSL() {}

inline unsigned int slBigVectorGSL::dim()
{
	return 0;
}

inline float slBigVectorGSL::get(const int x)
{
	return _vec->data[x];
}

inline void slBigVectorGSL::set(const int x, const float value)
{
	_vec->data[x] = value;
}

/**
 *  Base 2D matrix class using GSL
 */

slBigMatrix2DGSL::slBigMatrix2DGSL(const int x, const int y) 
    : slVectorViewGSL((x * y)),
    _xdim (x),
    _ydim (y)
{
    _matrix = gsl_matrix_float_alloc_from_block(slVectorViewGSL::_vec->block, 0, _ydim, _xdim, _xdim);
}

slBigMatrix2DGSL::slBigMatrix2DGSL(const slBigMatrix2DGSL& source) 
    : slVectorViewGSL(static_cast<slVectorViewGSL>(source)),
    _xdim (source._xdim),
    _ydim (source._ydim)
{
    
    _matrix = gsl_matrix_float_alloc_from_block(slVectorViewGSL::_vec->block, 0, _ydim, _xdim, _xdim);
    
}

slBigMatrix2DGSL::~slBigMatrix2DGSL()
{
    gsl_matrix_float_free(_matrix);
}

inline unsigned int slBigMatrix2DGSL::xDim() const
{
    return _xdim;
} 

inline unsigned int slBigMatrix2DGSL::yDim() const
{
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
float slBigMatrix2DGSL::get(const int x, const int y) const
{
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
 void slBigMatrix2DGSL::set(const int x, const int y,const float value)
{
    _matrix->data[x * _matrix->tda + y] = value;
}

/**
 *  set(x, y) - sets the matrix element at x,y without range checking
 *
 *  The basic slBigMatrix classes do not implement range checking.  When used
 *  in steve code, the range checking should occur there.  In the future a
 *  subclass with range checking can be provided.
 */ 
//inline
 void slBigMatrix2DGSL::setAll(const float value)
{
	gsl_matrix_float_set_all( _matrix, value);
}

/**
 * 
 * clamp(low, tolerance, high)
 *
 * Clamps the values in the matrix to be between low and high.  If the value is below 
 * tolerance, then it is set to low.
 */

void slBigMatrix2DGSL::clamp(const float low, const float tolerance, const float high) {
	unsigned int x, y;

	for(x = 0; x < _xdim; x++ ) {
		for(y = 0; y < _ydim; y++ ) {
			if( _matrix->data[x * _matrix->tda + y] > high) {
			 	_matrix->data[x * _matrix->tda + y] = high;
			} else if( _matrix->data[x * _matrix->tda + y] < tolerance) {
				_matrix->data[x * _matrix->tda + y] = low;
			}
		}
	}
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

slBigMatrix3DGSL::slBigMatrix3DGSL(const int x, const int y, const int z) 
    : slVectorViewGSL((x * y * z)),
    _xdim (x),
    _ydim (y),
    _zdim (z)
{
    unsigned int i = 0;
    
    _matrix = new gsl_matrix_float*[_zdim];
    for (i = 0; i < _zdim; i++) {
        _matrix[i] = gsl_matrix_float_alloc_from_block(slVectorViewGSL::_vec->block, (i * _xdim * _ydim), _ydim, _xdim, _xdim);
    }
}

slBigMatrix3DGSL::slBigMatrix3DGSL(const slBigMatrix3DGSL& source) 
    : slVectorViewGSL(static_cast<slVectorViewGSL>(source)),
    _xdim (source._xdim),
    _ydim (source._ydim),
    _zdim (source._zdim)
{
    unsigned int i = 0;
    
    _matrix = new gsl_matrix_float*[_zdim];
    for (i = 0; i < _zdim; i++) {
        _matrix[i] = gsl_matrix_float_alloc_from_block(slVectorViewGSL::_vec->block, (i * _xdim * _ydim), _ydim, _xdim, _xdim);
    }
}

slBigMatrix3DGSL::~slBigMatrix3DGSL()
{
    for (unsigned int i = 0; i < _zdim; i++)
        gsl_matrix_float_free(_matrix[i]);
}

inline unsigned int slBigMatrix3DGSL::xDim() const
{
    return _xdim;
} 

inline unsigned int slBigMatrix3DGSL::yDim() const
{
    return _ydim;
} 

inline unsigned int slBigMatrix3DGSL::zDim() const
{
    return _zdim;
} 

// inline 
float slBigMatrix3DGSL::get(const int x, const int y, const int z) const
{
    return (_matrix[z])->data[x * (_matrix[z])->tda + y];
} 

//inline 
void slBigMatrix3DGSL::set(const int x, const int y, const int z, const float value)
{
    (_matrix[z])->data[x * (_matrix[z])->tda + y] = value;
}

//inline 
void slBigMatrix3DGSL::setAll(const float value)
{
	for( unsigned int n = 0; n < _zdim; n++ ) gsl_matrix_float_set_all( _matrix[n], value);
}

//slBigMatrix3DGSL& slBigMatrix3DGSL::convolve(const slBigMatrix3D& kernel) {}

//slBigMatrix3DGSL& slBigMatrix3DGSL::convolvePeriodic(const slBigMatrix3D& kernel) {}
