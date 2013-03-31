//============================================================================================
// Compressor.cpp Compress texture based on Crunch Library (Open Source)
//============================================================================================
// Part of Fly! Legacy project
//
// Copyright 2005 Chris Wallace
// Copyright 2007 Jean Sabatier
// Fly! Legacy is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
// Fly! Legacy is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//   along with Fly! Legacy; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//===========================================================================================
//	CRUNCH Library is developped by Rich Geldreich (<richgel99@gmail.com)
//	and was kind enough to support the project.  Check it at http://code.google.com/p/crunch/
//-------------------------------------------------------------------------------------------
//============================================================================================
#include "../Include/Compression.h"
#include "../Include/globals.h"
#include "../Include/TerrainTexture.h"
#include "../Include/TerrainCache.h"
//-------------------------------------------------------------------------------------------
#include "crn_decomp.h"
using namespace crnd;
//===========================================================================================
//	Compressor code 
//===========================================================================================
U_INT intFORMAT[] =
{	GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
};
//===========================================================================================
//	Compressor constructor 
//===========================================================================================
Compressor::Compressor(U_INT t,U_INT m)
{ mip		= m - 1;
  mode	= t;
	crnd	= 0;
	ctx		= 0;
	xOBJ	= 0;
	memset(img, 0, sizeof(img));
}
//------------------------------------------------------------------------------
//	Compress RGBA memory buffer
//NOTE:  Resquested Quality should be encoded in txd.bpp
//------------------------------------------------------------------------------
void Compressor::EncodeCRN(TEXT_INFO &txd)
{	GLubyte *rgba =   txd.mADR;
	if (0 == rgba)		return;
	//--- Prepare compression parameters -----------------------------
	crn_comp_params		mp0; 
	crn_mipmap_params pm1;
	crn_uint32				sze;						// Returned size
	crn_uint32        qty;						// Returned quality
	float							bitr;						// Returned bitrate
	//--- Compression parameters --------------------------------------
	mp0.m_file_type			= cCRNFileTypeCRN;
	mp0.m_faces					= 1;
	mp0.m_width					= txd.wd;
	mp0.m_height				= txd.ht;
	mp0.m_levels				= 1;
	mp0.m_format				= cCRNFmtDXT5;
	mp0.m_flags					= cCRNCompFlagPerceptual | cCRNCompFlagHierarchical | cCRNCompFlagUseBothBlockTypes;
	mp0.m_target_bitrate = 0.0f;
  mp0.m_quality_level  = txd.qty;								///cCRNMaxQualityLevel;
  mp0.m_dxt1a_alpha_threshold = 128;
  mp0.m_dxt_quality = cCRNDXTQualityUber;
  mp0.m_dxt_compressor_type = cCRNDXTCompressorCRN;
  mp0.m_alpha_component = 3;
	mp0.m_pImages[0][0]	  = (U_INT *)rgba;
 	//---Mipmap parameters      --------------------------------------
	pm1.m_mode				= cCRNMipModeGenerateMips;
	pm1.m_max_levels	= mip + 1;
	pm1.m_gamma_filtering = false;
	//--- Compression ------------------------------------------------ 
  txd.mADR	= (GLubyte*)crn_compress(mp0,pm1,sze, &qty, &bitr);
	txd.dim		= sze;
	delete [] rgba;
	return;
}
//------------------------------------------------------------------------------
//	Free resources and abort 
//------------------------------------------------------------------------------
U_INT Compressor::Clean()
{	if (ctx)	crnd_unpack_end(ctx);
	//for (U_INT k = 0; k < cCRNMaxLevels; k++)
	for (U_INT k = 0; k < txd.m_levels; k++)
	{	void *m = img[0][k];
		if (m)	free(m);
	}
	return 0;
}
//------------------------------------------------------------------------------
//	Decode a CRN file 
//	l0:			Resolution level:		0 => max resolution R, 
//														1 => R/2 resolution
//	ln:			is the maximum mip level (excluded) to decode and load
//	mode:		Is the DTX format
//------------------------------------------------------------------------------
U_INT Compressor::DecodeCRN(void *data, U_INT sze,U_CHAR l0, U_CHAR ln)
{ bool ok = crnd_get_texture_info(data, sze, &txd);
	if (!ok)		
			return Clean();
	//--- Start decoding -----------------------------
	crnd	= data;
	ctx = crnd_unpack_begin(data, sze);
	if (!ctx)		
			return Clean();
	//--- Allocate a texture object ------------------
	glGenTextures(1,&xOBJ);
  glBindTexture(GL_TEXTURE_2D,xOBJ);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,(ln-1));
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	int lk	= l0;
	//--- Extract the texture for each mip level------------------------
	for (U_INT inx = 0; inx < ln; inx++)			// Was txd.m_levels
   {  // Compute the face's width, height, number of DXT blocks per row/col, etc.
      const U_INT wd = max(1U, txd.m_width  >> lk);		// Width of level k
      const U_INT ht = max(1U, txd.m_height >> lk);		// height of level k
      const U_INT bx = max(1U, (wd + 3) >> 2);					// Block X
      const U_INT by = max(1U, (ht + 3) >> 2);					// Block Y
      const U_INT rp = bx * crnd::crnd_get_bytes_per_dxt_block(txd.m_format);
      const U_INT tf = rp * by;													// Total face size
     
      siz[lk] = tf;
			//--- Process the face ---------------------------------------
      void *p = malloc(tf);
//      if (!p)		return Clean();
      img[0][lk] = p;				// Face pointer 
      //--- Prepare the face pointer array needed by crnd_unpack_level().
			void *pf[1];
			pf[0] = img[0][lk];
      //--- Now transcode the level to raw DXTn ---------------------
      if (!crnd::crnd_unpack_level(ctx, pf, tf, rp, lk)) return Clean();
			//--- Now feed the GPU with compressed texture ---------------
			glCompressedTexImage2D(GL_TEXTURE_2D,inx,intFORMAT[mode],wd,ht,0, siz[lk], pf[0] );
			lk++;
  //----Check for error -----------------------------------------
	
		{	GLenum e = glGetError ();
			if (e != GL_NO_ERROR) 
						WARNINGLOG ("OpenGL Error 0x%04X : %s", e, gluErrorString(e));
		}
		//---------------------------------------------------------------------
   }
	Clean();
	return xOBJ;
}

//=====================END OF FILE ==========================================================
