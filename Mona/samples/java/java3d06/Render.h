// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __Render__
#define __Render__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  class Render;
  class VERTEX;
  class TEXTURE;
  class PRIMITIVE;
  class Vector;
  class Matrix;
  class TLVERTEX;
  class SCANLINE;
}

class Render : public ::java::lang::Object
{
public:
  Render (jint, jint);
  static void clear ();
  static void setTransform (jint, ::Matrix *);
  static void setLight (::Vector *);
  static void drawPrimitive (::PRIMITIVE *);
private:
  static void setAttribute (::PRIMITIVE *);
  static void setTexture (::TEXTURE *);
public:
  static void transformAndLighting (JArray< ::VERTEX *> *);
private:
  static void drawPolygon (::TLVERTEX *, ::TLVERTEX *, ::TLVERTEX *);
  static void scanEdge (::TLVERTEX *, ::TLVERTEX *);
public: // actually package-private
  static jintArray pbuf;
private:
  static jintArray zbuf;
  static jintArray tbuf;
  static jintArray abuf;
  static jint BUFFERW;
  static jint BUFFERH;
  static JArray< ::SCANLINE *> *min;
  static JArray< ::SCANLINE *> *max;
public: // actually package-private
  static const jint MAX_VERTEX = 4096L;
private:
  static JArray< ::TLVERTEX *> *tvtx;
public: // actually package-private
  static const jint TRANSFORM_WORLD = 0L;
  static const jint TRANSFORM_VIEW = 2L;
  static const jint TRANSFORM_PROJ = 3L;
  static const jint TRANSFORM_MASTER = 4L;
  static jint TEXTUREW;
  static jint TEXTUREH;
  static jint TEXMASKW;
  static jint TEXMASKH;
  static jint TEXSHIFT;
  static ::Matrix *mWorld;
  static ::Matrix *mMaster;
  static ::Matrix *mView;
  static ::Matrix *mProj;
  static ::Vector *mLight;
  static jint mColor;
  static jfloat mSpec;
  static jfloat mDiff;
  static jint mGlos;
public:

  static ::java::lang::Class class$;
};

#endif /* __Render__ */