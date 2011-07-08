/*
 * Collada.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
 * Copyright 2007 Jean Sabatier
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


#ifndef COLLADA_H
#define COLLADA_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//============================================================================================
//  COLLADA FILE PARSER
//============================================================================================
#include "FlyLegacy.h"
//============================================================================================
class CAcmPart;
class CModelACM;
class CVehicleObject;
class CAcmVPack;
//============================================================================================

enum XMLnode {
  XN_TXTU  = 1,               // node Texture
  XN_EFFC  = 2,               // node effect
  XN_SRCE  = 3,               // Node source
  XN_RDIR  = 4,               // Redirection
  XN_GEOM  = 5,               // Part Geometrie
  XN_ANIM  = 6,               // Animation part
  XN_SAMP  = 7,               // Sampler
  XN_MATR  = 8,               // Material
};
//============================================================================================
class X_FCOLOR {
public:
  float R;
  float G;
  float B;
  float A;
  //----------------------
public:
  void Init() { R = G = B = A = 1;}
};
//============================================================================================
enum  EEffectType {
  EFF_BLINN = 1,
  EFF_PHONG = 2,
};
//============================================================================================
enum  EMatType {
  MAT_EFFECT = 1,
};
//============================================================================================
enum  EParamName {
  NAME_X      = 0x00000001,           // X
  NAME_Y      = 0x00000002,           // Y
  NAME_Z      = 0x00000004,           // Z
  //-----------------------------------
  NAME_TIME   = 0x00000010,           // TIME
  NAME_ANGLE  = 0x00000020,           // ANGLE
  //-------------------------------------
};
//============================================================================================
enum ETransform {
  TRANS_X = 1,
  TRANS_Y = 2,
  TRANS_Z = 3,
  ROT_X   = 4,
  ROT_Y   = 5,
  ROT_Z   = 6,
};
//============================================================================================
//  Class Xnode:  Describe a collada node
//
//============================================================================================
class Xnode {
  //--- Attributes -------------------------------------------
  U_INT     type;                         // Type of node
  //--- Methods ----------------------------------------------
public:
  Xnode(char t) {type = t;}
  //----------------------------------------------------------
  inline char Type()  {return type;}
};
//============================================================================================
//  Node for texture file
//============================================================================================
class Ximag: public Xnode {
  friend class ColladaParser;
  //--- Attributes -------------------------------------------
  char      tname[512];                   // Texture name
  //----------------------------------------------------------
public:
  Ximag(): Xnode(XN_TXTU) {}
  //----------------------------------------------------------
  inline void SetFile(char *f)  {strncpy(tname,f,512); tname[511]=0;}
};
//============================================================================================
//  Node for effect
//============================================================================================
class Xeffc: public Xnode {
  friend class ColladaParser;
  //--- Attributes -------------------------------------------
  char        texture[32];         // Texture name    
  //----------------------------------------------------------
  char        type;
  //----------------------------------------------------------
  X_FCOLOR    emission;
  X_FCOLOR    ambient;
  X_FCOLOR    diffuse;
  X_FCOLOR    specular;
  X_FCOLOR    reflective;
  X_FCOLOR    transparent;
  //----------------------------------------------------------
  float       shininess;
  float       reflectivity;
  float       transparency;
  //----------------------------------------------------------
public:
  Xeffc(): Xnode(XN_EFFC) {}
  void        Init();
};
//============================================================================================
//  Node for material
//============================================================================================
class Xmatr: public Xnode {
  friend class ColladaParser;
  //--- Attributes -------------------------------------------
  char        type;
  char        effet[32];         // Texture name
  //----------------------------------------------------------
public:
  Xmatr(): Xnode(XN_MATR) {}
  //----------------------------------------------------------
  void        Init() { effet[0] = 0; }
};
//============================================================================================
//  Node for source 
//============================================================================================
class Xsrce: public Xnode {
  friend class ColladaParser;
  //--- ATTRIBUTES -------------------------------------------
  U_INT   prop;             // Properties
  int     nbv;              // Vertex number
  float *vtab;              // Vertex table
  //----------------------------------------------------------
public:
  Xsrce();
 ~Xsrce()  {if (vtab) delete vtab;}
 void   Clear();
 //-----------------------------------------------------------
 inline bool NotEmpty() {return (0 != vtab);}
 inline bool IsNoTIME() {return (0 == (prop & NAME_TIME));}
};
//============================================================================================
//  Node for redirection 
//============================================================================================
class Xrdir: public Xnode {
  friend class ColladaParser;
  //--- ATTRIBUTES -------------------------------------------
  char   ident[32];
  //----------------------------------------------------------
public:
  Xrdir (): Xnode(XN_RDIR) {}
};
//============================================================================================
//  Node for geometrie 
//============================================================================================
class Xgeom: public Xnode {
  friend class ColladaParser;
	friend class CAcmVPack;
  //--- ATTRIBUTES -------------------------------------------
  char       rend;
  int        nbv;
  GN_VTAB   *vtab;
  //----------------------------------------------------------
public:
  Xgeom(): Xnode(XN_GEOM) {}
 ~Xgeom() {if (vtab)  delete [] vtab;}
  //----------------------------------------------------------
  inline void Clear() {nbv = 0; vtab = 0;}
};
//============================================================================================
//  Node for sampler 
//============================================================================================
class Xsamp: public Xnode {
  friend class ColladaParser;
  //--- ATTRIBUTES -------------------------------------------
  Xsrce     *inpt;
  Xsrce     *outp;
  //----------------------------------------------------------
public:
  Xsamp(): Xnode(XN_SAMP) {inpt = 0; outp = 0;}
};

//============================================================================================
//  Animation structure
//============================================================================================
class Xanim: public Xnode {
public:
  //--------------------------------------------
  U_INT trans;                      // Transformation
  float tim0;                       // Time 0
  float val0;                       // Value 0
  float tim1;                       // time 1
  float val1;                       // Value 1
  //--------------------------------------------
  char  target[32];                 // Target node
public:
  Xanim();
  void  Assign(CKeyFrame *key);
};

//============================================================================================
//  Collada parser
//============================================================================================
class ColladaParser {
  //-----Attributes ---------------------------------------------
  std::map<std::string,Xnode *>     Nodes;
  std::map<std::string,Ximag *>     Imags;
  std::map<std::string,Xeffc *>     Effcs;
  std::map<std::string,Xmatr *>     Matrs;
  std::vector<Xanim *>              Anims;
  std::vector<CKeyFrame*>           nKeys;
	//----- List of parts -----------------------------------------
	U_INT			nbVT;													// Total of vertices
	std::vector<CAcmVPack*> pack;
	CAcmVPack *pak;
	//-------------------------------------------------------------
  PODFILE  *pf;                           // Current file
  FILE     *file;                         // Window file
  U_INT     Pos;                          // Line position
  U_INT     Deb;                          // File deb
	//---feet coefficient -----------------------------------------
	double		fRat;													// Feet ratio
	//-------------------------------------------------------------
	CVehicleObject *mveh;										// Mother vehicle
  //------Model ACM ---------------------------------------------
  CModelACM *model;                       // Current model
  //-------------------------------------------------------------
  int       pSeq;                         // Parent index
  int       Seq;                          // Sequence number
  //-----Part working area --------------------------------------
  int       nbix;                         // Number of elements
  int       qcnt;                         // Number of polygons
  int       vcnt;                         // Number of vertices 
  int      *ppol;                         // P table
	//-------------------------------------------------------------
	char			indv;													// Vertex indicator
	char			indn;													// Normal indicator
	char			indt;													// Texture indicator
  //-------------------------------------------------------------
  U_CHAR    tr;                           // Trace indicator
  //-------------------------------------------------------------
  U_INT     render;                       // Render mode
  U_CHAR    vofs;                         // Vertex offset
  U_CHAR    nofs;                         // Normal offset
  U_CHAR    tofs;                         // Texture offset
  //-------------------------------------------------------------
  Xsrce    *vnod;                         // Vertex table
  Xsrce    *nnod;                         // Normal table
  Xsrce    *xnod;                         // Texture table
  //-----Material ----------------------------------------------
  Xmatr     matr;                         // Material
  //-----Effect-------------------------------------------------
  Xeffc     effet;                        // Effect
  //-----Sources -----------------------------------------------
  Xsrce     srce;                         // Current source
  U_INT     outP;                         // output property
  float     out0;                         // Output  value from
  float     out1;                         // Output  value to
  //-------Trace area ------------------- ------------------------
  char      ztr[128];                     // Trace buffer
  char     *zlv;                          // Level 
  //-------Decoded fields ---------------------------------------
  char      geoid[32];                    // geometry id
  char      srcid[32];                    // Source id
  char      effid[32];                    // effect id
  char      matid[32];                    // Material id
  //------------------------------------------------------
  char      fname[512];                   // File name
  char      attri[512];                   // Attribute
  char      name[32];                     // Last name
  char      id[32];                       // Last id
  char      sid[32];                      // Last SID
  char      url[32];                      // Last url
  char      count[32];                    // Count
  char      offset[32];                   // Offset
  char      semantic[32];                 // semantic
  char      source[32];                   // source
  char      set[32];                      // Szet
  char      material[32];
  //-------------------------------------------------------------
public:
  ColladaParser(char *fn,CVehicleObject *v);
 ~ColladaParser();
  bool				Backup();
  //--------------------------------------------------------------
  void       CodeFile(char *dst);
  //------Heelpers  ----------------------------------------------
  bool      GetVertex(char *sm,char *k);
  bool      GetNormal(char *sm,char *k);
  bool      GetTextST(char *sm,char *k);
  Xsrce    *GetSource(char *sem,char *key);
  void      BuildC3PartNode();
  U_INT     SetSrcProperty(char *nm);
  void      ExtractName(char *src,char *dst,int nb);
  //--------------------------------------------------------------
  bool      ParseXML();
  bool      FullToken(char *token);
  bool      ParseList(char *token,char **att);
  bool      SkipToken(char *token);
  bool      ParseToken(char *token);
  bool      ParseEnd(char *token);
  bool      ParseAttribute(char *msk,char *dst,int nb);
  bool      FloatColor(X_FCOLOR &col);
  bool      FloatValue(float *f);
  bool      DecodeTexture();
  bool      InstanceMaterial();
  bool      ParseFloatArray();
  bool      ParseIdName(char *token,char *idd,char *nm);
  bool      SkipToEnd(char *token);
  //----------------------------------------------
  void      ReadFloat(float *dst,int nbf);
  void      CopyKeys(CKeyFrame &prv,CKeyFrame &kfn);
  void      NormeKey(CKeyFrame &kfn);
  //----------------------------------------------
  bool      TopNode();
  bool      ParseAsset();
	bool			ParseCreated();
  bool      ParseUnit();
  bool      ParseUpAxis();
  //----------------------------------------------
  bool      ParseLibraryImage();
  bool      ParseImage();
  bool      ParseImageFrom(char *dst,int lg);
  //-----------------------------------------------
  bool      ParseTechniqueCommon();
  bool      ParseAccessor();
  bool      ParseParam();
  //-----------------------------------------------
  bool      ParseEffectTechnique();
  bool      EffectProfileCG();
  bool      EffectProfileCommon();
  bool      ParseEffectNewParam();
  bool      ParseEffect();
  bool      ParseEffectProfile();
  bool      ParseLibraryEffect();
  //-----------------------------------------------
  bool      ParseInitFrom();
  bool      ParseSurface();
  bool      ParseBlinnEffect();
  bool      ParseEmission();
  bool      ParseAmbient();
  bool      ParseDiffuse();
  bool      ParseSpecular();
  bool      ParseShininess();
  bool      ParseReflective();
  bool      ParseReflectivity();
  bool      ParseTransparent();
  bool      ParseTransparency();
  bool      ParseRefraction();
  //-----------------------------------------------
  bool      ParseMaterial();
  bool      ParseLibraryMaterial();
  //-----------------------------------------------
	bool			ParseGeomItem();
  bool      ParsePindices();
  bool      ParseVCount();
  bool      ParsePolylist();
	bool			ParseTriangle();
  //-----------------------------------------------
  void      RedirectSource();
  bool      ParseSource();
  bool      ParseInput();
  bool      ParseVertices();
  bool      ParseMesh();
	bool			ParseMeshEntry();
  bool      ParseGeometry();
  bool      ParseLibraryGeometry();
  //-----------------------------------------------
  U_INT     GetTransformation(char *trf,char *cmp);
  bool      GetInputKey (Xsamp *nod);
  bool      GetOutputKey(Xsamp *nod);
  bool      ParseChannel();
  bool      ParseSampler();
  bool      ParseAnimation();
  bool      ParseLibraryAnimations();
  //-----------------------------------------------
  Xanim    *NextAnimation(char *idn,CKeyFrame *key);
  void      SetMaterial(CAcmPart *part,char *mat);
	bool			ParseBindVertex();
  bool      ParseInstanceMaterial(CAcmPart *p);
  bool      ParseMaterialTechnique(CAcmPart *p);
  bool      ParseBindMaterial(CAcmPart *p);
  bool      BuildKeyframes(char *idn);
  bool      MakeKeyframe(CAcmPart *part,char *id);
  bool      InstanceGeometry(CAcmPart *part);
  bool      ParseScale(CAcmPart *p);
  bool      ParseRotation(CAcmPart *p);
  bool      ParseTranslation(CAcmPart *p);
  bool      ParseTransformations(CAcmPart *p);
  bool      ParseNode();
  bool      ParseVisualScene();
  bool      ParseLibraryVisualScene();
	//-------------------------------------------------
	inline CModelACM* GetModel()		{return model;}	
};
//================END OF FILE ================================================================
#endif // COLLADA_H

