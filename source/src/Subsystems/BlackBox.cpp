//===============================================================================
// BlackBox.cpp
//
//
// Part of Fly! Legacy project
//
// Copyright 2005 Chris Wallace
// CopyRight 2007 Jean Sabatier
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
//===============================================================================
#include "../Include/Globals.h"
#include "../Include/Fui.h"
//===============================================================================
//	Master Block box
//===============================================================================
//-----------------------------------------------------------
//	Return blck box for Tcache
//-----------------------------------------------------------
BBcache *BBM::CreateTcmBox(char *n)
{ BBcache *bb = new BBcache(n);
	boxes.push_back(bb);
	return bb;
}
//-----------------------------------------------------------
//	Return blck box for airport
//-----------------------------------------------------------
BBairport *BBM::CreateAptBox(char *n)
{ BBairport *bb = new BBairport(n);
	boxes.push_back(bb);
	return bb;
}

//-----------------------------------------------------------
//	Destroy all resources 
//-----------------------------------------------------------
void BBM::DumpAll()
{	std::vector<BlackBox*>::iterator it;
	for (it=boxes.begin(); it!=boxes.end(); it++)	(*it)->Dump();
	return;
}
//===============================================================================
//	Block box construction
//===============================================================================
BlackBox::BlackBox()
{	ind		= 0;
	dim		= BB_SIZE;
	use		= 0;
	for (int k=0; k < dim; k++)
		{ buf[k].frame  = 0;
			buf[k].action = "NONE";
			buf[k].item		= 0;
			buf[k].pm1		= 0;
	}
}
//-----------------------------------------------------------
//	Trace one slot 
//-----------------------------------------------------------
void	BlackBox::Enter(char *a,void *p1, U_INT p2)
{	BB_SLOT *s = buf + ind++;
	s->frame	= globals->Frame;
	s->action	= a;
	s->item		= p1;
	s->pm1		= p2;
	ind &= BB_MASK;
	return;
}
//-----------------------------------------------------------
//	Level 1 dump
//-----------------------------------------------------------
void BlackBox::Dump()
{	TRACE("===========> BlackBox %s",name);
  int    k  = ind;
	bool	go	= true;
	while (go)
	{	
		TRACE("   Frame %010u %s p1=%08x p2=%u",
						buf[k].frame,
						buf[k].action,
						buf[k].item,
						buf[k].pm1);
	 k	= (k+1) & BB_MASK;
	 if (k == ind)	break;
	}
	return;
}
//-----------------------------------------------------------
//	EDit BBox
//-----------------------------------------------------------
void BlackBox::Probe(CFuiCanva *cnv,int nb)
{ int		k = ind;
	int		n = nb;
	while (n--)
	{	cnv->AddText(1,buf[k].action,1);
	  k	= (k+1) & BB_MASK;
	  if (k == ind)	k = 0;
	}
	return;
}
//===============================================================================
//	TCM Edit blackBox
//===============================================================================
void	BBcache::Edit()
{}
//===============================================================================
//	Airport Edit blackBox
//===============================================================================
void	BBairport::Edit()
{}

//====================== END OF FILE ============================================
