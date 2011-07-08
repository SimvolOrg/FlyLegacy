/*
 * TerrainTile.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
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

/*! \file TerrainTile.cpp
 *  \brief Implements CTerrainTile base class for all terrain tile types
 *
 * Implemention for class CTerrainTile, the abstract base class for all
 *   terrain tile types.  CTerrainTile encapsulates common attributes such as:
 *     - tile index assignment state
 *     - culling state
 */

#include "../Include/Globals.h"
#include "../Include/Terrain.h"
#include "../Include/Utility.h"
#include "../Include/Ui.h"


//
// CTerrainTile
//
CTerrainTile::CTerrainTile (void)
{
  state = TILE_UNASSIGNED;
  texid = 0;
  cull = false;
  translate.Set (0, 0, 0);
  bSphereCentre.Set (0, 0, 0);
  bSphereRadius = 0;
}

CTerrainTile::~CTerrainTile (void)
{
  if (texid != 0) {
    glDeleteTextures (1, &texid);
  }
}

void CTerrainTile::Assign (void)
{
  if (state == TILE_UNASSIGNED) {
    state = TILE_ASSIGNED;
  } else {
    WARN ("CTerrainTile::Assign while not in state TILE_UNASSIGNED");
  }
}

void CTerrainTile::Unassign (void)
{
  if (state == TILE_ASSIGNED) {
    state = TILE_UNASSIGNED;
  } else {
    WARN ("CTerrainTile::Unassign while not in state TILE_ASSIGNED");
  }
}

void CTerrainTile::Create (void)
{
  if (state == TILE_ASSIGNED) {
    state = TILE_CREATED;
  } else {
    WARN ("CTerrainTile::Create while not in state TILE_ASSIGNED");
  }
}

void CTerrainTile::Destroy (void)
{
  if (state == TILE_CREATED) {
    state = TILE_ASSIGNED;
  } else {
    WARN ("CTerrainTile::Destroy while not in state TILE_CREATED");
  }
}

bool CTerrainTile::IsAssigned (void)
{
  return (state == TILE_ASSIGNED) || (state == TILE_CREATED);
}

bool CTerrainTile::IsCreated (void)
{
  return (state == TILE_CREATED);
}

void CTerrainTile::Cull (CFrustum &f)
{
  // Cull tile if bounding sphere is entirely outside of frustum
  cull = (f.SphereInFrustum (bSphereCentre, bSphereRadius) == CFrustum::OUTSIDE_FRUSTUM);
}

void CTerrainTile::Print (FILE *f)
{
  fprintf (f, "  Translate:  x=%10.2f y=%10.2f z=%10.2f\n",
    translate.x, translate.y, translate.z);
  fprintf (f, "  bSphere  :  (%10.2f, %10.2f, %10.2f) R=%10.2f\n",
    bSphereCentre.x, bSphereCentre.y, bSphereCentre.z, bSphereRadius);
  fprintf (f, "  Culled   :  %c\n", cull ? 'Y' : 'N');
}
