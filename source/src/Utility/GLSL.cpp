/*
 * GLSL.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2006 Chris Wallace
 *
 * Fly! Legacy is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 * Fly! Legacy is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *   along with Fly! Legacy; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "../Include/GLSL.h"


GLuint CreateShaderProgram (int nShaders, ...)
{
  GLuint rc = 0;

  // Create program
  rc = glCreateProgram();

  // Attach all shader objects to the program
  va_list argp;
  va_start(argp, nShaders);
  for (int i=0; i<nShaders; i++) {
    glAttachShader (rc, va_arg(argp, GLuint));
  }
  va_end(argp);

  // Link the program and check for success
  glLinkProgram (rc);
  int status;
  glGetProgramiv (rc, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    WARNINGLOG ("Shader program linking error");

    // Dump info log to warning
    int infoLength;
    glGetProgramiv (rc, GL_INFO_LOG_LENGTH, &infoLength);
    char *infoBuffer = new char[infoLength];
    glGetProgramInfoLog (rc, infoLength, NULL, infoBuffer);
    WARNINGLOG ("%s", infoBuffer);
    delete[] infoBuffer;

    // Set return code to zero to indicate error
    rc = 0;
  }
  CHECK_OPENGL_ERROR

  return rc;
}

GLuint LoadShader (GLenum type, const char* src)
{
  GLuint rc = 0;

  rc = glCreateShader (type);
  if (rc != 0) {
    // Successfully created shader object
    FILE *f = fopen (src, "r");
    if (f) {
      // Load shader source code
      // @TODO Replace constant 64K buffer with actual file size
      char *srcBuffer = new char[65536];
      int nRead = fread (srcBuffer, sizeof(char), 65535, f);

      // Bind the source to the shader object
      glShaderSource (rc, 1, (const char**)&srcBuffer, &nRead);
      delete[] srcBuffer;

      // Compile shader
      glCompileShader (rc);

      // Check compile status
      int status;
      glGetShaderiv (rc, GL_COMPILE_STATUS, &status);
      if (status == GL_FALSE) {
        WARNINGLOG ("GLSL Compile error : %s", src);

        // Dump info log to warning
        int infoLength;
        glGetShaderiv (rc, GL_INFO_LOG_LENGTH, &infoLength);
        char *infoBuffer = new char[infoLength];
        glGetShaderInfoLog (rc, infoLength, NULL, infoBuffer);
        WARNINGLOG ("%s", infoBuffer);
        delete[] infoBuffer;

        // Set return code to zero to indicate error
        rc = 0;
      }
    } else {
      // Could not open source file for shader
      WARNINGLOG ("Could not open shader source code file %s", src);
    }
  } else {
    WARNINGLOG ("Could not create shader object for %s", src);
  }
  CHECK_OPENGL_ERROR

  return rc;
}

GLint GetUniformLocation (GLuint prog, const char* name)
{
  GLint rc = 0;

  rc = glGetUniformLocation (prog, name);
  if (rc == -1) {
    WARNINGLOG ("Could not get uniform location for %s", name);
  }
  CHECK_OPENGL_ERROR

  return rc;
}

GLint GetAttribLocation (GLuint prog, const char* name)
{
  GLint rc = 0;

  rc = glGetAttribLocation (prog, name);
  if (rc == -1) {
    WARNINGLOG ("Could not get attribute location for %s", name);
  }
  CHECK_OPENGL_ERROR

  return rc;
}

void  DumpOpenGLDriver (const char* filename)
{
  FILE *f = fopen (filename, "w");
  if (f) {
    GLint i;
    const char* s;

    // Driver details
    s = (const char*)glGetString(GL_VENDOR);
    fprintf (f, "OpenGL Vendor    : %s\n", s);

    s = (const char*)glGetString(GL_VERSION);
    fprintf (f, "OpenGL Version   : %s\n", s);
    
    s = (const char*)glGetString(GL_RENDERER);
    fprintf (f, "OpenGL Renderer  : %s\n", s);

    s = (const char*)glGetString(GL_EXTENSIONS);
    fprintf (f, "OpenGL Extensions:\n");
    char* tokens = new char[strlen(s) + 1];
    strcpy (tokens, s);
    char* t = strtok (tokens, " ");
    while (t != NULL) {
      fprintf (f, "  %s\n", t);
      t = strtok (NULL, " ");
    }
    delete[] tokens;

    // Driver limits
    glGetIntegerv (GL_MAX_MODELVIEW_STACK_DEPTH, &i);
    fprintf (f, "Max Modelview Stack Depth: %d\n", i);
    glGetIntegerv (GL_MAX_PROJECTION_STACK_DEPTH, &i);
    fprintf (f, "Max Projection Stack Depth: %d\n", i);
    glGetIntegerv (GL_MAX_TEXTURE_STACK_DEPTH, &i);
    fprintf (f, "Max Texture Stack Depth: %d\n", i);
    fprintf (f,  "\n");

    // Dump supported texture compression formats
    GLint nFormats;
    glGetIntegerv (GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB, &nFormats);
    GLint *format = new GLint[nFormats];
    glGetIntegerv (GL_COMPRESSED_TEXTURE_FORMATS_ARB, format);
    fprintf (f, "%d compressed formats supported : ", nFormats);
    for (int i=0; i<nFormats; i++) {
      fprintf (f, "0x%04X  ", format[i]);
    }
    fprintf (f,  "\n");
    delete[] format;

    // Close output file
    fclose (f);
  } else {
    WARNINGLOG ("Could not open OpenGL driver file: %s", filename);
  }
}

void  DumpOpenGLState (const char* filename)
{
  FILE *f = fopen (filename, "w");
  if (f) {
    GLboolean b;
    GLint i;
    GLfloat fp;
    GLfloat fv4[4];

    // Geometry data
    fprintf (f, "Geometry:\n");
    fprintf (f, "---------\n");
    glGetBooleanv (GL_COLOR_ARRAY, &b);
    fprintf (f, "GL_COLOR_ARRAY         : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_EDGE_FLAG_ARRAY, &b);
    fprintf (f, "GL_EDGE_FLAG_ARRAY     : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_INDEX_ARRAY, &b);
    fprintf (f, "GL_INDEX_ARRAY         : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_TEXTURE_COORD_ARRAY, &b);
    fprintf (f, "GL_TEXTURE_COORD_ARRAY : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_NORMAL_ARRAY, &b);
    fprintf (f, "GL_NORMAL_ARRAY        : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_VERTEX_ARRAY, &b);
    fprintf (f, "GL_VERTEX_ARRAY        : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_AUTO_NORMAL, &b);
    fprintf (f, "GL_AUTO_NORMAL         : %c\n", b ? 'Y' : 'N');
    fprintf (f, "\n");

    // Lighting
    fprintf (f, "Lighting:\n");
    fprintf (f, "---------\n");
    glGetBooleanv (GL_LIGHTING, &b);
    fprintf (f, "GL_LIGHTING : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_LIGHT0, &b);
    fprintf (f, "GL_LIGHT0   : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_LIGHT1, &b);
    fprintf (f, "GL_LIGHT1   : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_LIGHT2, &b);
    fprintf (f, "GL_LIGHT2   : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_LIGHT3, &b);
    fprintf (f, "GL_LIGHT3   : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_LIGHT4, &b);
    fprintf (f, "GL_LIGHT4   : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_LIGHT5, &b);
    fprintf (f, "GL_LIGHT5   : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_LIGHT6, &b);
    fprintf (f, "GL_LIGHT6   : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_LIGHT7, &b);
    fprintf (f, "GL_LIGHT7   : %c\n", b ? 'Y' : 'N');
    glGetFloatv (GL_LIGHT_MODEL_AMBIENT, fv4);
    fprintf (f, "GL_LIGHT_MODEL_AMBIENT : %f, %f, %f, %f\n",
                fv4[0], fv4[1], fv4[2], fv4[3]);
    fprintf (f, "\n");

    // Rasterizing
    fprintf (f, "Rasterizing:\n");
    fprintf (f, "------------\n");
    fprintf (f, "\n");

    // Fog
    fprintf (f, "Fog:\n");
    fprintf (f, "----\n");
    glGetBooleanv (GL_FOG, &b);
    fprintf (f, "GL_FOG         : %c\n", b ? 'Y' : 'N');
    glGetIntegerv (GL_FOG_MODE, &i);
    fprintf (f, "GL_FOG_MODE    : %d\n", i);
    glGetIntegerv (GL_FOG_INDEX, &i);
    fprintf (f, "GL_FOG_INDEX   : %d\n", i);
    glGetFloatv (GL_FOG_COLOR, fv4);
    fprintf (f, "GL_FOG_COLOR   : %f, %f, %f, %f\n", fv4[0], fv4[1], fv4[2], fv4[3]);
    glGetFloatv (GL_FOG_DENSITY, &fp);
    fprintf (f, "GL_FOG_DENSITY : %f\n", fp);
    glGetFloatv (GL_FOG_START, &fp);
    fprintf (f, "GL_FOG_START   : %f\n", fp);
    glGetFloatv (GL_FOG_END, &fp);
    fprintf (f, "GL_FOG_END     : %f\n", fp);
    glGetIntegerv (GL_FOG_HINT, &i);
    fprintf (f, "GL_FOG_HINT : %d\n", i);
    fprintf (f, "\n");
    
    // Fragment processing
    fprintf (f, "Fragment Processing:\n");
    fprintf (f, "-------------------\n");
    glGetFloatv (GL_COLOR_CLEAR_VALUE, fv4);
    fprintf (f, "GL_COLOR_CLEAR_VALUE : %f, %f, %f, %f\n", fv4[0], fv4[1], fv4[2], fv4[3]);
    glGetBooleanv (GL_TEXTURE_2D, &b);
    fprintf (f, "GL_TEXTURE_2D     : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_ALPHA_TEST, &b);
    fprintf (f, "GL_ALPHA_TEST     : %c\n", b ? 'Y' : 'N');
    glGetIntegerv (GL_ALPHA_TEST_FUNC, &i);
    fprintf (f, "GL_ALPHA_TEST_FUNC: %d\n", i);
    glGetFloatv (GL_ALPHA_TEST_REF, &fp);
    fprintf (f, "GL_ALPHA_TEST_REF : %f\n", fp);
    glGetBooleanv (GL_DEPTH_TEST, &b);
    fprintf (f, "GL_DEPTH_TEST     : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_BLEND, &b);
    fprintf (f, "GL_BLEND          : %c\n", b ? 'Y' : 'N');
    glGetBooleanv (GL_COLOR_MATERIAL, &b);
    fprintf (f, "GL_COLOR_MATERIAL : %c\n", b ? 'Y' : 'N');
    glGetIntegerv (GL_BLEND_SRC, &i);
    fprintf (f, "GL_BLEND_SRC      : %d\n", i);
    glGetIntegerv (GL_BLEND_DST, &i);
    fprintf (f, "GL_BLEND_DST      : %d\n", i);
    glGetBooleanv (GL_CULL_FACE, &b);
    fprintf (f, "GL_CULL_FACE      : %c\n", b ? 'Y' : 'N');
    fprintf (f, "\n");

    // Pixel transfer
    fprintf (f, "Pixel Transfer:\n");
    fprintf (f, "---------------\n");
    glGetFloatv (GL_RED_SCALE, &fp);
    fprintf (f, "GL_RED_SCALE     : %f\n", fp);
    glGetFloatv (GL_GREEN_SCALE, &fp);
    fprintf (f, "GL_GREEN_SCALE   : %f\n", fp);
    glGetFloatv (GL_BLUE_SCALE, &fp);
    fprintf (f, "GL_BLUE_SCALE    : %f\n", fp);
    fprintf (f, "\n");

    // Stack depths
    fprintf (f, "Stack Depths:\n");
    fprintf (f, "-------------\n");
    glGetIntegerv (GL_ATTRIB_STACK_DEPTH, &i);
    fprintf (f, "GL_ATTRIB_STACK_DEPTH        : %d\n", i);
    glGetIntegerv (GL_CLIENT_ATTRIB_STACK_DEPTH, &i);
    fprintf (f, "GL_CLIENT_ATTRIB_STACK_DEPTH : %d\n", i);
    glGetIntegerv (GL_PROJECTION_STACK_DEPTH, &i);
    fprintf (f, "GL_PROJECTION_STACK_DEPTH    : %d\n", i);
    glGetIntegerv (GL_MODELVIEW_STACK_DEPTH, &i);
    fprintf (f, "GL_MODELVIEW_STACK_DEPTH     : %d\n", i);
    fprintf (f, "\n");

    fclose (f);
  } else {
    WARNINGLOG ("Could not open OpenGL state file: %s", filename);
  }
}
