

#include "VecOps.h"

void vecAdd(const GLdouble* vec1, const GLdouble* vec2, GLdouble* vecSum)
{
    vecSum[0] = vec1[0] + vec2[0];
    vecSum[1] = vec1[1] + vec2[1];
    vecSum[2] = vec1[2] + vec2[2];
}

void vecScale(GLdouble* vec, GLdouble scale, GLdouble* dest)
{
        dest[0] = vec[0]*scale;
        dest[1] = vec[1]*scale;
        dest[2] = vec[2]*scale;
}

void vecMinus(const GLdouble* vec1, const GLdouble* vec2, GLdouble* vecRes)
{
    vecRes[0] = vec1[0] - vec2[0];
    vecRes[1] = vec1[1] - vec2[1];
    vecRes[2] = vec1[2] - vec2[2];
}

GLdouble vecMag(const GLdouble* vec)
{
    GLdouble total = vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];

    // should be ABSOLUTE value, but c doesnt seem to give the negative answer for a sqrt
    // Also, abs() seemed to make the number an int.
    return ((GLdouble)sqrt(total));
}

void vecCopy(const GLdouble* source, GLdouble* dest)
{
    dest[0] = source[0];
    dest[1] = source[1];
    dest[2] = source[2];
}

GLdouble vecDot(const GLdouble* vec1, const GLdouble* vec2)
{

    return (  vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2]  );
}

void vecPrint(const GLdouble* vec)
{
    printf("\nX:%f Y:%f Z:%f",vec[0], vec[1], vec[2] );
}
