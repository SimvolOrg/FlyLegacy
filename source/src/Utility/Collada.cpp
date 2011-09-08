/*
 * Collada.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2007-.... Jean Sabatier
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
#include "../Include/Globals.h"
#include "../Include/Collada.h"
#include "../Include/ModelACM.h"
#include "../Include/TerrainTexture.h"
#include <algorithm>
using namespace std;
//===================================================================================
//  COLLADA NODE MANAGEMENT
//===================================================================================
//============================================================================================
X_FCOLOR white = {1,1,1,1};
//===================================================================================
//  Attributes list
//===================================================================================
char *attTok      = " < %31[^ ><]s ";
char *attId       = " id       = %31[^ ></]s ";
char *attName     = " name     = %31[^ ></]s ";
char *attSid      = " sid      = %31[^ ></]s ";
char *attCount    = " count    = %31[^ ></]s ";
char *attType     = " type     = %31[^ ></]s ";
char *attSemantic = " semantic = %31[^ ></]s ";
//===================================================================================
char *atlIN00[]  = {attId, attName, 0};
//===================================================================================
//  Level table for trace
//===================================================================================
char *colLEV = "................................";
//=====================================================================================
//  sort key on time part
//=====================================================================================
bool SortKeyframe(CKeyFrame *p1,CKeyFrame *p2)
{ return (p1->frame < p2->frame);
}

//===================================================================================
//  COLLADA FILE PARSER: Class animator
//===================================================================================
Xanim::Xanim(): Xnode(XN_ANIM)
{ tim0  = tim1 = 0;
  val0  = val1 = 0;
}
//----------------------------------------------------------------------
//  Assign value to keyframe
//  NOTE:  For translation Y and Z are inverted
//----------------------------------------------------------------------
void Xanim::Assign(CKeyFrame *key)
{ switch (trans) {
  case TRANS_X:
    key->dx = +val1;
    return;
  case TRANS_Y:
    key->dz = +val1;
    return;
  case TRANS_Z:
    key->dy = -val1;
    return;
  case ROT_X:
    key->p  = +val1;
    return;
  case ROT_Y:
    key->h  = +val1;
    return;
  case ROT_Z:
    key->b  = -val1;
    return;
  }
  return;
}
//===================================================================================
//  COLLADA FILE PARSER: Class effect
//===================================================================================
void Xeffc::Init()
{ texture[0] = 0;
  emission.Init();
  ambient.Init();
  diffuse.Init();
  specular.Init();
  reflective.Init();
  transparent.Init();
  shininess = 0.01f;
  reflectivity = 0.01f;
  transparency = 0;
}
//===================================================================================
//  COLLADA FILE PARSER: Class source
//===================================================================================
Xsrce::Xsrce(): Xnode(XN_SRCE)
{ nbv   = 0;
  vtab  = 0;
}
//--------------------------------------------------------------------
void  Xsrce::Clear()
{ prop  = 0;
  nbv   = 0;
  vtab  = 0;
}
//===================================================================================
//  COLLADA FILE PARSER
//===================================================================================
//  Constructor
//====================================================================
ColladaParser::ColladaParser(char *fn,CVehicleObject *v)
{ mveh	= v;
	pf    = popen (&globals->pfs, fn);
	if (0 == pf)	gtfo("No file %s",fn);
  file  = pf->pFile;
  Deb   = 0;
  tr    = 1;
  zlv   = colLEV + 32;
  pSeq  = -1;
  Seq   = 0;
	nbVT	= 0;
  model = new CModelACM(PART_BODY,PART_CHILD,mveh);
  if (file) ParseXML();
}
//--------------------------------------------------------------------
//  Destructor
//  TODO Clear all data
//--------------------------------------------------------------------
ColladaParser::~ColladaParser()
{ if (pf) pclose(pf);
  //---Free all nodes -------------------------------
  std::map<std::string,Xnode*>::iterator it;
  for (it = Nodes.begin(); it != Nodes.end(); it++)
  { Xnode *nod = (*it).second;
    delete (nod);
  }
  //-------------------------------------------------
  std::map<std::string,Ximag*>::iterator i1;
  for (i1 = Imags.begin(); i1 != Imags.end(); i1++)
  { Ximag *nod = (*i1).second;
    delete (nod);
  }
  //-------------------------------------------------
  std::map<std::string,Xeffc*>::iterator i2;
  for (i2 = Effcs.begin(); i2 != Effcs.end(); i2++)
  { Xeffc *nod = (*i2).second;
    delete (nod);
  }
  //-------------------------------------------------
  std::map<std::string,Xmatr*>::iterator i3;
  for (i3 = Matrs.begin(); i3 != Matrs.end(); i3++)
  { Xmatr *nod = (*i3).second;
    delete (nod);
  }
  //-------------------------------------------------
  std::vector<Xanim*>::iterator i4;
  for (i4 = Anims.begin(); i4 != Anims.end(); i4++)
  { Xanim *nod = (*i4);
    delete (nod);
  }
  //-------------------------------------------------
  std::vector<CKeyFrame*>::iterator i5;
  for (i5 = nKeys.begin(); i5 != nKeys.end(); i5++)
  { CKeyFrame *nod = (*i5);
    delete (nod);
  }

}
//--------------------------------------------------------------------
//  Back up file position and return false
//--------------------------------------------------------------------
bool ColladaParser::Backup()
{	fseek(file,Pos,Deb); 
	return false;
}
//--------------------------------------------------------------------
//  Retrieve a mesh by name
//--------------------------------------------------------------------
Xsrce *ColladaParser::GetSource(char *sem,char *key)
{ std::map<std::string,Xnode*>::iterator it = Nodes.find(key);
  if (it == Nodes.end())  return 0;
  Xnode *nod = (*it).second;
  switch (nod->Type())  {
    case XN_RDIR:
      { Xrdir *ndr = (Xrdir*)nod;
        return GetSource(sem,ndr->ident);
      }
    case XN_SRCE:
      { Xsrce *nms = (Xsrce*)nod;
        return nms;
      }
  }
  return 0;
}
//--------------------------------------------------------------------
//  Get vertex mesh
//--------------------------------------------------------------------
bool ColladaParser::GetVertex(char *sem,char *key)
{ if (strcmp(sem,"VERTEX") != 0)  return false;
  vnod = GetSource(sem,key);
  if (0 == vnod)  gtfo("No VERTEX table %s",key);
  nbix++;
  vofs  = atoi(offset);
  if (vofs > 2)   gtfo("Bad offset %s",key);
	indv	= 1;
  return true;
}
//--------------------------------------------------------------------
//  Get normal mesh
//--------------------------------------------------------------------
bool ColladaParser::GetNormal(char *sem,char *key)
{ if (strcmp(sem,"NORMAL") != 0)  return false;
  nnod = GetSource(sem,key);
  if (0 == nnod)  gtfo("No NORMAL table %s",key);
  nbix++;
  nofs  = atoi(offset);
  if (nofs > 2)   gtfo("Bad offset %s",key);
	indn	= 1;
  return true;
}
//--------------------------------------------------------------------
//  Get texture mesh
//--------------------------------------------------------------------
bool ColladaParser::GetTextST(char *sem,char *key)
{ if (strcmp(sem,"TEXCOORD") != 0)  return false;
  xnod = GetSource(sem,key);
  if (0 == xnod)  gtfo("No TEXCOORD table %s",key);
  nbix++;
  tofs  = atoi(offset);
  if (nofs > 2)   gtfo("Bad offset %s",key);
	indt	= 1;
  return true;
}
//--------------------------------------------------------------------
//  Build a part node
//  NOTE: 
//  1) 3 components may be present (vertex,normal and texture)
//  2) it is assumed that offset in indices table (ppol) are
//  offset 0:   Vertex index
//  offset 1:   Normal index
//  offset 2:   Texture index
//--------------------------------------------------------------------
void ColladaParser::BuildC3PartNode()
{ Xgeom   *nod = new Xgeom();
  GN_VTAB *tab = new GN_VTAB[vcnt];
  int      pvl;         // p value
  int      vno;         // Vertex No
  int      nno;         // Normal No
  int      xno;         // Texture No
  //---current indices ------------------------
  int     ind[3];       
  //---init source values ---------------------
	float   *vsr = (vnod)?(vnod->vtab):(0); // Coordinates
	float   *nsr = (nnod)?(nnod->vtab):(0);	// Normals
	float   *xsr = (xnod)?(xnod->vtab):(0);	// Textures
  //---init source index ----------------------
  int     *six = ppol;
  //---Init destination vertex table ----------
  GN_VTAB *dst = tab;
  //-------------------------------------------
  for (int k=0; k<vcnt; k++)
  { //---Extract the 3 indices --------
    if (indv)	ind[0]  = six[vofs];
    if (indn) ind[1]  = six[nofs];
    if (indt) ind[2]  = six[tofs];
		six += nbix;
    //---Compute vertex base indice -------
    pvl     = ind[0];
    vno     = pvl * 3;			// Vertex base
    if (vnod && (vno > vnod->nbv))    gtfo("VERTEX Indice error"); 
		//---Compute normal base indice --------
    pvl     = ind[1];
    nno     = pvl * 3;			// Normal base
    if (nnod && (nno > nnod->nbv))    gtfo("NORMAL Indice error");
		//---Compute texture base indice -------
    pvl     = ind[2];
    xno     = pvl * 2;			// Base texture
    if (xno && (xno > xnod->nbv))			gtfo("TEXTCOORD Indice error");
    //---Store vertex coordinates ----------
		if (indv)
		{	dst->VT_X = +vsr[vno++] * fRat;     // X coordinate
			dst->VT_Z = +vsr[vno++] * fRat;     // Y coordinate
			dst->VT_Y = -vsr[vno]   * fRat;     // Z coordinate
		}
		//---Store normal index -----------------
		if (indn)
		{ dst->VN_X = +nsr[nno++];     // X normal
			dst->VN_Z = +nsr[nno++];     // Y normal
			dst->VN_Y = -nsr[nno];       // Z normal
		}
		//---Store texture index ------------------
		if (indt)
		{ dst->VT_S = +xsr[xno++];     // S coordinate
			dst->VT_T = -xsr[xno];       // T coordinate
		}
		//---Vertex is now complete ---------------
		model->SaveExtension("body",*dst);
    //---Next vertex entry --------------------
    dst++;
  }
  //----Store the geometry part ---------------
  nod->rend = render;
  nod->nbv  = vcnt;              // Number of vertices
  nod->vtab = tab;
  //-------------------------------------------
  Nodes[geoid] = nod;
  return;
}
//--------------------------------------------------------------------
//  Extract file name
//  Eliminate all path
//  Reencode special characters
//--------------------------------------------------------------------
void ColladaParser::CodeFile(char *dfn)
{ char *sfn = strchr(fname,'/');
  if  (0 == sfn) sfn = fname;
  else  sfn++;
  char *src = sfn;
  char *dst = dfn;
  char  car = *sfn;
  while (*src)
  { car = *src;
    if (car != '%') {*dst++ = *src++; continue;}
    // Decode the specific character -----------
    src++;
    char p1 = (*src++) - '0';
    char p2 = (*src++) - '0';
    int  cx = (p1 << 4) | p2;
    *dst++  = cx;
  }
 *dst = 0;
  return;
}

//--------------------------------------------------------------------
//  Skip token
//  NOTE:  Actual routine considers token up to 32 char size.
//--------------------------------------------------------------------
bool ColladaParser::SkipToken(char *token)
{ char a1[128];
  char go = 1;
  Pos     = ftell(file);
  fscanf(file," < %32[^ ><]s",a1);
  if (strcmp(a1,token) != 0)  return Backup();
  //---Trace token ---------------------------------
  if (tr) 
  { sprintf(ztr,"%sSKIP %s",zlv,token);
    TRACE(ztr,0);
  }
  //---Skip token until intro < --------------------
  char car = 0;
  while (go)
  { //--- Look for introduction < ------------
    car = fgetc(file);
    if (EOF == car)                 return false;
    if (car != '<')                 continue;
    a1[0] = 0;
    //---- Check for end ---------------------
    fscanf(file," /%32[^> ]s",a1);
    if (strncmp(a1,token,31) != 0)  continue;
    fscanf(file," > ");
    go = 0;
  }
  return true;
}
//--------------------------------------------------------------------
//  Skip to end of token
//  NOTE:  Actual routine considers token up to 32 char size.
//--------------------------------------------------------------------
bool ColladaParser::SkipToEnd(char *token)
{ char a1[32];
  char go = 1;
  char car = 0;
  while (go)
  { // -- Look for introduction ----------
    car = fgetc(file);
    if (EOF == car)                 return false;
    if (car != '<')                 continue;
    a1[0] = 0;
    //---- Check for end ---------------------
    fscanf(file," /%32[^> ]s",a1);
    if (strncmp(a1,token,31) != 0)  continue;
    fscanf(file," > ");
    go = 0;
  }
  zlv++;
  return true;
}
//--------------------------------------------------------------------
//  Parse only the token
//--------------------------------------------------------------------
bool ColladaParser::ParseToken(char *token)
{ char a1[128];
  Pos = ftell(file);
  fscanf(file, attTok,a1);
  if (strcmp(a1,token)!= 0) return Backup();
  //---- Trace token -------------------------
  if (tr)
  { sprintf(ztr,"%s %s",zlv--,token);
    TRACE(ztr,0);
  }
  return true;
}
//--------------------------------------------------------------------
//  Parse full token
//	-token is specified by token
//	The parser check for the termination token '>'
//--------------------------------------------------------------------
bool ColladaParser::FullToken(char *token)
{ char a1[128];
  Pos = ftell(file);
  fscanf(file, attTok,a1);
  fscanf(file, " > ");
  if (strcmp(a1,token)!= 0) return Backup();
  //-------Trace token ---------------------------
  if (tr)
  { sprintf(ztr,"%s %s",zlv--,token);
    TRACE(ztr,0);
  }
  return true;
}
//--------------------------------------------------------------------
//  Parse full token aId and name
//--------------------------------------------------------------------
bool ColladaParser::ParseIdName(char *token,char *idd,char *nmm)
{ char a1[32];
  Pos = ftell(file);
  fscanf(file, attTok,a1);
  if (!strcmp(a1,token)== 0) return Backup();
  //---Parse id and name -----------------------
  *idd = 0;
  *nmm = 0;
  ParseAttribute(attId,idd,31);
  ParseAttribute(attName,nmm,31);
  fscanf(file," > ");
  //------Trace node ---------------------------
  if (tr)
  { sprintf(ztr,"%s %s ID %s",zlv--,token,idd);
    TRACE(ztr,0);
  }
  return true;
}
//--------------------------------------------------------------------
//  Decode texture item
//--------------------------------------------------------------------
bool ColladaParser::DecodeTexture()
{ char pm1[32];
  char pm2[32];
  Pos      = ftell(file);
  int  nf  = fscanf(file,"< texture texture = %32[^ ><]s ",pm1);
       nf += fscanf(file,"  texcoord = %32[^ ></]s ", pm2);
 fscanf(file," / >");
 if (nf == 2)               return true;
 //-----must remove "" if needed ------------------------
 return Backup();
}
//--------------------------------------------------------------------
//  Parse float  color
//--------------------------------------------------------------------
bool ColladaParser::FloatColor(X_FCOLOR &col)
{ Pos      = ftell(file);
  int nf = fscanf(file," < color > %f %f %f %f < /color > ",&col.R,&col.G,&col.B,&col.A);
	if (4 != nf)	return Backup();
  return true;
}
//--------------------------------------------------------------------
//  Parse float  value
//--------------------------------------------------------------------
bool ColladaParser::FloatValue(float *f)
{ int nf = fscanf(file, " < float > %f < / float> ",f);
  return true;
}
//--------------------------------------------------------------------
// Read an array of float values
//--------------------------------------------------------------------
void ColladaParser::ReadFloat(float *dst,int nbf)
{ for (int k=0; k<nbf; k++) fscanf(file," %f ",dst++);
  return;
}
//--------------------------------------------------------------------
//  Parse a float array
//--------------------------------------------------------------------
bool ColladaParser::ParseFloatArray()
{ char *token = "float_array";
  if (!ParseToken(token))   return false;
  //---Set count to 0 ------------------------------
  if (srce.NotEmpty())   gtfo("Parse error on float_array");
  strcpy(count,"0");
  //---Parse attributes ----------------------------
  char pm1[32];           // Id
  char pm2[32];           // Name
  ParseAttribute(" id    = %31[^ ><]s ",pm1,   31);
  ParseAttribute(" name  = %31[^ ><]s ",pm2,   31);
  ParseAttribute(" count = %31[^ ><]s ",count, 31);
  fscanf(file, " > ");
  //-- Parse childs --------------------------------
  int nbv    = atoi(count);
  srce.nbv   = nbv;               // Extract nbr vertices
  srce.vtab  = new float[nbv];    // Allocate table
  ReadFloat(srce.vtab,nbv);
	//---- Trace token -------------------------
  if (tr)
  { sprintf(ztr,"%sArray %s count %d",zlv,pm1,nbv);
    TRACE(ztr,0);
  }
  //-- Parse end -----------------------------------
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Extract name
//--------------------------------------------------------------------
void ColladaParser::ExtractName(char *org,char *dst,int nb)
{ char *src = (*org == '"')?(org + 1):(org);
  while (*src)
  { if (*src == '"')  break;
    if (0 == nb--)    break;
    *dst++ = *src++;
  }
  *dst = 0;
  return;
}
//--------------------------------------------------------------------
//  Parse Attribute up to 128 characters
//--------------------------------------------------------------------
bool ColladaParser::ParseAttribute(char *msk,char *dst,int nb)
{ Pos = ftell(file);
  int nf = fscanf(file,msk,attri);
	if (1 != nf)	return Backup();
  ExtractName(attri,dst,nb);
  return true;
}
//--------------------------------------------------------------------
//  Parse Token end
//--------------------------------------------------------------------
bool ColladaParser::ParseEnd(char *token)
{ char a1[128];
  Pos = ftell(file);
  fscanf(file, " < / %32[^ ><]s",a1);
  fscanf(file, " > ");
  if (strcmp(a1,token)== 0) {zlv++; return true;}
	return Backup();
}
//--------------------------------------------------------------------
//  Parse Created for debug purpose to get the date
//  
//--------------------------------------------------------------------
bool ColladaParser::ParseCreated()
{	char *token = "created";
  if (!FullToken(token))  return false;
	//--- Extract attribute ----------------------------------
	char pm1[128];
	ParseAttribute(" %128[^>< ]s ",pm1,127);
	if (tr)
  { sprintf(ztr,"  %s",pm1);
    TRACE(ztr,0);
  }
	return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse unit
//	Compute the feet ratio:  Coeeficient by wich all coordinates
//	must be multiplied by to get feet.
//--------------------------------------------------------------------
bool ColladaParser::ParseUnit()
{ Pos = ftell(file);
	//--- Extract attributes ------------------------
	char pm1[32];							// Unit meter
	char pm2[31];							// unit name
	ParseAttribute(" < unit meter = %12[^> ]s ",pm1,31);
	ParseAttribute(" name = %16[^>/ ]s ",				pm2,31);
 // int nf = fscanf(file," < unit meter = %12[^> ]s ",a1);
//  nf    += fscanf(file," name = %16[^>/ ]s ",a2);
 // if (nf != 2) return Backup();
	//--- Compute feet ratio ------------------------
	fRat	 = strtod(pm1,0);
	fRat	 = FN_FEET_FROM_METER(fRat);
  //----Trace token -------------------------------
  if (tr)
  { sprintf(ztr,"%s unit",zlv);
    TRACE(ztr,0);
		sprintf(ztr,"%sis %s",zlv,pm2);
		TRACE(ztr,0);
  }
  //----------------------------------------------
  fscanf(file," / > ");
  return true;
}
//--------------------------------------------------------------------
//  Parse unit
//--------------------------------------------------------------------
bool ColladaParser::ParseUpAxis()
{ char *token = "up_axis";
  if (!FullToken(token))   return false;
  //---------------------------------------------
  char a1[32];
  fscanf(file,"%12[^<> ]s",a1);
  fscanf(file," > ");
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse Asset
//--------------------------------------------------------------------
bool ColladaParser::ParseAsset()
{ char *token = "asset";
  if (!FullToken(token))  return false;
  //----------------------------------------
  while (SkipToken("contributor")) continue;
//  SkipToken("created");
	ParseCreated();
  SkipToken("keywords");
  SkipToken("modified");
  SkipToken("revision");
  SkipToken("subject");
  SkipToken("title");
  ParseUnit();
  ParseUpAxis();
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse image source
//--------------------------------------------------------------------
bool ColladaParser::ParseImageFrom(char *dst,int lg)
{ char *token = "init_from";
  if (!FullToken(token)) return false;
  //---Decode file name ----------------------
  fscanf(file,"%256[^ ><]s ",fname);
  CodeFile(dst);
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse image
//--------------------------------------------------------------------
bool ColladaParser::ParseImage()
{char p2[32] = "";
 char p3[32] = "";
 char p4[32] = "";
 char p5[32] = "";
 char p6[32] = "";
 char *token = "image";
 if (!ParseToken(token))    return false;
 ParseAttribute(" id =     %32[^ ><]s",id,31);
 ParseAttribute(" name =   %32[^ ><]s",p2,31);
 ParseAttribute(" format = %32[^ ><]s",p3,31);
 ParseAttribute(" height = %32[^ ><]s",p4,31);
 ParseAttribute(" width  = %32[^ ><]s",p5,31);
 ParseAttribute(" depth  = %32[^ ><]s",p6,31);
 //------------------------------------------------------
 fscanf(file," > ");
 char p7[256];
 ParseImageFrom(p7,256);
 //---Create a node with texture name -------------------
 Ximag *nod = new Ximag();
 nod->SetFile(p7);
 Imags[id]  = nod;
 return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse Effect profile CG
//--------------------------------------------------------------------
bool ColladaParser::EffectProfileCG()
{ char *token = "profile_CG";
  if (!ParseToken(token)) return false;
  //---To do: implement it -----------
  zlv++;
  return true;
}
//--------------------------------------------------------------------
//  Parse emissive color
//--------------------------------------------------------------------
bool ColladaParser::ParseEmission()
{ char *token = "emission";
  if (!FullToken(token))   return false;
  FloatColor(effet.emission);
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse ambient color
//--------------------------------------------------------------------
bool ColladaParser::ParseAmbient()
{ char *token = "ambient";
  if (!FullToken(token))   return false;
  FloatColor(effet.ambient);
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse diffuse color
//--------------------------------------------------------------------
bool ColladaParser::ParseDiffuse()
{ char *token = "diffuse";
  if (!FullToken(token))   return false;
  //---Decode texture ---------------
  DecodeTexture();
  FloatColor(effet.diffuse);
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse specular color
//--------------------------------------------------------------------
bool ColladaParser::ParseSpecular()
{ char *token = "specular";
  if (!FullToken(token))   return false;
  FloatColor(effet.specular);
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse shininess value
//--------------------------------------------------------------------
bool ColladaParser::ParseShininess()
{ char *token = "shininess";
  if (!FullToken(token))   return false;
  FloatValue(&effet.shininess);
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse reflective value
//--------------------------------------------------------------------
bool ColladaParser::ParseReflective()
{ char *token = "reflective";
  if (!FullToken(token))   return false;
  FloatColor(effet.reflective);
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse reflective value
//--------------------------------------------------------------------
bool ColladaParser::ParseTransparent()
{ char *token = "transparent";
  if (!ParseToken(token))   return false;
  char pm1[31];
  ParseAttribute(" opaque = %31[^ ><]s ",pm1,31);
  fscanf(file," > ");
  FloatColor(effet.transparent);
  return ParseEnd(token);
}

//--------------------------------------------------------------------
//  Parse reflectivity value
//--------------------------------------------------------------------
bool ColladaParser::ParseReflectivity()
{ char *token = "reflectivity";
  if (!FullToken(token))   return false;
  FloatValue(&effet.reflectivity);
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse transparency value
//--------------------------------------------------------------------
bool ColladaParser::ParseTransparency()
{ char *token = "transparency";
  if (!FullToken(token))   return false;
  FloatValue(&effet.transparency);
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse refraction value
//--------------------------------------------------------------------
bool ColladaParser::ParseRefraction()
{ float ref;
  char *token = "index_of_refraction";
  if (!FullToken(token))   return false;
  FloatValue(&ref);
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse Effect blinn
//--------------------------------------------------------------------
bool ColladaParser::ParseBlinnEffect()
{ char *token = "blinn";
  if (!FullToken(token))  return false;
  //--------------------------------------
  effet.type  = EFF_BLINN;
  //-------------------------------------
  ParseEmission();
  ParseAmbient();
  ParseDiffuse();
  ParseSpecular();
  ParseShininess();
  ParseReflective();
  ParseReflectivity();
  ParseTransparent();
  ParseTransparency();
  ParseRefraction();
  //---Create a effect node -------------
  Xeffc *nod = new Xeffc();
  *nod       = effet;
  //------------------------------------
  Effcs[effid] = nod;
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Set source property
//--------------------------------------------------------------------
U_INT ColladaParser::SetSrcProperty(char *nm)
{ if (strcmp(nm,"X")        == 0) return NAME_X;
  if (strcmp(nm,"Y")        == 0) return NAME_Y;
  if (strcmp(nm,"Z")        == 0) return NAME_Z;
  if (strcmp(nm,"TIME")     == 0) return NAME_TIME;
  if (strcmp(nm,"ANGLE")    == 0) return NAME_ANGLE;
  return 0;
}
//--------------------------------------------------------------------
//  Parse  parameters
//--------------------------------------------------------------------
bool ColladaParser::ParseParam()
{ char *token = "param";
  if (!ParseToken(token)) return false;
  //---Parse Attributes ------------------
  char pm1[32];         // name
  char pm2[32];         // sid
  char pm3[32];         // type
  char pm4[32];         // semantic
  //---------------------------------------
  bool p1 = ParseAttribute(" name      = %31[^ ></]s ", pm1,31);
  bool p2 = ParseAttribute(" sid       = %31[^ ></]s ", pm2,31);
  bool p3 = ParseAttribute(" type      = %31[^ ></]s ", pm3,31);
  bool p4 = ParseAttribute(" semantic  = %31[^ ></]s ", pm4,31);
  fscanf(file," / >");
  //----------------------------------------
  srce.prop |= SetSrcProperty(pm1);
	if (0 == tr)	return true;
	if (p1)	{sprintf(ztr,"%sname %s",			zlv,pm1); TRACE(ztr,0);	}
	if (p2) {sprintf(ztr,"%sSID  %s",			zlv,pm2); TRACE(ztr,0); }
	if (p3) {sprintf(ztr,"%stype %s",			zlv,pm3); TRACE(ztr,0); }
	if (p4) {sprintf(ztr,"%ssemantic %s",	zlv,pm4); TRACE(ztr,0); }
	zlv++;
  return true;
}
//--------------------------------------------------------------------
//  Parse Accessor parameters
//--------------------------------------------------------------------
bool ColladaParser::ParseAccessor()
{ char *token = "accessor";
  if (!ParseToken(token)) return false;
  //---Parse attributes ------------------
  char pm1[32];         // Count
  char pm2[32];         // offset
  char pm3[32];         // source
  char pm4[32];         // stride
  //---------------------------------------
  bool p1 = ParseAttribute(" count  = %31[^ ><]s ",pm1,31);
  bool p2 = ParseAttribute(" offset = %31[^ ><]s ",pm2,31);
  bool p3 = ParseAttribute(" source = %31[^ ><]s ",pm3,31);
  bool p4 = ParseAttribute(" stride = %31[^ ><]s ",pm4,31);
  fscanf(file," > ");
	//-----------------------------------------
	int nbr = atoi(pm1);
	int str = atoi(pm4);
	if (tr)
	{	sprintf(ztr,"%ssource %s count %d stride %d",zlv,pm3,nbr,str);
		TRACE(ztr,0);
	}
  //----Parse child nodes -------------------
  while (ParseParam())  continue;
  //----Parse End ---------------------------
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse technique common
//--------------------------------------------------------------------
bool ColladaParser::ParseTechniqueCommon()
{ char *token = "technique_common";
  if (!FullToken(token))  return false;
  //----Parse child nodes --------------
  ParseAccessor();
  //----Parse End ----------------------
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Check for surface parameters
//--------------------------------------------------------------------
bool ColladaParser::ParseInitFrom()
{ char *token = "init_from";
  if (!FullToken(token))  return false;
  //------------------------------------------
  fscanf(file," %31[^ ><]s ",effet.texture,31);
  //------------------------------------------
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Check for surface parameters
//--------------------------------------------------------------------
bool ColladaParser::ParseSurface()
{ char *token = "surface";
  if (!ParseToken(token))   return false;
  //-------------------------------------------
  char pm[32];
  ParseAttribute(" type = %31[^></]s ",pm,31);
  fscanf(file," > ");
  ParseInitFrom();
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse Effect new parameter
//--------------------------------------------------------------------
bool ColladaParser::ParseEffectNewParam()
{ char *token = "newparam";
  if (!ParseToken(token))       return false;
  //----Parse sid attribute --------------------
  ParseAttribute(" sid = %31[^ ></>]s ", sid,31);
  fscanf(file," > ");
  //--------------------------------------------
  ParseSurface();
  //---End of parsing --------------------------
  return SkipToEnd(token);
}
//--------------------------------------------------------------------
//  Parse Effect technique
//--------------------------------------------------------------------
bool ColladaParser::ParseEffectTechnique()
{ char *token = "technique";
  if (!ParseToken(token))       return false;
  ParseAttribute(" sid = %31[^><]s ",sid,31);
  fscanf(file," > ");
  //---Parse options -----------------------
  while (ParseEffectNewParam()) continue;
  ParseBlinnEffect();
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse Effect profile common
//--------------------------------------------------------------------
bool ColladaParser::EffectProfileCommon()
{ char *token = "profile_COMMON";
  if (!FullToken(token)) return false;
  //-------------------------------------
  while (ParseEffectNewParam()) continue;
  ParseEffectTechnique();
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse Effect profile
//--------------------------------------------------------------------
bool ColladaParser::ParseEffectProfile()
{ bool pf  = EffectProfileCG();
       pf |= EffectProfileCommon();
  return pf;
}
//--------------------------------------------------------------------
//  Parse Effect
//--------------------------------------------------------------------
bool ColladaParser::ParseEffect()
{ char *token = "effect";
  if (!ParseIdName(token,effid,name)) return false;
  //----Parse profile -------------------------
  effet.Init();
  ParseEffectProfile();
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse Library image
//--------------------------------------------------------------------
bool ColladaParser::ParseLibraryImage()
{ char *token = "library_images";
  if (!ParseIdName(token,id,name))  return false;
  //---Parse child nodes ----------------------
  while (ParseImage())              continue;
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse instance Material
//--------------------------------------------------------------------
bool ColladaParser::InstanceMaterial()
{ Pos = ftell(file);
  int nf = fscanf(file," < instance_effect url = %31[^ ></]s ",url);
  if (1 != nf)  return Backup();
  fscanf(file, " / > ");
  ExtractName(url,matr.effet,32);
  return true;
}
//--------------------------------------------------------------------
//  Parse Material
//--------------------------------------------------------------------
bool ColladaParser::ParseMaterial()
{ char *token = "material";
  if (!ParseIdName(token,matid,name)) return false;
  //--Parse child nodes ---------------------
  matr.Init();
  InstanceMaterial();
  //----Add a material node -----------------
  Xmatr *nod  = new Xmatr();
  matr.type   = MAT_EFFECT;
  *nod        = matr;
  Matrs[matid]= nod;
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse a Mesh source
//--------------------------------------------------------------------
bool ColladaParser::ParseSource()
{ char *token = "source";
  if (!ParseIdName(token,srcid,name)) return false;
  srce.prop   = 0;
  //---Parse child nodes---------------------
  ParseFloatArray();
  ParseTechniqueCommon();
  //---Save a node mesh ---------------------
  Xsrce *nod  = new Xsrce();
  nod->prop   = srce.prop;
  nod->nbv    = srce.nbv;
  nod->vtab   = srce.vtab;
  Nodes[srcid]= nod;
  //-----------------------------------------
  srce.Clear();
  //---Parse end ----------------------------
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse input statement
//--------------------------------------------------------------------
bool ColladaParser::ParseInput()
{ char *token = "input";
  if (!ParseToken(token))   return false;
  //----Parse attributes ---------------------
  bool p1 = ParseAttribute(" offset   = %31[^ ></]s ",offset,   31);
  bool p2 = ParseAttribute(" semantic = %31[^ ></]s ",semantic, 31);
  bool p3 = ParseAttribute(" source   = %31[^ ></]s ",source,   31);
  bool p4 = ParseAttribute(" set      = %31[^ ></]s ",set   ,   31); 
  fscanf(file," / > ");
  //-------------------------------------------
	if (0 == tr)							return true;
	if (p1) {sprintf(ztr,"%s offset %s",		zlv,offset);		TRACE(ztr,0);}
	if (p2) {sprintf(ztr,"%s semantic %s",	zlv,semantic);	TRACE(ztr,0);}
	if (p3)	{sprintf(ztr,"%s source %s",		zlv,source);		TRACE(ztr,0);}
	if (p4)	{sprintf(ztr,"%s set %s",				zlv,set);				TRACE(ztr,0);}
  zlv++;
  return true;
}
//--------------------------------------------------------------------
//  Locate input Key
//--------------------------------------------------------------------
bool ColladaParser::GetInputKey(Xsamp *smp)
{ if (strcmp(semantic,"INPUT") != 0)  return false;
  //---Locate the input source ---------------
  char *key = source + 1;
  std::map<std::string,Xnode*>::iterator it = Nodes.find(key);
  if (it == Nodes.end())    gtfo("INPUT source not found");
  Xsrce *nod = (Xsrce*)(*it).second;
  if (nod->IsNoTIME())      gtfo("INPUT source is not TIME");
  //------------------------------------------
  smp->inpt = nod;
  return true;
}
//--------------------------------------------------------------------
//  Locate output Key
//--------------------------------------------------------------------
bool ColladaParser::GetOutputKey(Xsamp *smp)
{ if (strcmp(semantic,"OUTPUT") != 0)  return false;
  //---Locate the input source ---------------
  char *key = source + 1;
  std::map<std::string,Xnode*>::iterator it = Nodes.find(key);
  if (it == Nodes.end())    gtfo("OUTPUT source not found");
  Xsrce *nod = (Xsrce*)(*it).second;
  //------------------------------------------
  smp->outp = nod;
  return true;
}

//--------------------------------------------------------------------
//  Parse Sampler
//--------------------------------------------------------------------
bool ColladaParser::ParseSampler()
{ char *token = "sampler";
  if (!ParseIdName(token,id,name))  return false;
  //----Allocate a sampler ------------------
  Xsamp *nod = new Xsamp();
  //-----Parse Child Node -------------------
  while (ParseInput())
  { if (GetInputKey(nod))      continue;
    if (GetOutputKey(nod))     continue;
  }
  //-----------------------------------------
  Nodes[id] = nod;
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Check for rotation
//  Note:  This will be product dependent
//--------------------------------------------------------------------
U_INT ColladaParser::GetTransformation(char *trf,char *cmp)
{ if (0 == strncmp(trf,"rotateX",7))    return ROT_X;
  if (0 == strncmp(trf,"rotateY",7))    return ROT_Y;
  if (0 == strncmp(trf,"rotateZ",7))    return ROT_Z;
  if (0 != strncmp(trf,"translate",9))  return 0;
  if (0 == strcmp(cmp,"X"))             return TRANS_X;
  if (0 == strcmp(cmp,"Y"))             return TRANS_Y;
  if (0 == strcmp(cmp,"Z"))             return TRANS_Z;
  return 0;
}
//--------------------------------------------------------------------
//  Parse a channel
//--------------------------------------------------------------------
bool ColladaParser::ParseChannel()
{ char *token = "channel";
  if (!ParseToken(token))   return false;
  //---Parse attributes ----------------------------
  char tgt[32] = {0};
  char tfm[32] = {0};
  char cmp[32] = {0};
  ParseAttribute(" source = %31[^ ></]s ",source,31);
  ParseAttribute(" target = %31[^ ></]s ",tgt,   31);
  ParseAttribute("/%31[^ ></.]s ", tfm,31);
  ParseAttribute(".%31[^ ></]s " , cmp,31);
  fscanf(file," / >");
  //--------build  keyframe ----------------------
  Xanim *kfm  = new Xanim();
  kfm->trans  = GetTransformation(tfm,cmp);
  //---Locate the sampler -------------------------
  char *skey  = source + 1;
  std::map<std::string,Xnode *>::iterator it = Nodes.find(skey);
  if (it == Nodes.end()) gtfo("No sampler %s",skey);
  Xsamp *smp  = (Xsamp*)(*it).second;
  //----Locate the INPUT source of animation ------
  Xsrce *nf0  = smp->inpt;
  kfm->tim0   = nf0->vtab[0];
  kfm->tim1   = nf0->vtab[1];
  Xsrce *nf1  = smp->outp;
  //---Check for transformation --------------------
  kfm->val0   = nf1->vtab[0];
  kfm->val1   = nf1->vtab[1];
  //----Save animation with target node -------------
  strncpy(kfm->target,tgt,31);
  Anims.push_back(kfm);
  //-------------------------------------------------
  zlv++;
  return true;
}
//--------------------------------------------------------------------
// Create a redirection node
//--------------------------------------------------------------------
void ColladaParser::RedirectSource()
{ Xrdir *nod = new Xrdir();
  char *dst  = nod->ident;
  char *src  = source + 1;
  //---Store ident ------------------
  while (*src)  *dst++ = *src++;
  *dst = 0;
  Nodes[id]  = nod;
  return;
}
//--------------------------------------------------------------------
//  Parse vertice statement
//--------------------------------------------------------------------
bool ColladaParser::ParseVertices()
{ char *token = "vertices";
  if (!ParseIdName(token,id,name))   return false;
  //---Parse child nodes --------------------
  while (ParseInput())
  { if (strcmp(semantic,"POSITION") != 0) continue;
    RedirectSource();
  }
  //---End of statement ---------------------
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse p statement
//  Build a table of integers. Each integer is an index in one of
//	the table (vertex,norma,texture) for one vertex of one polygon
//--------------------------------------------------------------------
bool ColladaParser::ParsePindices()
{ char *token = "p";
  if (!FullToken(token))  return false;
  //----------------------------------------------------
  int dim = vcnt * nbix;					// Allocate room for 
  ppol = new int[dim];
  for (int k=0; k< dim; k++) fscanf(file," %d ",ppol + k);
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse vcount statement
//  Build a table of integers
//--------------------------------------------------------------------
bool ColladaParser::ParseVCount()
{ char *token = "vcount";
  if (!FullToken(token)) return false;
  int ns;
  vcnt		=	0;
  render	= GL_QUADS;
  //------------------------------------------
  for (int k=0; k< qcnt; k++)
  { fscanf(file," %d ",&ns);
    vcnt    += ns;
  }
  //------------------------------------------
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//	Parse geometric parameters
//--------------------------------------------------------------------
bool ColladaParser::ParseGeomItem()
{ nbix  = 0;
  ppol  = 0;
  vcnt  = 0;								// Vertex count
	//---Clear indicators ----------------------
	indv	= 0;
	indn	= 0;
	indt	= 0;
  //---Parse attributes ----------------------
  strcpy(count,"0");
  bool p1 = ParseAttribute(" name     = %31[^ ></]s ",name,    31);
  bool p2 = ParseAttribute(" count    = %31[^ ></]s ",count,   31);
  bool p3 = ParseAttribute(" material = %31[^ ></]s ",material,31);
  fscanf(file," > ");
	//-------------------------------------------
	if (tr)
	{	if (p1) {sprintf(ztr,"%s name %s",zlv,name);				TRACE(ztr,0);}
		if (p2) {sprintf(ztr,"%s count %s", zlv,count);			TRACE(ztr,0);}
		if (p3) {sprintf(ztr,"%s material %s",zlv,material);TRACE(ztr,0);}
	}
	zlv++;
  //---Compute polygon count ------------------
  qcnt  = atoi(count);
	//---Indicators ----------------------------
  vnod  = 0;
  nnod  = 0;
  xnod  = 0;
	//---Offset in tables -----------------------
	vofs	= 0;
	nofs	= 0;
	tofs	= 0;
	//---Parse child nodes ---------------------
  char *key = source+1;
  while (ParseInput())
  { if (GetVertex(semantic,key)) continue;  // Locate vertex
    if (GetNormal(semantic,key)) continue;  // Locate normal
    if (GetTextST(semantic,key)) continue;  // Locate texture coordinates    
  }

	return true;
}
//--------------------------------------------------------------------
//	Parse a triangle array
//--------------------------------------------------------------------
bool ColladaParser::ParseTriangle()
{	char *token = "triangles";
  if (!ParseToken(token))   return false;
  //------------------------------------------
  ParseGeomItem();
	vcnt	= 3 * qcnt;										// Total vertices
	ParsePindices();										// Parse indices for vertex
	BuildC3PartNode();									// Build the final vertex table
	return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse polylist
//	Polylist contains all item o build a part
//	<polylist count="3" material="Material1">
//		<input offset="0" semantic="VERTEX" source="vertTable"/>
//		<input offset="1" semantic="NORMAL" source="normTable"/>
//		<vcount> 4 4 3 </vcount>
//		<p> 0 0  2 1  32  13 .... </p>
//	This defines coordinates for 3 polygons: 2 Quad and a Triangle (4 4 3 in vcount)
//	p defines indices in vertTable and normTable for each vertex in each polygon
//	 0 0	indices for vertex 0 in Q0
//	 2 1  indices for vertex 1 in Q1 (2 in vertTable, 1 in normTable)
//		
//--------------------------------------------------------------------
bool ColladaParser::ParsePolylist()
{ char *token = "polylist";
  if (!ParseToken(token))   return false;
  //------------------------------------------
	ParseGeomItem();
  ParseVCount();											//	Parse polygon structure
  ParsePindices();										//	Parse indices for vertex
  //------------------------------------------
  BuildC3PartNode();
  //------------------------------------------
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse a mesh entry
//--------------------------------------------------------------------
bool ColladaParser::ParseMeshEntry()
{if (ParsePolylist())	return true;
 if (ParseTriangle())	return true;
 return false;
}
//--------------------------------------------------------------------
//  Parse a mesh
//--------------------------------------------------------------------
bool ColladaParser::ParseMesh()
{ char *token = "mesh";
  if (!FullToken(token))  return false;
  //---Parse child nodes ------------------------
  render =  GL_TRIANGLES;
  while (ParseSource())   continue;
  ParseVertices();
  ParseMeshEntry();
  //---Parse end --------------------------------
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse Geometry
//--------------------------------------------------------------------
bool ColladaParser::ParseGeometry()
{ char *token = "geometry";
  if (!ParseIdName(token,geoid,name)) return false;
  //---Parse child nodes -----------------------
  ParseMesh();
  //---Parse end -------------------------------
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse animation section
//--------------------------------------------------------------------
bool ColladaParser::ParseAnimation()
{ char *token = "animation";
  if (!ParseIdName(token,id,name))    return false;
  //----Parse Child nodes ----------------------
  while (ParseAnimation())            continue;
  while (ParseSource())               continue;
  ParseSampler();
  ParseChannel();
  //----Parse end ------------------------------
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse Library effect
//--------------------------------------------------------------------
bool ColladaParser::ParseLibraryEffect()
{ char *token = "library_effects";
  if (!ParseIdName(token,id,name))  return false;
  //---Parse child nodes -----------------------
  while (ParseEffect())   continue;
  //---Parse end -------------------------------
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse Library Material
//--------------------------------------------------------------------
bool ColladaParser::ParseLibraryMaterial()
{ char *token = "library_materials";
  if (!ParseIdName(token,id,name))  return false;
  //---Parse child nodes ---------------------
  while (ParseMaterial())           continue;
  //---Parse end -----------------------------
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse Library geometry
//--------------------------------------------------------------------
bool ColladaParser::ParseLibraryGeometry()
{ char *token = "library_geometries";
  if (!ParseIdName(token,id,name))  return false;
  while (ParseGeometry())           continue;
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse library animation
//--------------------------------------------------------------------
bool ColladaParser::ParseLibraryAnimations()
{ char *token = "library_animations";
  if (!ParseIdName(token,id,name))  return false;
  while (ParseAnimation())          continue;
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Set material to part
//--------------------------------------------------------------------
void ColladaParser::SetMaterial(CAcmPart *part,char *kmat)
{ std::map<std::string,Xmatr*>::iterator im = Matrs.find(kmat);
  if (im == Matrs.end())          return;
  Xmatr *nmat = (*im).second;
  //---Get effect -------------------------------
  if (nmat->type != MAT_EFFECT)   return;
  char *kef = nmat->effet+1;
  std::map<std::string,Xeffc*>::iterator ef = Effcs.find(kef);
  if (ef == Effcs.end())          return;
  Xeffc *nef = (*ef).second;
  //----Set other properties ----------------------
  bool trs = (nef->transparency > 0.01);
  if (trs)  part->SetAlpha(1-nef->transparency);
  part->SetDiffuse(true);
  //---Get texture if any ------------------------
  if (0 == *nef->texture)         return;
  char  *kim = nef->texture;
  std::map<std::string,Ximag*>::iterator ig = Imags.find(kim);
  Ximag *nim = (*ig).second;
  //-----Load texture from texture ward -----------
  char *tname = nim->tname;
  void *ref = globals->txw->GetM3DPodTexture(tname,0);
  U_INT obj = globals->txw->Get3DObject(ref);
  part->SetTexREF(ref,obj);
  return;
}
//--------------------------------------------------------------------
//  Parse Bind_Vertex_Input in technique material
//--------------------------------------------------------------------
bool ColladaParser::ParseBindVertex()
{	char *token = "bind_vertex_input";
  if (!ParseToken(token))		return false;
	//---extract attributes ----------------------
	char pm1[64];						//semantic
	char pm2[64];						//input semantic
	char pm3[16];						//input_set
	//----Parse attibutes -------------------------
	ParseAttribute(" semantic = %63[^ ></]s ",      pm1, 63);
	ParseAttribute(" input_semantic = %63[^ ></]s ",pm2, 63);
	ParseAttribute(" input_set = %63[^  ></]s ",    pm3, 63);
	//----Bypass end of statement ----------------
	fscanf(file, " / > ");
	return true;
}
//--------------------------------------------------------------------
//  Parse instance material technique
//--------------------------------------------------------------------
bool ColladaParser::ParseInstanceMaterial(CAcmPart *part)
{ char *token = "instance_material";
  if (!ParseToken(token))   return false;
  //---Get attributes -------------------------
  char pm1[64];     // symbol
  char pm2[64];     // Target
  //--------------------------------------------
  ParseAttribute(" symbol = %63[^ ></]s ",pm1, 63);
  ParseAttribute(" target = %63[^ ></]s ",pm2, 63);
  fscanf(file," > ");
	while (ParseBindVertex())		continue;
  //----Locate matepolyrial -----------------------
  SetMaterial(part,pm2+1);
  return SkipToEnd(token);
}
//--------------------------------------------------------------------
//  Parse material technique
//--------------------------------------------------------------------
bool ColladaParser::ParseMaterialTechnique(CAcmPart *part)
{ char *token = "technique_common";
  if (!FullToken(token))  return false;
  ParseInstanceMaterial(part);
  //-------------------------------------------
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse bind material
//--------------------------------------------------------------------
bool ColladaParser::ParseBindMaterial(CAcmPart *part)
{ char *token = "bind_material";
  if (!FullToken(token))  return false;
  //---Get child node ---------------------
  ParseMaterialTechnique(part);
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse instance geometry
//	A part node is created
//--------------------------------------------------------------------
bool ColladaParser::InstanceGeometry(CAcmPart *part)
{ char *token = "instance_geometry";
  if (!ParseToken(token))   return false;
  //---Get the vertices array--------------------
  char pm1[32];
  ParseAttribute(" url = %31[^ >< /]s ", pm1,31);
  fscanf(file," > ");
  //---------------------------------------------
  ParseBindMaterial(part);
  //--Get the geometry --------------------------
  char *gkey = pm1 + 1;
  std::map<std::string,Xnode *>::iterator it = Nodes.find(gkey);
  if (it == Nodes.end())   gtfo("No geometry %s",gkey);
  Xgeom *nod = (Xgeom *)(*it).second;
  part->RenderAs(nod->rend);
	//--- Create a pack with this part ------------
	pak	= new CAcmVPack(part,nod);
	pack.push_back(pak);
	nbVT	+= nod->nbv;
	//---------------------------------------------
  nod->Clear();
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse scale
//  this is skipped
//--------------------------------------------------------------------
bool ColladaParser::ParseScale(CAcmPart *part)
{ char *token = "scale";
  if (!ParseToken(token))       return false;
  //--------------------------------------------
  float fx;
  float fy;
  float fz;
  //--------------------------------------------
  ParseAttribute(" sid = %31[^ ></]s ",sid,31);
  fscanf(file," > ");
  fscanf(file," %f %f %f ",&fx,&fy,&fz);
  fscanf(file," < / scale > ");
  //--------------------------------------------
  zlv++;
  return true;
}
//--------------------------------------------------------------------
//  Parse rotation
//  ORIENTATION sensitive
//--------------------------------------------------------------------
bool ColladaParser::ParseRotation(CAcmPart *part)
{ char *token = "rotate";
  if (!ParseToken(token))       return false;
  //----Parse Attributes -----------------------
  char pm1[32];
  float fx;
  float fy;
  float fz;
  float an;
  //--------------------------------------------
  ParseAttribute(" sid = %31[^ ></]s ",pm1,31);
  fscanf(file," > ");
  fscanf(file," %f %f %f %f ",&fx,&fy,&fz,&an);
  fscanf(file," < / rotate > ");
  //--------------------------------------------
  U_INT axis = GetTransformation(pm1,0);
  if (ROT_X == axis)  part->SetPitch(+an);
  if (ROT_Z == axis)  part->SetBank (+an);
  if (ROT_Y == axis)  part->SetHead (+an);
  //--------------------------------------------
  zlv++;
  return true;
}
//--------------------------------------------------------------------
//  Parse translation
//  ORIENTATION sensitive
//--------------------------------------------------------------------
bool ColladaParser::ParseTranslation(CAcmPart *part)
{ char *token = "translate";
  if (!ParseToken(token))         return false;
  //---Parse attribute -------------------------
  char pm1[32];
  float f0;
  float f1;
  float f2;
  //------Invert Y and Z------------------------
  ParseAttribute(" sid = %31[^ ></]s ",pm1,31);
  fscanf(file," > ");
  fscanf(file," %f %f %f > ",&f0,&f1,&f2);
  fscanf(file," < / translate > ");
  CVector vt(f0,-f2,f1);
  part->CopyTranslation(vt);
  zlv++;
  return true;
}
//--------------------------------------------------------------------
//  Parse node transformations
//--------------------------------------------------------------------
bool ColladaParser::ParseTransformations(CAcmPart *part)
{ if (ParseTranslation(part))     return true;
  if (ParseRotation(part))        return true;
  if (ParseScale(part))           return true;
  return false;
}
//--------------------------------------------------------------------
//  Parse a node
//--------------------------------------------------------------------
bool ColladaParser::ParseNode()
{ char *token = "node";
  if (!ParseToken(token))       return false;
  //---Parse attributes ------------------------
  char nid[32];                 // Node ID
  char nam[64];                 // Name
  char sid[32];                 // sid
  char typ[32];                 // type
  //--------------------------------------------
  bool p1 = ParseAttribute(" id      = %31[^ ></]s ", nid,31);
  bool p2 = ParseAttribute(" name    = %31[^ ></]s ", nam,63);
  bool p3 = ParseAttribute(" sid     = %31[^ ></]s ", sid,31);
  bool p4 = ParseAttribute(" type    = %31[^ ></]s ", typ,31);
  fscanf(file," > ");
	//-------------------------------------------
	if (tr)
	{	if (p1) {sprintf(ztr,"%s id %s",				zlv,nid);		TRACE(ztr,0);}
		if (p2) {sprintf(ztr,"%s name %s",			zlv,nam);		TRACE(ztr,0);}
		if (p3) {sprintf(ztr,"%s sid  %s",			zlv,sid);		TRACE(ztr,0);}
		if (p4) {sprintf(ztr,"%s type %s",			zlv,typ);		TRACE(ztr,0);}
	}
  //----allocate a part -----------------------
  CAcmPart *part = new CAcmPart(model,31,0,1);
  part->setName(nam);
  model->AddPart(Seq,pSeq,part);
  //----Parse transformations -------------------
  while (ParseTransformations(part))    continue;
  InstanceGeometry(part);
  //----Build keyframes -------------------------
  MakeKeyframe(part,nid);
  //--- Parse child nodes ----------------------
  pSeq  = Seq++;
  while (ParseNode())   continue;
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse the only allowed scene
//--------------------------------------------------------------------
bool ColladaParser::ParseVisualScene()
{ char *token = "visual_scene";
  if (!ParseIdName(token,id,name))  return false;
  ParseNode();                // Parse the body node
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Parse library visual scenes
//--------------------------------------------------------------------
bool ColladaParser::ParseLibraryVisualScene()
{ char *token = "library_visual_scenes";
  if (!ParseIdName(token,id,name))  return false;
  ParseVisualScene();
  return ParseEnd(token);
}
//--------------------------------------------------------------------
//  Make Keyframe for the same node same time
//--------------------------------------------------------------------
Xanim *ColladaParser::NextAnimation(char *idn,CKeyFrame *key)
{ std::vector<Xanim*>::iterator it;
  for (it = Anims.begin(); it != Anims.end(); )
  { Xanim *nod = (*it);
    if (strcmp(idn,nod->target))  continue;
    if (nod->tim1 != key->frame)  continue;
    Anims.erase(it);
    return nod;
  }
  return 0;
}
//--------------------------------------------------------------------
//  Make Keyframe for the same node same time
//--------------------------------------------------------------------
bool ColladaParser::BuildKeyframes(char *idn)
{ std::vector<Xanim*>::iterator it;
  Xanim *nod = 0;
  for (it = Anims.begin(); it != Anims.end(); it++)
  { nod = (*it);
    if (strcmp(idn,nod->target))  continue;
    //----Found a candidate ----------------
    Anims.erase(it);
    //-----Build a keyframe with target ----
    CKeyFrame *key = new CKeyFrame();
    key->frame = nod->tim1;
    nod->Assign(key);
    //-----Extract target node -------------
    delete nod;
    //--------------------------------------
    while (nod = NextAnimation(idn,key)) nod->Assign(key);
    //---Add a new keyframe ----------------
    nKeys.push_back(key);
    //--------------------------------------
    return true;
  }
  //-----No keyframe for this idn ----------
  return false;
}
//--------------------------------------------------------------------
//  Copy key when transform is nul
//--------------------------------------------------------------------
void ColladaParser::CopyKeys(CKeyFrame &prv,CKeyFrame &kfn)
{ if (kfn.dx == 0)  kfn.dx = prv.dx;
  if (kfn.dy == 0)  kfn.dy = prv.dy;
  if (kfn.dz == 0)  kfn.dz = prv.dz;
  if (kfn.p  == 0)  kfn.p  = prv.p;
  if (kfn.b  == 0)  kfn.b  = prv.b;
  if (kfn.h  == 0)  kfn.h  = prv.h;
  //-------------------------------------
  return;
}
//--------------------------------------------------------------------
//  Norme transformation
//--------------------------------------------------------------------
void ColladaParser::NormeKey(CKeyFrame &kfn)
{ if (kfn.p > 359.99)   kfn.p = 0;
  if (kfn.b > 359.99)   kfn.b = 0;
  if (kfn.h > 359.99)   kfn.h = 0;
}
//--------------------------------------------------------------------
//  Make Keyframe for the part
//--------------------------------------------------------------------
bool ColladaParser::MakeKeyframe(CAcmPart *part,char *id)
{ //----Init the first key -------------------------
  CKeyFrame *kfp = 0;             // previous keyframe
  CKeyFrame *kf1 = 0;             // End keyframe
  CKeyFrame *kf0 = part->GetCopyKey();
  kf0->frame = 0;
  nKeys.push_back(kf0);
  while (BuildKeyframes(id))    continue;
  int nb     = nKeys.size();
  //--If only one key, add a last ------------------
  if (1 == nb)
  { kf1 = part->GetCopyKey();
    kf1->frame = 1;
    nKeys.push_back(kf1);
  }
  //---Sort on time --------------------------------
  std::sort(nKeys.begin(), nKeys.end(), SortKeyframe);
  //---Extract the last keyframe -------------------
  nb  = nKeys.size();
  kf1 = nKeys[nb-1];
  float etm = kf1->frame;
  //---Add all keyframes to the part ---------------
  part->AllocateKeyframe(nb);
  for (int k = 0; k < nb; k++)
  { CKeyFrame *kfn = nKeys[k];
    NormeKey(*kfn);
    float rat  = kfn->frame / etm;
    kfn->frame = rat;
    if (kfp) CopyKeys(*kfp,*kfn);
//    model->CheckGap(kfp,kfn);
    kfp = part->addKeyframe(*kfn);
    kfn->Trace();
    delete kfn;
  }
  //----Clear keys ---------------------------------
  nKeys.clear();
  return true;
}
//--------------------------------------------------------------------
//  Parse top node
//--------------------------------------------------------------------
bool ColladaParser::TopNode()
{ if (ParseAsset())               return true;
  if (ParseLibraryImage())        return true;
  if (ParseLibraryEffect())       return true;
  if (ParseLibraryMaterial())     return true;
  if (ParseLibraryGeometry())     return true;
  if (ParseLibraryAnimations())   return true;
  if (ParseLibraryVisualScene())  return true;
  return false;
}
//--------------------------------------------------------------------
//  Parse xml line
//--------------------------------------------------------------------
bool ColladaParser::ParseXML()
{ char a1[128];
  char a2[128];
  bool ok  = 1; 
       ok &= (1 == fscanf(file," < ?xml version= %12s > ",a1));
       ok &= (2 == fscanf(file," < COLLADA xmlns= %126s version= %16[^>]s",a1,a2));
       fscanf(file," > ");
  if (!ok)          return false;

  while (TopNode()) continue;
	model->BuildVBO(nbVT,pack);
  return true;
}
//=========================END 0F FILE ====================================================
