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

/*
 *  @file bigMatrix.hh
 *  @brief A large matrix manipulation class.
 *  @Author Eric DeWitt
 *  Copyright (C) 2005 Eric DeWitt, Jonathan Klein
 */
 
#ifndef _BREVE_MATRIX_H
#define _BREVE_MATRIX_H

// for now we rely on libgsl--which will use either the unaccellerated blas
// or an accellerated

#include <gsl/gsl_matrix_float.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>

/*
 * for all operators and operations that are accelerated/benefit by in-place
 * manipulation, I am using in-place operations--even if this is not the
 * normal semantics for some of these operators.  I plan on potentially 
 * converting this to copy semantics so BE WARNED... but I'll announce it if
 * it occurs and replace the in-place operators with in-place methods.
 */

/**
 *  @file bigMatrix util classes
 *  
 *  These classes provide a basic structure for working with Matrices.
 *  Methods that can be applied to Matrix and Vectors rely on the
 *  slVectorView interface class.  The 2D and 3D matrix classes are
 *  currently implemented using the GNU Scientific Library which uses
 *  BLAS for underlying operations.  If an accelerated BLAS library is
 *  availible, it will be used.
 *
 *  *** IMPORTANT ***
 *  In the current implementation, which is a work in progress, the
 *  interface defines an implementation specific data access (should
 *  be protected) and also defines operators in the interface (maybe
 *  it shouldn't).  These are very much in flux
 *  @todo   bounds checking
 *  @todo   macstl implementation
 *  @todo   std implementation
 */

class slBigMatrix {
    
};

/**
 *  slVectorView is an abstract class that defines an interface for all
 *  vector and matrix dimension independant manipulations.
 *  
 *  This class is used by vector and matrix classes for applying all
 *  uniform operations (which for matricies requires that the memory
 *  layout/dimensions be identical). 
 */
class slVectorView {
    public:
        
        /**
         *  Returns the dimensionality of the vector.
         */
        virtual unsigned int dim() const = 0;

        /**
         *  Adds a scaled matrix (combined operations).
         */
        virtual slVectorView& scaleAndAdd(const float scalar,
                                          const slVectorView& other) = 0;    
        /**
         *  Returns the absolute sum of the elements in the Matrix.
         */
        virtual float absoluteSum() const = 0;

        /**
         *  Returns the min of the elements in the Matrix.
         */
        virtual float min() const = 0;

        /**
         *  Returns the max of the elements in the Matrix.
         */
        virtual float max() const = 0;

        /**
         *  Add a scalar to each elements of a matrix in place.
         */
        virtual slVectorView& inPlaceAdd(const float scalar) = 0;

        /**
         *  Add two equivilant dimension vectors in place.
         */
        virtual slVectorView& inPlaceAdd(const slVectorView& other) = 0;

       /**
         *  Multiply two equivilant dimension vectors in place.
         */
        virtual slVectorView& inPlaceMultiply(const slVectorView& other) = 0;

        /**
         *  Multiply each element by a scalar factgor in place.
         */
        virtual slVectorView& inPlaceMultiply(const float scalar) = 0;

        /**
         *  Matrix in-place addition operator (must have common dimensions).
         *  This is a proper matrix function.
         */
        virtual slVectorView& operator+(const slVectorView& other) = 0;
        
        /**
         *  Matrix in-place element addition operator (adds scalar to each element).
         */
        virtual slVectorView& operator+(const float scalar) = 0;
        
        /**
         *  Matrix multiplication operator.
         */
        virtual slVectorView& operator*(const slVectorView& other) = 0;

        /**
         *  Matrix in-place multiplication (must have identical dimensions).
         */
        virtual slVectorView& operator*(const float scalar) = 0;
                
        virtual ~slVectorView() { }
        
        /**
         *  Returns the VectorView's vector elements as a gsl_vector
         *
         *  This is a short term solution to ensure that the gsl
         *  representation is availible to all classes that need
         *  it.  Down the road the interface/underlying data
         *  representation WILL change
         */
        virtual inline gsl_vector_float* getGSLVector() const = 0;
      
    protected:
      
};

/**
 *  slBigVector is an abstract class that defines the interface for 1D
 *  matrix (vector) manipulations.
 *
 *  slBigVector is intended to be used for large vectors and not for
 *  internal 1x3 vectors or 1x4 vectors used in internal physics and
 *  graphics calculations where slVector should be used.
 */
class slBigVector : public slVectorView {
    public:
    
        virtual float get(const int x) const = 0;

        virtual void set(const int x, const float value) = 0;

};

/**
 *  slBigMatrix2D is an abstract class that defines the interface for 2D
 *  matrix manipulations.
 */
class slBigMatrix2D : public slBigMatrix {
	public:
	
        virtual unsigned int xDim() const = 0;
        
        virtual unsigned int yDim() const = 0;
        
        virtual float get(const int x, const int y) const = 0;

        virtual void set(const int x, const int y, const float value) = 0;

};

/**
 *  slBigMatrix3D is an abstract class that defines the interface for 3D
 *  matrix manipulations.
 */
class slBigMatrix3D : public slBigMatrix {
	public:
		
        virtual unsigned int xDim() const = 0;
        
        virtual unsigned int yDim() const = 0;
        
        virtual unsigned int zDim() const = 0;
        
        virtual float get(int x, int y, int z) const = 0;

        virtual void set(int x, int y, int z, float value) = 0;

};

/**
 *  slVectorViewGSL provides an implementation of the dimension
 *  intependent manipulations defined in slVectorView.
 *
 *  This class rebresents a view into a gsl_block which is a chunk of
 *  data allocated in (normally) a derived class or (alternatively) a
 *  sister object.  The gsl_vector struct is allocated in this class
 *  but the block allocation/deallocation is strictly NOT handled here.
 */ 
class slVectorViewGSL : public slVectorView {
	public:
		
		/**
		 *    Vector length constructor.
		 */
		slVectorViewGSL(const int length);
		
		/**
		 *    Copy constructor
		 */
        slVectorViewGSL(const slVectorViewGSL& source);

        /**
         *  Adds a scaled vector view (combined operations).
         */
        slVectorViewGSL& scaleAndAdd(const float scalar,
                                      const slVectorView& other);    
        /**
         *  Returns the dimensionality of the vector.
         */
        inline unsigned int dim() const;

        /**
         *  Returns the absolute sum of the elements.
         */
        float absoluteSum() const;

        /**
         *  Returns the min of the elements.
         */
        float min() const;

        /**
         *  Returns the max of the elements.
         */
        float max() const;

        /**
         *  Add a scalar to each element in place.
         */
        slVectorViewGSL& inPlaceAdd(const float scalar);

        /**
         *  Multiply two equivilant dimension vectors in place.
         */
        slVectorViewGSL& inPlaceMultiply(const slVectorView& other);

        /**
         *  Multiply each element by a scalar factor in place.
         */
        slVectorViewGSL& inPlaceMultiply(const float scalar);

        /** Scale by a scalar
         *
         */
        slVectorViewGSL& scale(const float scalar);

        /**
         *  Add two equivilant dimension vectors in place.
         */
        slVectorViewGSL& inPlaceAdd(const slVectorView& other);

        /**
         *  Addition operator (must have common dimensions).
         *  This is a proper matrix function.
         */
        slVectorViewGSL& operator+(const slVectorView& other);
        
        /**
         *  Addition operator (adds scalar to each element).
         */
        slVectorViewGSL& operator+(const float scalar);
        
        /**
         *  Matrix multiplication operator.
         */
        slVectorViewGSL& operator*(const slVectorView& other);

        /**
         *  Matrix in-place multiplication (must have identical dimensions).
         */
        slVectorViewGSL& operator*(const float scalar);

        ~slVectorViewGSL();
        
        /**
         *  Returns the VectorView's vector elements as a gsl_vector
         *
         *  This is a short term solution to ensure that the gsl
         *  representation is availible to all classes that need
         *  it.  Down the road the interface/underlying data
         *  representation WILL change
         */
        inline gsl_vector_float* getGSLVector() const;

    protected:
        
// doesnt work: friend gsl_vector_float* slVectorView::getGSLVector() const;
        
		gsl_vector_float* _vec;
		gsl_block_float* _block;
		int _dim;
        
};


class slBigMatrix2DGSL : public slBigMatrix2D, slVectorViewGSL {
    public:
        
        slBigMatrix2DGSL(int x, int y);
                
		slBigMatrix2DGSL(const slBigMatrix2DGSL& source);

        ~slBigMatrix2DGSL();        

        unsigned int xDim() const;
        
        unsigned int yDim() const;

        float get(const int x, const int y) const;

        void set(const int x, const int y, const float value);
        
    protected:
    
        unsigned int _xdim, _ydim;

		gsl_matrix_float *_matrix;

};


class slBigMatrix3DGSL : public slBigMatrix3D, slVectorViewGSL {
    public:
	
		slBigMatrix3DGSL(int x, int y, int z);
		
		slBigMatrix3DGSL(const slBigMatrix3DGSL& source);
		
		~slBigMatrix3DGSL();

        unsigned int xDim() const;
        
        unsigned int yDim() const;
        
        unsigned int zDim() const;

        float get(const int x, const int y, const int z) const;

        void set(const int x, const int y, const int z, const float value);
    
    protected:

		unsigned int _xdim, _ydim, _zdim;
		
		gsl_matrix_float **_matrix;
		
};

#endif
