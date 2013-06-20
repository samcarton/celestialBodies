//loadTexture.h
// Code to load RAW texture files into GL
// Sam Carton 31510651
// Original 17 May 2012
// Updated 20 June 2013 - added loadTex


#ifndef LOADTEXTURE_H
#define LOADTEXTURE_H

#include <stdio.h>
#include <GL/freeglut.h>

///
/// loadRawTex
/// \brief Load and return a texture from a .raw file. Optimised for mapping to planets.
/// Mapping to other surfaces may require additional tweaking of texparameters.
/// \retval GLuint of TextureID
/// \param filename Filename string of texture file
/// \param width of texture in pixels
/// \param height of texture in pixels
///
GLuint loadRawTex(const char *filename, int width, int height);

///
/// loadTex
/// \brief Load and return texture from a number of file formats. Optimized for mapping
/// to planets.
/// \retval GLuint of TextureID
/// \param fileName Filename string of path to the texture file.
GLuint loadTex(const char* fileName);


#endif
