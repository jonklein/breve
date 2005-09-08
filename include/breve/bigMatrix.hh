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
 *
 * I think that the entire matrix/vector class structure for breve may be
 * replaced with a simpler, and more complete class system in the future.
 * Further, 3D matricies do not have many opperations defined for them 
 * (perhaps they shouldn't be called matrices and we should have array
 * classes over which matrix operations can be performed where appropraite.)
 * the goal of this class was largely to take advantage of optimizations
 * availible in the GSL/BLAS libraries for matrix operations.  Alternative
 * methods could achieve the same results.
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
 *  @todo   implement formated and unformated string output operators
 *  @todo   implement gsl file read and write functions
 *  @todo   implement matlab compatible binary file read and write functions
 */


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
         *  Copies the data block of other into the current vector.
         */
        virtual void copyData(const slVectorView& other) = 0;
        
        /**
         *  Returns the dimensionality of the vector.
         */
        virtual unsigned int dim() const = 0;

        /**
         *  Adds a scaled matrix (combined operations).
         */
        virtual slVectorView& inPlaceScaleAndAdd(const float scalar,
                                          const slVectorView& other) = 0;    

        /**
         *  Returns the  sum of the elements.
         */
        virtual float sum() const = 0;

        /**
         *  Returns the absolute sum of the elements.
         */
        virtual float absoluteSum() const = 0;

        /**
         *  Returns the min of the elements.
         */
        virtual float min() const = 0;

        /**
         *  Returns the max of the elements.
         */
        virtual float max() const = 0;

        /**
         *  Returns the max of the absolute value of the elements.
         */
        virtual float maxAbsolute() const = 0;
        
        /**
         *  Returns the magnitude of the vector.
         */
        virtual float magnitude() const = 0;

        /**
         *  set all values to a constant
         */
        virtual void setAll(const float value) = 0;
        
        /**
         *  keep all values in a range
         */
        virtual void clamp(const float low, const float tolerance, const float high) = 0;

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
         *  Convolve with general kernel (odd dimension).
         *  
         *  This method convolves this vector with another smaller
         *  vector of odd dimensionality.  This convolution is zero
         *  padded at the boundaries.
         */
        //virtual slVectorView& convolve(const slVectorView&) = 0;

        /**
         *  Convolve with general kernel (odd dimension).
         *  
         *  This method convolves this vector with another smaller
         *  vector of odd dimensionality.  This convolution is periodic
         *  at the boundaries.
         */
        //virtual slVectorView& convolvePeriodic(const slVectorView&) = 0;

        /**
         *  Convolve with general kernel (odd dimension).
         *  
         *  This method convolves this vector with another smaller
         *  vector of odd dimensionality.  This convolution is implemented
         *  by converting the vector to the frequency domain using a DFT.
         */
        //virtual slVectorView& convolveFFT(const slVectorView&) = 0;

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

        /**
         *  Vector dot product.
         */
        virtual float dotProduct(const slVectorView& other) const = 0;

        /**
         *  Vector outer product.
         */
        // cannot co-varry return for external class--until I think about this
        // it will just be declared in the sub-class
        // virtual slBigMatrix2D& outerProduct(const slVectorView& other) const = 0;

        virtual ~slVectorView() { }
        
        /**
         *  Returns the VectorView's vector elements as a gsl_vector
         *
         *  This is a short term solution to ensure that the gsl
         *  representation is availible to all classes that need
         *  it.  Down the road the interface/underlying data
         *  representation WILL change
         */
        virtual gsl_vector_float* getGSLVector() const = 0;
      
    // protected:
      
};

/**
 *  slBigVector is an abstract class that defines the interface for 1D
 *  matrix (vector) manipulations.
 *
 *  slBigVector is intended to be used for large vectors and not for
 *  internal 1x3 vectors or 1x4 vectors used in internal physics and
 *  graphics calculations where slVector should be used.
 */

class slBigVector {
    public:
    
        virtual float get(const int x) const = 0;

        virtual void set(const int x, const float value) = 0;

};

/**
 *  slBigMatrix is the super class for 2 and 3 dimensional matricies
 */
class slBigMatrix {
    
};

class slBigVectorGSL;   // forward declaration protection
class slVectorViewGSL;  // forward declaration protection
class slBigMatrix2DGSL; // forward declaration protection
/**
 *  slBigMatrix2D is an abstract class that defines the interface for 2D
 *  matrix manipulations.
 */
class slBigMatrix2D : public slBigMatrix {
	public:
	
        virtual unsigned int xDim() const = 0;
        
        virtual unsigned int yDim() const = 0;

/*
 *  virtual is apparently not supported by gcc 3.2 or 3.3
 */    
        virtual float get(const int x, const int y) const = 0;

        virtual void set(const int x, const int y, const float value) = 0;

        virtual slBigVectorGSL& vectorMultiply(const slVectorViewGSL& vector) const = 0;
        
        virtual slBigMatrix2DGSL& vectorMultiplyInto(const slVectorViewGSL& sourceVector, slVectorViewGSL& resultVector) = 0;
        
        virtual slBigMatrix2DGSL& vectorMultiplyInto(const slVectorViewGSL& sourceVector, const float scalar, slVectorViewGSL& resultVector) = 0;

/* ************* These methods will be implemented in the next release or sooner
        virtual slBigMatrix2D& inPlaceConvolve(const slBigMatrix2D& kernel) = 0;

        virtual slBigMatrix2D& inPlaceConvolvePeriodic(const slBigMatrix2D& kernel) = 0;

        virtual slBigMatrix2D& inPlaceConvolve3x3(const slBigMatrix2D& kernel) = 0;

        virtual slBigMatrix2D& convolve(const slBigMatrix2D& kernel) = 0;

        virtual slBigMatrix2D& convolvePeriodic(const slBigMatrix2D& kernel) = 0;

        virtual slBigMatrix2D& convolve3x3(const slBigMatrix2D& kernel) = 0;
*/

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

/*
 *  virtual is apparently not supported by gcc 3.2 or 3.3
 */
        virtual float get(int x, int y, int z) const = 0;

        virtual void set(int x, int y, int z, float value) = 0;
                
/*
*/
/* ************* These methods will be implemented in the next release or sooner
        virtual slBigMatrix3D& inPlaceConvolve(const slBigMatrix3D& kernel) = 0;

        virtual slBigMatrix3D& inPlaceConvolvePeriodic(const slBigMatrix3D& kernel) = 0;

        virtual slBigMatrix3D& convolve(const slBigMatrix3D& kernel) = 0;

        virtual slBigMatrix3D& convolvePeriodic(const slBigMatrix3D& kernel) = 0;
*/

};

// forward declare protection
class slBigMatrix2DGSL;

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
        slVectorViewGSL(const slVectorViewGSL& other);
        
        // TODO: this is a vew constructor--needs to be changed **********
		/**
		 *    Sub-Vector Copy constructor
		 */
        slVectorViewGSL(const slVectorViewGSL& other, const int offset, const int length);     

		/**
		 *    Vector View constructor
		 */
        //slVectorViewGSL(gsl_vector_float* other);     

		/**
		 *    Sub-Vector View constructor
		 */
        //slVectorViewGSL(gsl_vector_float* other, const int offset, const int stride, const int length);     
        
        /**
         *  Destructor
         */
        ~slVectorViewGSL();

        /**
         *  Copy method
         */
        void copyData(const slVectorView& other);

        /**
         *  Returns the dimensionality of the vector.
         */
        unsigned int dim() const;        
        
        /**
         *  set all values to a constant
         */
        void setAll (const float value);
        
        /**
         *  keep all values in a range
         */        
        void clamp(const float low, const float tolerance, const float high);

        /**
         *  Returns the sum of the elements.
         */
        float sum() const;

        /**
         *  Returns the absolute sum of the elements.
         */
        float absoluteSum() const;

        /**
         *  Returns the min of the elements.
         */
        float min() const;

        /**
         *  Returns the max of the absolute value of the elements.
         */
        float maxAbsolute () const;

        /**
         *  Returns the max of the elements.
         */
        float max() const;

        /**
         *  Returns the magnitude of the vector.
         */
        float magnitude() const;

        /**
         *  Add a scalar to each element in place.
         */
        slVectorViewGSL& inPlaceAdd(const float scalar);

        /**
         *  Multiply two equivilant dimension vectors in place.
         */
        slVectorViewGSL& inPlaceMultiply(const slVectorView& other);

        /**
         *  Adds a scaled vector view (combined operations).
         */
        slVectorViewGSL& inPlaceScaleAndAdd(const float scalar,
                                      const slVectorView& other);

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
         *  Vector dot product.
         */
        float dotProduct(const slVectorView& other) const;

        /**
         *  Vector outer product.
         */
        slBigMatrix2DGSL& outerProduct(const slVectorView& other) const;
        
        
        slBigMatrix2DGSL& outerProductInto(const slVectorView& other, slBigMatrix2DGSL& result) const;
        /**
         *  Convolve with general kernel (odd dimension).
         */
        // slVectorViewGSL& convolve(const slVectorView&);

        /**
         *  Convolve periodic with general kernel (odd dimension).
        */
        // slVectorViewGSL& convolvePeriodic(const slVectorView&);

        /**
         *  Convolve with general kernel via DFT (odd dimension).
         */
        // slVectorViewGSL& convolveFFT(const slVectorView&);

        /**
         *  Convolve with 3x3 kernel (odd dimension).
         */
        slVectorViewGSL& convolve3x3(const slVectorView&);

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
         *  Multiplication operator.
         */
        slVectorViewGSL& operator*(const slVectorView& other);

        /**
         *  In-place multiplication (must have identical dimensions).
         */
        slVectorViewGSL& operator*(const float scalar);
        
        /**
         *  Returns the VectorView's vector elements as a gsl_vector
         *
         *  This is a short term solution to ensure that the gsl
         *  representation is availible to all classes that need
         *  it.  Down the road the interface/underlying data
         *  representation WILL change
         */
        gsl_vector_float* getGSLVector() const;

    protected:
        
		gsl_vector_float* _vec;
		gsl_block_float* _block;
		bool _view;
		int _dim;
        
};

class slBigVectorGSL : public slBigVector, public slVectorViewGSL {
    public:
    
        slBigVectorGSL(const int x);
        
        slBigVectorGSL(const slBigVectorGSL& other);
        
        // TODO: this is a vew constructor--needs to be changed **********
        slBigVectorGSL(const slVectorViewGSL& other, const int offset, const int length);
        //slBigVectorGSL(gsl_vector_float* other);
        
        //slBigVectorGSL(gsl_vector_float* other, const int offset, const int stride, const int length);

        ~slBigVectorGSL();
        
        float get(const int x) const;
        
        void set(const int x, const float value);

};

class slBigMatrix2DGSL : public slBigMatrix2D, public slVectorViewGSL {
    public:
        
        slBigMatrix2DGSL(const int x, const int y);
                
		slBigMatrix2DGSL(const slBigMatrix2DGSL& other);

        ~slBigMatrix2DGSL();        

        unsigned int xDim() const;
        
        unsigned int yDim() const;

        float get(const int x, const int y) const;

        void set(const int x, const int y, const float value);
        
        //slBigVectorGSL& getRowVector(const int x);
        
        float getRowMagnitude(const int x);
        
        slBigMatrix2DGSL& inPlaceRowMultiply(const int x, const float scalar);
        
        //slBigVectorGSL& getColumnVector(const int y);
        
        float getColumnMagnitude(const int y);
        
        slBigMatrix2DGSL& inPlaceColumnMultiply(const int y, const float scalar);
        
        slBigVectorGSL& vectorMultiply(const slVectorViewGSL& vector) const;

        slBigMatrix2DGSL& vectorMultiplyInto(const slVectorViewGSL& sourceVector, slVectorViewGSL& resultVector);
        
        slBigMatrix2DGSL& vectorMultiplyInto(const slVectorViewGSL& sourceVector, const float scalar, slVectorViewGSL& resultVector);


/*        
        slBigMatrix2DGSL& inPlaceConvolve(const slBigMatrix2D& kernel);

        slBigMatrix2DGSL& convolvePeriodic(const slBigMatrix2D& kernel);

        slBigMatrix2DGSL& convolve3x3(const slBigMatrix2D& kernel);
*/       
    protected:
    
        unsigned int _xdim, _ydim;

		gsl_matrix_float *_matrix;
		
		friend class slVectorViewGSL;

};


class slBigMatrix3DGSL : public slBigMatrix3D, public slVectorViewGSL {
    public:
	
		slBigMatrix3DGSL(const int x, const int y, const int z);
		
		slBigMatrix3DGSL(const slBigMatrix3DGSL& other);
		
		~slBigMatrix3DGSL();

        unsigned int xDim() const;
        
        unsigned int yDim() const;
        
        unsigned int zDim() const;

        float get(const int x, const int y, const int z) const;

        void set(const int x, const int y, const int z, const float value);

/*        
        slBigMatrix3DGSL& convolve(const slBigMatrix3D& kernel);

        slBigMatrix3DGSL& convolvePeriodic(const slBigMatrix3D& kernel);
*/    
    protected:

		unsigned int _xdim, _ydim, _zdim;
		
		gsl_matrix_float **_matrix;
		
};

#endif
