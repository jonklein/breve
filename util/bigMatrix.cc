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
float slVectorViewGSL::absoluteSum() const
{
    return gsl_blas_sasum(_vec);
}

/**
 *  Returns the max of the elements in the vector.
 */
float slVectorViewGSL::max() const
{
    return gsl_blas_isamax(_vec);
}

/**
 *  Returns the min of the elements in the vector.
 */
float slVectorViewGSL::min() const
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
 *  Matrix multiplication operator.
 */
slVectorViewGSL& slVectorViewGSL::operator*(const slVectorView& other)
{
    slVectorViewGSL *tmp = new slVectorViewGSL(*this);
    gsl_vector_float_mul(other.getGSLVector(), tmp->getGSLVector());
    return *tmp;
}

/**
 *  Matrix in-place multiplication (must have identical dimensions).
 */
slVectorViewGSL& slVectorViewGSL::operator*(const float scalar)
{
    slVectorViewGSL *tmp = new slVectorViewGSL(*this);
    gsl_blas_sscal(scalar, tmp->getGSLVector());
    return *tmp;
}

/**
 *  Base 2D matrix class using GSL
 */

slBigMatrix2DGSL::slBigMatrix2DGSL(const int x, const int y) 
    : slVectorViewGSL((x * y)),
    _xdim (x),
    _ydim (y)
{
    _matrix = gsl_matrix_float_alloc_from_block(slVectorViewGSL::_vec->block, 0, _xdim, _ydim, _xdim);
}

slBigMatrix2DGSL::slBigMatrix2DGSL(const slBigMatrix2DGSL& source) 
    : slVectorViewGSL(static_cast<slVectorViewGSL>(source)),
    _xdim (source._xdim),
    _ydim (source._ydim)
{
    
    _matrix = gsl_matrix_float_alloc_from_block(slVectorViewGSL::_vec->block, 0, _xdim, _ydim, _xdim);
    
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

inline float slBigMatrix2DGSL::get(int x, int y) const
{
    return gsl_matrix_float_get(_matrix, x, y);
} 

inline void slBigMatrix2DGSL::set(int x, int y, float value)
{
    gsl_matrix_float_set(_matrix, x, y, value);
}

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
        _matrix[i] = gsl_matrix_float_alloc_from_block(slVectorViewGSL::_vec->block, (i * _zdim), _xdim, _ydim, _xdim);
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
        _matrix[i] = gsl_matrix_float_alloc_from_block(slVectorViewGSL::_vec->block, (i * _zdim), _xdim, _ydim, _xdim);
    }
}

slBigMatrix3DGSL::~slBigMatrix3DGSL()
{
    int i = 0;
    for (i = 0; i < _zdim; i++) {
        gsl_matrix_float_free(_matrix[i]);
    }
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

inline float slBigMatrix3DGSL::get(int x, int y, int z) const
{
    return gsl_matrix_float_get(_matrix[z], x, y);
} 

inline void slBigMatrix3DGSL::set(int x, int y, int z, float value)
{
    gsl_matrix_float_set(_matrix[z], x, y, value);
}

