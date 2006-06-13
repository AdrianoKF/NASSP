/***************************************************************************
  This file is part of Project Apollo - NASSP
  Copyright 2004-2005

  ORBITER vessel module: Saturn1Abort

  Project Apollo is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Project Apollo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Project Apollo; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  See http://nassp.sourceforge.net/license/ for more details.

  **************************** Revision History ****************************
  *	$Log$
  *	Revision 1.4  2006/01/05 19:40:53  movieman523
  *	Added Saturn1b abort stages to build.
  *	
  *	Revision 1.3  2006/01/04 23:06:03  movieman523
  *	Moved meshes into ProjectApollo directory and renamed a few.
  *	
  *	Revision 1.2  2005/11/21 23:08:15  movieman523
  *	Moved more mesh files into the ProjectApollo directory.
  *	
  *	Revision 1.1  2005/02/11 12:54:06  tschachim
  *	Initial version
  *	
  **************************************************************************/

#include "orbitersdk.h"
#include "stdio.h"

const VECTOR3 OFS_STAGE1 =  { 0, 0, -8.935};
const VECTOR3 OFS_STAGE2 =  { 0, 0, 9.25-12.25};
const VECTOR3 OFS_STAGE21 =  { 1.85,1.85,24.5-12.25};
const VECTOR3 OFS_STAGE22 =  { -1.85,1.85,24.5-12.25};
const VECTOR3 OFS_STAGE23 =  { 1.85,-1.85,24.5-12.25};
const VECTOR3 OFS_STAGE24 =  { -1.85,-1.85,24.5-12.25};


static int refcount = 0;
static MESHHANDLE hSat1stg1;
static MESHHANDLE hSat1stg2;
static MESHHANDLE hSat1stg21;
static MESHHANDLE hSat1stg22;
static MESHHANDLE hSat1stg23;
static MESHHANDLE hSat1stg24;
static MESHHANDLE hSat1tower;
static MESHHANDLE hSM;

void SetFirstStage (VESSEL *vessel)
{
	vessel->SetSize (25);
	vessel->SetCOG_elev (21.2);
	vessel->SetEmptyMass (185500.0);
	vessel->SetMaxFuelMass (407100);
	vessel->SetFuelMass(407100);
	//vessel->SetISP (ISP_FIRST_VAC);
	vessel->SetISP (2795);
	//slThrust = CalcSeaLevelThrust(THRUST_FIRST_VAC,ISP_FIRST_VAC,ISP_FIRST_SL, 407100);
	//vacThrust = THRUST_FIRST_VAC;
	vessel->SetMaxThrust (ENGINE_MAIN, 0);//slThrust
	vessel->SetMaxThrust (ENGINE_RETRO, 0);
	vessel->SetMaxThrust (ENGINE_HOVER, 0);
	vessel->SetMaxThrust (ENGINE_ATTITUDE, 3e4);
	vessel->SetPMI (_V(140,145,28));
	vessel->SetCrossSections (_V(395,380,115));
	vessel->SetCW (0.1, 0.3, 1.4, 1.4);
	vessel->SetRotDrag (_V(0.7,0.7,1.2));
	vessel->SetPitchMomentScale (0);
	vessel->SetBankMomentScale (0);
	vessel->SetLiftCoeffFunc (0);
	vessel->ClearMeshes();
	vessel->ClearExhaustRefs();
	vessel->ClearAttExhaustRefs();
	//vessel->ShiftCentreOfMass (_V(0,0,8.935));
	vessel->SetTouchdownPoints (_V(0,-1.0,-22.185), _V(-.7,.7,-22.185), _V(.7,.7,-22.185));
	VECTOR3 mesh_dir=_V(0,0,-8.935);
	vessel->AddMesh (hSat1stg1, &mesh_dir);
    mesh_dir=_V(0,0,9.25);
	vessel->AddMesh (hSat1stg2, &mesh_dir);
	mesh_dir=_V(1.85,1.85,24.5);
    vessel->AddMesh (hSat1stg21, &mesh_dir);
	mesh_dir=_V(-1.85,1.85,24.5);
    vessel->AddMesh (hSat1stg22, &mesh_dir);
	mesh_dir=_V(1.85,-1.85,24.5);
    vessel->AddMesh (hSat1stg23, &mesh_dir);
	mesh_dir=_V(-1.85,-1.85,24.5);
    vessel->AddMesh (hSat1stg24, &mesh_dir);
	mesh_dir=_V(0,0,30.15);
	vessel->AddMesh (hSM, &mesh_dir);
	mesh_dir=_V(0,0,34.40);
	VECTOR3 m_exhaust_pos1= {2,0,-23.5};
    VECTOR3 m_exhaust_pos2= {-2,0,-23.5};
	VECTOR3 m_exhaust_pos3= {0,2,-23.5};
	VECTOR3 m_exhaust_pos4= {0,-2,-23.5};
	VECTOR3 m_exhaust_ref = {0,0,-1};
	vessel->AddExhaustRef (EXHAUST_MAIN, m_exhaust_pos1, 30.0, 1.25, &m_exhaust_ref);
	vessel->AddExhaustRef (EXHAUST_MAIN, m_exhaust_pos2, 30.0, 1.25, &m_exhaust_ref);
	vessel->AddExhaustRef (EXHAUST_MAIN, m_exhaust_pos3, 30.0, 1.25, &m_exhaust_ref);
	vessel->AddExhaustRef (EXHAUST_MAIN, m_exhaust_pos4, 30.0, 1.25, &m_exhaust_ref);
	vessel->SetCameraOffset (_V(-1,1.0,34.15));
	vessel->SetEngineLevel(ENGINE_MAIN,0);
}



// ==============================================================
// API interface
// ==============================================================


DLLCLBK VESSEL *ovcInit (OBJHANDLE hvessel, int flightmodel)
{
if (!refcount++) {
		hSat1stg1 = oapiLoadMeshGlobal ("ProjectApollo/nsat1stg1");
		hSat1stg2 = oapiLoadMeshGlobal ("ProjectApollo/nsat1stg2");
		hSat1stg21 = oapiLoadMeshGlobal ("ProjectApollo/nsat1stg21");
		hSat1stg22 = oapiLoadMeshGlobal ("ProjectApollo/nsat1stg22");
		hSat1stg23 = oapiLoadMeshGlobal ("ProjectApollo/nsat1stg23");
		hSat1stg24 = oapiLoadMeshGlobal ("ProjectApollo/nsat1stg24");
		hSM = oapiLoadMeshGlobal ("ProjectApollo/SM");
	}
	return new VESSEL (hvessel, flightmodel);

}

DLLCLBK void ovcSaveState (VESSEL *vessel, FILEHANDLE scn)

{
	vessel->SaveDefaultState (scn);
}


DLLCLBK void ovcSetClassCaps (VESSEL *vessel, FILEHANDLE cfg)
{
	SetFirstStage (vessel);
}

