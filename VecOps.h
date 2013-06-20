//VecOps.h
// Some 3 component vector operations.
// Sam Carton 31510651
// Created 29 April 2012
// Last Modified 21 May 2012

// Vectors are assumed to be GLdouble arrays of size 3, with components x, y, and z in that order.


#ifndef VECOPS_H
#define VECOPS_H

#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>

///
/// vecAdd
/// \brief Add vec1 and vec2 vectors, store in vecSum vector. vecSum can also be one of vec1 or vec2.
/// \param vec1 first vector to sum
/// \param vec2 second vector to sum
/// \param vecSum vector to store result into
///
void vecAdd(const GLdouble* vec1, const GLdouble* vec2, GLdouble* vecSum);

///
/// vecScale
/// \brief Scales an array of 3 GLdoubles (vector) by a scale factor (scale)
/// \param vec vector to scale
/// \param scale the scale factor
/// \param dest vector to store scaled result into
///
void vecScale(GLdouble* vec, GLdouble scale, GLdouble* dest);

///
/// vecMinus
/// \brief V1-V2=Vresult
/// \param vec1 first vector
/// \param vec2 vector to minus from first vector
/// \param vecRes vector to store result into
///
void vecMinus(const GLdouble* vec1, const GLdouble* vec2, GLdouble* vecRes);

///
/// vecMag
/// \brief calculates vector magnitude
/// \param vec vector to compute magnitude of
/// \retval GLdouble returns the input vector's magnitude (absolute value)
///
GLdouble vecMag(const GLdouble* vec);

///
/// vecCopy
/// \brief copy vector from one variable to another
/// \param source vector to copy
/// \param destination vector to copy into
///
void vecCopy(const GLdouble* source, GLdouble* dest);

///
/// vecDot
/// \brief calculate and return vector dot product of two 3 component vectors
/// \param vec1 first vector
/// \param vec2 second vector
/// \retval GLdouble resulting dot product
///
GLdouble vecDot(const GLdouble* vec1, const GLdouble* vec2);

///
/// vecPrint
/// \brief print vector for debug purposes. (X: _ Y: _ Z: _)
/// \param vec Vector to print.
///
void vecPrint(const GLdouble* vec);

#endif

