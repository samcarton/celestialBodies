#include "loadTexture.h"


GLuint loadRawTex(const char *filename, int width, int height)
 {
     GLuint texture;
     unsigned char *texBuff;
     FILE *file;

     // open texture file
     file = fopen(filename, "rb");
     if (file == NULL)
     {
         puts("*** Can't open texture file: ");
         puts(filename);
         return 0;
     }


     // allocate memory for texture
     texBuff = (unsigned char*) malloc(width * height * 3); // *3 for RGB components

     // read texture data
     fread(texBuff, width * height * 3, 1, file); //again, *3 for RGB components
     fclose(file);

     // allocate a texture name
     glGenTextures(1, &texture);

     // select current texture
     glBindTexture(GL_TEXTURE_2D, texture);

     // set texture environment, replace surface colour with texture
     glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

     //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_DECAL);
     //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_DECAL);

     // set minify function to map to closest mipmap
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
     // set magnify function to map to first mipmap
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

     // modify how texture tiles
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //if GL_CLAMP, a vertical seam appears in the short edge of sphere
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

     // build texture mipmaps. params:(target, internalFormat, width, height, format, type, *data)
     gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, texBuff);

     // free buffer
     free(texBuff);

     puts("***Loaded Texture:");
     puts(filename);

     return texture;
 }

GLuint loadTex(const char* fileName)
{
    GLuint texture;
    int x,y,n;
    unsigned char* data = stbi_load(fileName,&x,&y,&n,0);
    if(data == NULL)
    {
        puts("**Error loading texture file: ");
        puts(fileName);
        return 0;
    }

    // allocate a texture name
     glGenTextures(1, &texture);

     // select current texture
     glBindTexture(GL_TEXTURE_2D, texture);

     // set texture environment, replace surface colour with texture
     glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

     //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_DECAL);
     //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_DECAL);

     // set minify function to map to closest mipmap
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
     // set magnify function to map to first mipmap
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

     // modify how texture tiles
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //if GL_CLAMP, a vertical seam appears in the short edge of sphere
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

     GLenum pixFormat;
     if(n == 3)
     {
        pixFormat = GL_RGB;
     }
     else if(n == 4)
     {
         pixFormat = GL_RGBA;
     }


     // build texture mipmaps. params:(target, internalFormat, width, height, format, type, *data)
     gluBuild2DMipmaps(GL_TEXTURE_2D, n, x, y, pixFormat, GL_UNSIGNED_BYTE, data);

    // free buffer
     free(data);

     puts("***Loaded Texture:");
     puts(fileName);


     return texture;

}
