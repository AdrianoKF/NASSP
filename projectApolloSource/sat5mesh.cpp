/***************************************************************************
  This file is part of Project Apollo - NASSP
  Copyright 2004-2005 Jean-Luc Rocca-Serra, Mark Grant

  ORBITER vessel module: Saturn 5
  This file holds the mesh-related functions.

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
  *	Revision 1.92  2008/04/11 12:19:07  tschachim
  *	New SM and CM RCS.
  *	Improved abort handling.
  *	Fixed BasicExcel for VC6, reduced VS2005 warnings, bugfixes.
  *	
  *	Revision 1.91  2008/01/18 05:57:23  movieman523
  *	Moved SIVB creation code into generic Saturn function, and made ASTP sort of start to work.
  *	
  *	Revision 1.90  2008/01/17 01:46:27  movieman523
  *	Renamed LEMName to PayloadName and replaced LEMN with PAYN in the scenario file; reading LEMN is still supported for backward compatibility.
  *	
  *	Revision 1.89  2008/01/16 05:52:07  movieman523
  *	Removed all dockstate code.
  *	
  *	Revision 1.88  2008/01/14 04:31:10  movieman523
  *	Initial tidyup: ASTP should now work too.
  *	
  *	Revision 1.87  2008/01/14 01:17:06  movieman523
  *	Numerous changes to move payload creation from the CSM to SIVB.
  *	
  *	Revision 1.86  2008/01/12 04:14:10  movieman523
  *	Pass payload information to SIVB and have LEM use the fuel masses passed to it.
  *	
  *	Revision 1.85  2008/01/11 05:24:11  movieman523
  *	Added LEM fuel masses; currently they're passed to the LEM but it ignores them.
  *	
  *	Revision 1.84  2008/01/09 15:00:20  lassombra
  *	Added support for checklistController to save/load state.
  *	
  *	Added support for new scenario options LEMCHECK <lem checklist file and LEMCHECKAUTO <whether the lem should automatically execute checklists.
  *	
  *	Will document new options on the wiki
  *	
  *	Revision 1.83  2007/12/05 23:07:46  movieman523
  *	Revised to allow SLA panel rotaton to be specified up to 150 degrees. Also start of new connector-equipped vessel code which was mixed up with the rest!
  *	
  *	Revision 1.82  2007/10/09 21:49:38  tschachim
  *	Bugfixes
  *	
  *	Revision 1.81  2007/08/13 16:06:14  tschachim
  *	Moved bitmaps to subdirectory.
  *	New VAGC mission time pad load handling.
  *	New telescope and sextant panels.
  *	Fixed CSM/LV separation speed.
  *	
  *	Revision 1.80  2007/08/11 16:31:25  jasonims
  *	Changed name of BuildFirstStage to ChangeSatVBuildState  to clarify function...
  *	A little more Capcom...
  *	
  *	Revision 1.79  2007/06/23 21:20:37  dseagrav
  *	LVDC++ Update: Now with Pre-IGM guidance
  *	
  *	Revision 1.78  2007/06/06 15:02:16  tschachim
  *	OrbiterSound 3.5 support, various fixes and improvements.
  *	
  *	Revision 1.77  2007/03/01 18:24:33  tschachim
  *	Fixed Saturn V assembly
  *	
  *	Revision 1.76  2007/02/18 01:35:29  dseagrav
  *	MCC / LVDC++ CHECKPOINT COMMIT. No user-visible functionality added. lvimu.cpp/h and mcc.cpp/h added.
  *	
  *	Revision 1.75  2007/02/06 18:30:17  tschachim
  *	Bugfixes docking probe, CSM/LM separation. The ASTP stuff still needs fixing though.
  *	
  *	Revision 1.74  2006/11/13 14:47:30  tschachim
  *	New SPS engine.
  *	New ProjectApolloConfigurator.
  *	Fixed and changed camera and FOV handling.
  *	
  *	Revision 1.73  2006/10/23 13:41:48  tschachim
  *	Bugfix
  *	
  *	Revision 1.72  2006/09/23 22:34:40  jasonims
  *	New J-2 Engine textures...
  *	
  *	Revision 1.71  2006/08/13 16:55:35  movieman523
  *	Removed a bunch of unused files.
  *	
  *	Revision 1.70  2006/08/13 16:01:52  movieman523
  *	Renamed LEM. Think it all builds properly, I'm checking it in before the lightning knocks out the power here :).
  *	
  *	Revision 1.69  2006/08/11 20:37:46  movieman523
  *	Added HasProbe flag for docking probe.
  *	
  *	Revision 1.68  2006/08/02 04:32:05  jasonims
  *	corrected probe problem
  *	
  *	Revision 1.67  2006/07/07 19:44:58  movieman523
  *	First version of connector support.
  *	
  *	Revision 1.66  2006/06/26 19:05:36  movieman523
  *	More doxygen, made Lunar EVA a VESSEL2, made SM breakup, made LRV use VESSEL2 save/load functions.
  *	
  *	Revision 1.65  2006/06/12 20:47:36  movieman523
  *	Made switch lighting optional based on REALISM, and fixed SII SEP light.
  *	
  *	Revision 1.64  2006/06/10 23:27:41  movieman523
  *	Updated abort code.
  *	
  *	Revision 1.63  2006/06/10 14:36:44  movieman523
  *	Numerous changes. Lots of bug-fixes, new LES jettison code, lighting for guarded push switches and a partial rewrite of the Saturn 1b mesh code.
  *	
  *	Revision 1.62  2006/06/07 02:05:03  jasonims
  *	VC Stopping place....new VC cameras added (GNPanel, Right Dock) and VC cameras renamed to reflect position.  No working buttons yet, but theoretically they're possible.
  *	
  *	Revision 1.61  2006/05/27 11:50:04  movieman523
  *	Improved INT20 support, and made LET jettison work any time during launch on Saturn V.
  *	
  *	Revision 1.60  2006/05/27 00:54:28  movieman523
  *	Simplified Saturn V mesh code a lot, and added beginnings ot INT-20.
  *	
  *	Revision 1.59  2006/05/26 22:01:50  movieman523
  *	Revised stage handling some. Removed two of the three second-stage functions and split out the mesh and engine code.
  *	
  *	Revision 1.58  2006/05/26 18:42:54  movieman523
  *	Updated S1C DLL to support INT-20 stage.
  *	
  *	Revision 1.57  2006/05/21 15:42:54  tschachim
  *	Bugfix S-IC staging
  *	
  *	Revision 1.56  2006/05/19 13:46:56  tschachim
  *	Smoother S-IC staging.
  *	
  *	Revision 1.55  2006/05/06 06:00:35  jasonims
  *	No more venting our Astronauts into space...and no more LRV popping out of an Astronauts pocket....well sorta.
  *	
  *	Revision 1.54  2006/05/01 03:33:22  jasonims
  *	New CM and all the fixin's....
  *	
  *	Revision 1.53  2006/04/25 19:25:07  jasonims
  *	More Mesh Offset Updates.... Including removing extra code thanks to CM, Hatch, and Crew all aligned on same axis.  Only one Mesh offset required for all three
  *	
  *	Revision 1.52  2006/04/25 13:34:50  tschachim
  *	New first stage exhaust.
  *	
  *	Revision 1.51  2006/03/30 00:21:37  movieman523
  *	Pass empty mass correctly and remember to check in SM files :).
  *	
  *	Revision 1.50  2006/03/30 00:14:46  movieman523
  *	First pass at SM DLL.
  *	
  *	Revision 1.49  2006/03/29 19:06:49  movieman523
  *	First support for new SM.
  *	
  *	Revision 1.48  2006/02/03 13:19:55  tschachim
  *	Fixed SIC velocity during creation.
  *	
  *	Revision 1.47  2006/02/01 18:32:17  tschachim
  *	Adjusted particle streams.
  *	
  *	Revision 1.46  2006/01/26 19:26:31  movieman523
  *	Now we can set any scenario state from the config file for Saturn 1b or Saturn V. Also wired up a couple of LEM switches.
  *	
  *	Revision 1.45  2006/01/26 03:31:57  movieman523
  *	Less hacky low-res mesh support for Saturn V.
  *	
  *	Revision 1.44  2006/01/26 03:07:49  movieman523
  *	Quick hack to support low-res mesh.
  *	
  *	Revision 1.43  2006/01/24 13:48:28  tschachim
  *	Smoother staging with more eye-candy.
  *	
  *	Revision 1.42  2006/01/15 02:38:59  movieman523
  *	Moved CoG and removed phantom thrusters. Also delete launch site when we get a reasonable distance away.
  *	
  *	Revision 1.41  2006/01/12 19:57:24  tschachim
  *	Better prelaunch tank venting particles now?
  *	
  *	Revision 1.40  2006/01/12 14:52:31  tschachim
  *	Added prelaunch tank venting.
  *	
  *	Revision 1.39  2006/01/09 21:56:44  movieman523
  *	Added support for LEM and CSM AGC PAD loads in scenario file.
  *	
  *	Revision 1.38  2006/01/08 17:11:41  movieman523
  *	Added seperation particles to SII/SIVb sep.
  *	
  *	Revision 1.37  2006/01/08 14:51:22  movieman523
  *	Revised camera 3 position to be more photogenic, and added seperation particle effects.
  *	
  *	Revision 1.36  2006/01/08 04:00:24  movieman523
  *	Added first two engineering cameras.
  *	
  *	Revision 1.35  2006/01/06 22:55:53  movieman523
  *	Fixed SM seperation and cut off fuel cell power when it happens.
  *	
  *	Revision 1.34  2006/01/05 11:27:48  tschachim
  *	New dockingprobe handling.
  *	
  *	Revision 1.33  2006/01/04 02:55:49  movieman523
  *	Use LEM.cfg, not sat5_LMPKD.cfg.
  *	
  *	Revision 1.32  2005/12/28 16:19:10  movieman523
  *	Should now be getting all config files from ProjectApollo directory.
  *	
  *	Revision 1.31  2005/12/28 15:52:56  movieman523
  *	Get interstage config from subdirectory.
  *	
  *	Revision 1.30  2005/12/20 19:45:27  tschachim
  *	Added a dummy docking port to the SIVB just because
  *	now you can use the IMFD 4.2 autoburn feature.
  *	
  *	Revision 1.29  2005/11/28 00:36:48  movieman523
  *	Code tidyup - no intentional functional changes.
  *	
  *	Revision 1.28  2005/11/25 02:03:47  movieman523
  *	Fixed mixture-ratio change code and made it more realistic.
  *	
  *	Revision 1.27  2005/11/24 20:31:23  movieman523
  *	Added support for engine thrust decay during launch.
  *	
  *	Revision 1.26  2005/11/23 21:05:57  movieman523
  *	Use ProjectApollo directory for LEM config file.
  *	
  *	Revision 1.25  2005/11/23 01:43:13  movieman523
  *	Added SII stage DLL.
  *	
  *	Revision 1.24  2005/11/23 00:29:38  movieman523
  *	Added S1C DLL and revised LEVA code to look for NEP-specific flag if it exists.
  *	
  *	Revision 1.23  2005/11/21 13:26:41  tschachim
  *	New assembly meshes.
  *	
  *	Revision 1.22  2005/11/20 20:35:14  movieman523
  *	Moved mesh files into ProjectApollo directory, and fixed RCS on Saturn V SIVb after seperation.
  *	
  *	Revision 1.21  2005/11/20 01:06:27  movieman523
  *	Saturn V now uses SIVB DLL too.
  *	
  *	Revision 1.20  2005/11/16 20:21:39  movieman523
  *	CSM/LEM renaming changes.
  *	
  *	Revision 1.19  2005/11/16 00:18:49  movieman523
  *	Added beginnings of really basic IU emulation. Added random failures of caution and warning lights on non-historical missions. Added initial support for Skylab CM and SM. Added LEM Name option in scenario file.
  *	
  *	Revision 1.18  2005/11/09 18:14:51  tschachim
  *	New Saturn assembly process.
  *	
  *	Revision 1.17  2005/08/30 18:37:39  movieman523
  *	Added support for new interstage meshes.
  *	
  *	Revision 1.16  2005/08/24 00:30:00  movieman523
  *	Revised CM RCS code, and removed a load of switches that aren't used anymore.
  *	
  *	Revision 1.15  2005/08/21 22:21:00  movieman523
  *	Fixed SM RCS and activated SIVB RCS at all times for now.
  *	
  *	Revision 1.14  2005/08/10 21:54:04  movieman523
  *	Initial IMU implementation based on 'Virtual Apollo' code.
  *	
  *	Revision 1.13  2005/08/06 01:12:52  movieman523
  *	Added initial I/O channel support for CSM, and added Realism setting for LEM AGC.
  *	
  *	Revision 1.12  2005/08/05 13:05:55  tschachim
  *	Fixed BuildFirstStage
  *	
  *	Revision 1.11  2005/08/01 19:07:46  movieman523
  *	Genericised code to deal with SM destruction on re-entry, and did some tidying up of Saturn 1b code.
  *	
  *	Revision 1.10  2005/05/21 16:14:36  movieman523
  *	Pass Realism and AudioLanguage correctly from CSM to LEM.
  *	
  *	Revision 1.9  2005/05/18 23:34:23  movieman523
  *	Added roughly correct masses for the various Saturn payloads.
  *	
  *	Revision 1.8  2005/04/11 23:48:45  yogenfrutz
  *	correctes SIVB parked lem docking position
  *	
  *	Revision 1.7  2005/03/27 03:40:14  chode99
  *	Added support for LTA payloads (LM test articles) for Apollo 4, 6, 8.
  *	
  *	Revision 1.6  2005/03/26 01:48:39  chode99
  *	Added the LTA for Apollo 8, need scenario file with line "S4PL 2" to use it as a payload instead of the LM.
  *	
  *	Revision 1.5  2005/03/09 19:08:37  chode99
  *	SIC retrorockets moved back into main rocket motor fairings, made more visible by moving out and forward a bit.
  *	
  *	Revision 1.4  2005/03/09 00:22:53  chode99
  *	Added 4 SII retrorockets.
  *	Corrected placement of SIC retros.
  *	
  *	Revision 1.3  2005/02/19 19:45:07  chode99
  *	Moved 1 line of code (VECTOR3 mesh_dir=...) to allow compilation in .NET 2003
  *	
  *	Revision 1.2  2005/02/18 00:40:17  movieman523
  *	Play appropriate seperation sound at SM sep.
  *	
  *	Revision 1.1  2005/02/11 12:54:07  tschachim
  *	Initial version
  *	
  **************************************************************************/

// To force orbitersdk.h to use <fstream> in any compiler version
#pragma include_alias( <fstream.h>, <fstream> )
#include "Orbitersdk.h"
#include <stdio.h>
#include <math.h>
#include "OrbiterSoundSDK35.h"
#include "soundlib.h"

#include "nasspdefs.h"
#include "nasspsound.h"

#include "toggleswitch.h"
#include "apolloguidance.h"
#include "dsky.h"
#include "csmcomputer.h"
#include "lemcomputer.h"
#include "IMU.h"
#include "lvimu.h"

#include "saturn.h"
#include "saturnv.h"
#include "tracer.h"
#include "LEM.h"

#include "sivb.h"
#include "sii.h"
#include "s1c.h"
#include "sm.h"

static PARTICLESTREAMSPEC srb_contrail = {
	0, 
	12.0,	// size
	20,		// rate
	150.0,	// velocity
	0.3,	// velocity distribution
	3.0,	// lifetime
	4,		// growthrate
	3.5,	// atmslowdown 
	PARTICLESTREAMSPEC::DIFFUSE,
	PARTICLESTREAMSPEC::LVL_PLIN, 0, 1.0,
	PARTICLESTREAMSPEC::ATM_FLAT, 0.8, 0.8
};

static PARTICLESTREAMSPEC srb_exhaust = {
	0,		// flag
	2.85,	// size
	1500,	// rate
	60.0,	// velocity
	0.1,	// velocity distribution
	1.5,	// lifetime
	2.0,	// growthrate
	0.0,	// atmslowdown 
	PARTICLESTREAMSPEC::EMISSIVE,
	PARTICLESTREAMSPEC::LVL_PSQRT, 0, 1.0,
	PARTICLESTREAMSPEC::ATM_FLAT, 1.0, 1.0
};

static PARTICLESTREAMSPEC solid_exhaust = {
	0, 0.5, 250, 35.0, 0.1, 0.15, 0.5, 1.0, 
	PARTICLESTREAMSPEC::EMISSIVE,
	PARTICLESTREAMSPEC::LVL_PSQRT, 0, 0.5,
	PARTICLESTREAMSPEC::ATM_FLAT, 1.0, 1.0
};

//
// Spew out particles to simulate the junk thrown out by stage
// seperation explosives.
//

static PARTICLESTREAMSPEC seperation_junk = {
	0,		// flag
	0.04,	// size
	500,	// rate
	4.0,    // velocity
	5.0,    // velocity distribution
	30,		// lifetime
	0,	    // growthrate
	0,      // atmslowdown 
	PARTICLESTREAMSPEC::EMISSIVE,
	PARTICLESTREAMSPEC::LVL_FLAT, 1.0, 1.0,
	PARTICLESTREAMSPEC::ATM_FLAT, 1.0, 1.0
};

// "prelaunch tank venting" particle streams
static PARTICLESTREAMSPEC prelaunchvent1_spec = {
	0,		// flag
	0.6,	// size
	60,		// rate
	2,	    // velocity
	0.5,    // velocity distribution
	2,		// lifetime
	0.2,	// growthrate
	0.5,    // atmslowdown 
	PARTICLESTREAMSPEC::EMISSIVE,
	PARTICLESTREAMSPEC::LVL_FLAT, 0.25, 0.25,
	PARTICLESTREAMSPEC::ATM_FLAT, 0.25, 0.25
};

static PARTICLESTREAMSPEC prelaunchvent2_spec = {
	0,		// flag
	0.5,	// size
	80,		// rate
	3,		// velocity
	0.6,	// velocity distribution
	1,		// lifetime
	0.1,	// growthrate
	0.7,    // atmslowdown
	PARTICLESTREAMSPEC::EMISSIVE,
	PARTICLESTREAMSPEC::LVL_FLAT, 0.2, 0.2,
	PARTICLESTREAMSPEC::ATM_FLAT, 0.2, 0.2
};

static PARTICLESTREAMSPEC prelaunchvent3_spec = {
	0,		// flag
	0.4,	// size
	100,	// rate
	4,	    // velocity
	0.6,    // velocity distribution
	1,		// lifetime
	0.2,	// growthrate
	0.9,    // atmslowdown
	PARTICLESTREAMSPEC::EMISSIVE,
	PARTICLESTREAMSPEC::LVL_FLAT, 0.1, 0.1,
	PARTICLESTREAMSPEC::ATM_FLAT, 0.1, 0.1
};

// "staging vent" particle streams
static PARTICLESTREAMSPEC stagingvent_spec = {
	0,		// flag
	2.5,	// size
	100,	// rate
	10,	    // velocity
	2,		// velocity distribution
	2,		// lifetime
	2.0,	// growthrate
	0.5,    // atmslowdown 
	PARTICLESTREAMSPEC::EMISSIVE,
	PARTICLESTREAMSPEC::LVL_FLAT, 0.1, 0.1,
	PARTICLESTREAMSPEC::ATM_FLAT, 0.1, 0.1
};

static MESHHANDLE hsat5stg1;
static MESHHANDLE hsat5intstg;
static MESHHANDLE hsat5intstg4;
static MESHHANDLE hsat5intstg8;
static MESHHANDLE hsat5stg2;
static MESHHANDLE hsat5stg2base;
static MESHHANDLE hsat5stg2interstage;
static MESHHANDLE hsat5stg3;
static MESHHANDLE hsat5stg3base;
static MESHHANDLE hsat5stg31;
static MESHHANDLE hsat5stg32;
static MESHHANDLE hsat5stg33;
static MESHHANDLE hsat5stg34;

static MESHHANDLE hsat5stg1low;
static MESHHANDLE hsat5intstglow;
static MESHHANDLE hsat5stg2low;
static MESHHANDLE hsat5stg3low;
static MESHHANDLE hsat5stg31low;
static MESHHANDLE hsat5stg32low;
static MESHHANDLE hsat5stg33low;
static MESHHANDLE hsat5stg34low;

static MESHHANDLE hCONE;

static MESHHANDLE hApollocsm;

static MESHHANDLE hLMPKD;
static MESHHANDLE hapollo8lta;
static MESHHANDLE hlta_2r;

static SURFHANDLE contrail_tex;
static SURFHANDLE exhaust_tex;

#define LOAD_MESH(var, name) var = oapiLoadMeshGlobal(name);

void LoadSat5Meshes()

{
	LOAD_MESH(hsat5stg1low, "ProjectApollo/LowRes/sat5stg1");
	LOAD_MESH(hsat5stg1, "ProjectApollo/sat5stg1");

	LOAD_MESH(hsat5intstglow, "ProjectApollo/LowRes/sat5intstg");
	LOAD_MESH(hsat5intstg, "ProjectApollo/sat5intstg");
	LOAD_MESH(hsat5intstg4, "ProjectApollo/sat5intstg4");
	LOAD_MESH(hsat5intstg8, "ProjectApollo/sat5intstg8");

	LOAD_MESH(hsat5stg2low, "ProjectApollo/LowRes/sat5stg2");
	LOAD_MESH(hsat5stg2, "ProjectApollo/sat5stg2");
	LOAD_MESH(hsat5stg2base, "ProjectApollo/sat5stg2base");
	LOAD_MESH(hsat5stg2interstage, "ProjectApollo/sat5stg2intstg");

	LOAD_MESH(hsat5stg3low, "ProjectApollo/LowRes/sat5stg3");
	LOAD_MESH(hsat5stg31low, "ProjectApollo/LowRes/sat5stg31");
	LOAD_MESH(hsat5stg32low, "ProjectApollo/LowRes/sat5stg32");
	LOAD_MESH(hsat5stg33low, "ProjectApollo/LowRes/sat5stg33");
	LOAD_MESH(hsat5stg34low, "ProjectApollo/LowRes/sat5stg34");

	LOAD_MESH(hsat5stg3, "ProjectApollo/sat5stg3");
	LOAD_MESH(hsat5stg3base, "ProjectApollo/sat5stg3base");
	LOAD_MESH(hsat5stg31, "ProjectApollo/sat5stg31");
	LOAD_MESH(hsat5stg32, "ProjectApollo/sat5stg32");
	LOAD_MESH(hsat5stg33, "ProjectApollo/sat5stg33");
	LOAD_MESH(hsat5stg34, "ProjectApollo/sat5stg34");

	LOAD_MESH(hLMPKD, "ProjectApollo/LM_Parked");
	LOAD_MESH(hapollo8lta, "ProjectApollo/apollo8_lta");
	LOAD_MESH(hlta_2r, "ProjectApollo/LTA_2R");

	contrail_tex = oapiRegisterParticleTexture("Contrail2");
	exhaust_tex = oapiRegisterExhaustTexture("ProjectApollo/Exhaust2");

	srb_exhaust.tex = contrail_tex;
	solid_exhaust.tex = contrail_tex;
	seperation_junk.tex = oapiRegisterParticleTexture("ProjectApollo/junk");;
}

void SaturnV::SetupMeshes()

{
	if (LowRes)
	{
		hStage1Mesh = hsat5stg1low;
		hStage2Mesh = hsat5stg2low;
		hStage3Mesh = hsat5stg3low;
		hStageSLA1Mesh = hsat5stg31low;
		hStageSLA2Mesh = hsat5stg32low;
		hStageSLA3Mesh = hsat5stg33low;
		hStageSLA4Mesh = hsat5stg34low;
	}
	else
	{
		hStage1Mesh = hsat5stg1;
		hStage2Mesh = hsat5stg2;
		hStage3Mesh = hsat5stg3;
		hStageSLA1Mesh = hsat5stg31;
		hStageSLA2Mesh = hsat5stg32;
		hStageSLA3Mesh = hsat5stg33;
		hStageSLA4Mesh = hsat5stg34;
	}
}

MESHHANDLE SaturnV::GetInterstageMesh()

{
	if (LowRes)
		return hsat5intstglow;

	switch (SII_UllageNum)
	{
	case 4:
		return hsat5intstg4;

	case 8:
		return hsat5intstg8;

	default:
		return hsat5intstg;
	}
}

void SaturnV::ChangeSatVBuildState (int bstate)

{
	TRACESETUP("ChangeSatVBuildState");

	SetSize (59.5);
	SetEmptyMass (Stage1Mass);

	SetPMI (_V(1147,1147,216.68));
	SetCrossSections (_V(1129,1133,52.4));
	SetCW (0.1, 0.3, 1.4, 1.4);
	SetRotDrag (_V(0.7,0.7,1.2));
	SetPitchMomentScale (0);
	SetBankMomentScale (0);
	SetLiftCoeffFunc (0);
	SetSurfaceFrictionCoeff(10e80,10e80);
	ClearMeshes();
	VECTOR3 m_exhaust_pos1= {3,3,Offset1st};
	VECTOR3 mesh_dir=_V(0,0,-80.0+STG0O);

	if (bstate >=1)
	{
		mesh_dir=_V(0,0,-54.0+STG0O);
		AddMesh (hStage1Mesh, &mesh_dir);
	}

	if (bstate >=2)
	{
		mesh_dir=_V(0,0,-30.5+STG0O);
		AddMesh (GetInterstageMesh(), &mesh_dir);
	}

	if (bstate == 2)
	{
		mesh_dir=_V(0,0,-17.2+STG0O);
		AddMesh (hsat5stg2base, &mesh_dir);
	}

	if (bstate > 2)
	{
		mesh_dir=_V(0,0,-17.2+STG0O);
		AddMesh (hStage2Mesh, &mesh_dir);
	}

	if (bstate ==3 )
	{
		mesh_dir=_V(0,0,2.+STG0O);
		AddMesh (hsat5stg3base, &mesh_dir);
	}

	if (bstate > 3)
	{
		mesh_dir=_V(0,0,2.+STG0O);
		AddMesh (hStage3Mesh, &mesh_dir);
	}

	if (bstate >=4)
	{
		if (LEM_DISPLAY && (SIVBPayload == PAYLOAD_LEM))
		{
			mesh_dir=_V(0,0,12+STG0O);
			AddMesh (hLMPKD, &mesh_dir);
		}
	}

	if (bstate >=4)
	{
		mesh_dir=_V(-1.48,-1.48,14.55+STG0O);
		AddMesh (hStageSLA1Mesh, &mesh_dir);
	}

	if (bstate >=4)
	{
		mesh_dir=_V(1.48,-1.48,14.55+STG0O);
		AddMesh (hStageSLA2Mesh, &mesh_dir);
	}

	if (bstate >=4)
	{
		mesh_dir=_V(1.48,1.48,14.55+STG0O);
		AddMesh (hStageSLA3Mesh, &mesh_dir);
	}

	if (bstate >=4){
		mesh_dir=_V(-1.48,1.48,14.55+STG0O);
		AddMesh (hStageSLA4Mesh, &mesh_dir);
	}

	if (bstate >=4)
	{
		AddSM(19.1+STG0O, false);
	}

	if (bstate >=4)
	{
		mesh_dir=_V(0,0,23.25+STG0O);
		AddMesh (hCM, &mesh_dir);
	}

	if (bstate >=4)
	{
		mesh_dir=_V(0,0,23.25+STG0O);
		AddMesh (hCMInt, &mesh_dir);
	}

	if (bstate >=4)
	{
		mesh_dir=_V(0,0,23.25+STG0O);
		AddMesh (hFHC, &mesh_dir);
	}

	if (bstate >=4)
	{
		mesh_dir=_V(0,0,23.25+STG0O);
		AddMesh (hprobe, &mesh_dir);
	}

	if (bstate >=5)
	{
		mesh_dir=_V(0,0,28.2+STG0O);
		AddMesh (hsat5tower, &mesh_dir);
	}

	Offset1st = -60.1+STG0O;
	SetCameraOffset (_V(-1,1.0,23.1+STG0O));

	double TCP=-101.5+STG0O-TCPO;
	SetTouchdownPoints (_V(0,-100.0,TCP), _V(-7,7,TCP), _V(7,7,TCP));
}

void SaturnV::SetFirstStage ()

{
	TRACESETUP("SetFirstStage");

	double EmptyMass = Stage1Mass - (InterstageAttached ? 0.0 : Interstage_Mass) - (LESAttached ? 0.0 : Abort_Mass);

	//
	// *********************** physical parameters *********************************
	//

    SetSize (59.5);
	SetEmptyMass (EmptyMass);
	SetPMI (_V(1147,1147,116.60));
	SetCrossSections (_V(1129,1133,52.4));
	SetCW (0.1, 0.3, 1.4, 1.4);
	SetRotDrag (_V(0.7,0.7,1.2));
	SetPitchMomentScale (0);
	SetBankMomentScale (0);
	SetLiftCoeffFunc (0);
	SetSurfaceFrictionCoeff(10e90,10e90);

	//
	// ************************ visual parameters **********************************
	//

	ClearMeshes();
	UINT meshidx;
	double TCP=-101.5+STG0O-TCPO;
	SetTouchdownPoints (_V(0,-100.0,TCP), _V(-7,7,TCP), _V(7,7,TCP));

	VECTOR3 mesh_dir=_V(0,0,-54.0+STG0O);
	meshidx = AddMesh (hStage1Mesh, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_ALWAYS);

	if (SaturnType == SAT_INT20)
	{
		mesh_dir = _V(0, 0, 12.35-54.0+STG0O);
		meshidx = AddMesh (hsat5stg2interstage, &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_ALWAYS);
		SetThirdStageMesh(STG0O - 24.5);
	}
	else
	{
		SetSecondStageMesh(STG0O);
	}

	buildstatus = 6;

	EnableTransponder (true);

	//
	// **************************** NAV radios *************************************
	//

	InitNavRadios (4);
}

void SaturnV::SetFirstStageEngines ()

{
	int i;

	ClearThrusterDefinitions();

	//
	// ************************* propellant specs **********************************
	//

	if (!ph_1st)
		ph_1st  = CreatePropellantResource(SI_FuelMass); //1st stage Propellant

	SetDefaultPropellantResource (ph_1st); // display 1st stage propellant level in generic HUD

	//
	// *********************** thruster definitions ********************************
	//

	Offset1st = -75.1+STG0O;
	VECTOR3 m_exhaust_ref = {0,0,-1};
    VECTOR3 MAIN4a_Vector= {3,3,Offset1st+0.5};
	VECTOR3 MAIN2a_Vector={-3,-3,Offset1st+0.5};
	VECTOR3 MAIN1a_Vector= {-3,3,Offset1st+0.5};
	VECTOR3 MAIN3a_Vector={3,-3,Offset1st+0.5};
	VECTOR3 MAIN5a_Vector={0,0,Offset1st+0.5};

	PROPELLANT_HANDLE	pph;

	if (MissionTime < (-20.0))
		pph = 0;
	else
		pph = ph_1st;

	//
	// orbiter main thrusters
	//

	th_main[0] = CreateThruster (MAIN4a_Vector, _V( 0,0,1), THRUST_FIRST_VAC , pph, ISP_FIRST_VAC, ISP_FIRST_SL);
	th_main[1] = CreateThruster (MAIN2a_Vector, _V( 0,0,1), THRUST_FIRST_VAC , pph, ISP_FIRST_VAC, ISP_FIRST_SL);
	th_main[2] = CreateThruster (MAIN1a_Vector, _V( 0,0,1), THRUST_FIRST_VAC , pph, ISP_FIRST_VAC, ISP_FIRST_SL);
	th_main[3] = CreateThruster (MAIN3a_Vector, _V( 0,0,1), THRUST_FIRST_VAC , pph, ISP_FIRST_VAC, ISP_FIRST_SL);
	th_main[4] = CreateThruster (MAIN5a_Vector, _V( 0,0,1), THRUST_FIRST_VAC , pph, ISP_FIRST_VAC, ISP_FIRST_SL);

	thg_main = CreateThrusterGroup (th_main, SI_EngineNum, THGROUP_MAIN);
	for (i = 0; i < SI_EngineNum; i++) AddExhaust (th_main[i], 120.0, 3.5, exhaust_tex);

	AddExhaustStream (th_main[0], MAIN4a_Vector+_V(0,0,-18), &srb_exhaust);
	AddExhaustStream (th_main[1], MAIN2a_Vector+_V(0,0,-18), &srb_exhaust);
	AddExhaustStream (th_main[2], MAIN1a_Vector+_V(0,0,-18), &srb_exhaust);
	AddExhaustStream (th_main[3], MAIN3a_Vector+_V(0,0,-18), &srb_exhaust);
	AddExhaustStream (th_main[4], MAIN5a_Vector+_V(0,0,-18), &srb_exhaust);

	// Contrail
	for (i = 0; i < SI_EngineNum; i++) {
		if (contrail[i]) {
			DelExhaustStream(contrail[i]);
			contrail[i] = NULL;
		}
	}
	contrail[0] = AddParticleStream(&srb_contrail, MAIN4a_Vector+_V(0,0,-25), _V( 0,0,-1), &contrailLevel);
	contrail[1] = AddParticleStream(&srb_contrail, MAIN2a_Vector+_V(0,0,-25), _V( 0,0,-1), &contrailLevel);
	contrail[2] = AddParticleStream(&srb_contrail, MAIN1a_Vector+_V(0,0,-25), _V( 0,0,-1), &contrailLevel);
	contrail[3] = AddParticleStream(&srb_contrail, MAIN3a_Vector+_V(0,0,-25), _V( 0,0,-1), &contrailLevel);
	contrail[4] = AddParticleStream(&srb_contrail, MAIN5a_Vector+_V(0,0,-25), _V( 0,0,-1), &contrailLevel);

	ThrustAdjust = 1.0;
}

void SaturnV::SetSecondStage ()
{
	TRACESETUP("SetSecondStage");

	ClearMeshes();

	double EmptyMass = Stage2Mass - (InterstageAttached ? 0.0 : Interstage_Mass) - (LESAttached ? 0.0 : Abort_Mass);

	SetSize (35.5);
	SetCOG_elev (15.225);
	SetEmptyMass (EmptyMass);
	SetPMI (_V(374,374,60));
	SetCrossSections (_V(524,524,97));
	SetCW (0.1, 0.3, 1.4, 1.4);
	SetRotDrag (_V(0.7,0.7,1.2));
	SetPitchMomentScale (0);
	SetBankMomentScale (0);
	SetLiftCoeffFunc (0);

	SetSecondStageMesh(-STG1O);

	SIISepState = InterstageAttached;
}

void SaturnV::SetSecondStageMesh(double offset)

{
	VECTOR3 mesh_dir;
	UINT meshidx;

	if (InterstageAttached)
	{
		mesh_dir=_V(0,0,-30.5 + offset);
		meshidx = AddMesh (GetInterstageMesh(), &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_ALWAYS);
	}

	mesh_dir=_V(0,0,-17.2 + offset);
	meshidx = AddMesh (hStage2Mesh, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_ALWAYS);

	SetThirdStageMesh(offset);
}

void SaturnV::SetSecondStageEngines(double offset)

{
    ClearThrusterDefinitions();

	if(ph_1st) {
		DelPropellantResource(ph_1st);
		ph_1st = 0;
	}

	//
	// ************************* propellant specs **********************************
	//

	if (!ph_2nd)
		ph_2nd  = CreatePropellantResource(SII_FuelMass); //2nd stage Propellant
	SetDefaultPropellantResource (ph_2nd); // display 2nd stage propellant level in generic HUD

	//
	// *********************** thruster definitions ********************************
	//

	int i;

	VECTOR3 m_exhaust_pos1= {-1.8,-1.8,-33 + offset};
	VECTOR3 m_exhaust_pos2= {1.8,1.8,-33 + offset};
	VECTOR3 m_exhaust_pos3= {-1.8,1.8,-33 + offset};
	VECTOR3 m_exhaust_pos4 = {1.8,-1.8,-33 + offset};
	VECTOR3 m_exhaust_pos5 = {0,0,-33 + offset};
	VECTOR3 s_exhaust_pos = {0, 0, -35.0 + offset};

	VECTOR3	m_exhaust_pos6= _V(0,5.07,-33.15 + offset);
	VECTOR3 m_exhaust_pos7= _V(0,-5.07,-33.15 + offset);
	VECTOR3	m_exhaust_pos8= _V(5.07,0,-33.15 + offset);
	VECTOR3 m_exhaust_pos9= _V(-5.07,0,-33.15 + offset);
	VECTOR3	m_exhaust_pos10= _V(3.55,3.7,-33.15 + offset);
	VECTOR3 m_exhaust_pos11= _V(3.55,-3.7,-33.15 + offset);
	VECTOR3	m_exhaust_pos12= _V(-3.55,3.7,-33.15 + offset);
	VECTOR3 m_exhaust_pos13= _V(-3.55,-3.7,-33.15 + offset);

	//
	// Seperation 'thruster'.
	//

	ph_sep = CreatePropellantResource(0.25);

	th_sep[0] = CreateThruster (m_exhaust_pos10, _V( 1,1,0), 1.0, ph_sep, 10.0, 10.0);
	th_sep[1] = CreateThruster (m_exhaust_pos11, _V( 1,-1,0), 1.0, ph_sep, 10.0, 10.0);
	th_sep[2] = CreateThruster (m_exhaust_pos12, _V( -1,1,0), 1.0, ph_sep, 10.0, 10.0);
	th_sep[3] = CreateThruster (m_exhaust_pos13, _V( -1,-1,0), 1.0, ph_sep, 10.0, 10.0);
	th_sep[4] = CreateThruster (m_exhaust_pos6, _V( 0,1,0), 1.0, ph_sep, 10.0, 10.0);
	th_sep[5] = CreateThruster (m_exhaust_pos7, _V( 0,-1,0), 1.0, ph_sep, 10.0, 10.0);
	th_sep[6] = CreateThruster (m_exhaust_pos8, _V( 1,0,0), 1.0, ph_sep, 10.0, 10.0);
	th_sep[7] = CreateThruster (m_exhaust_pos9, _V( -1,0,0), 1.0, ph_sep, 10.0, 10.0);

	for (i = 0; i < 8; i++) {
		AddExhaustStream (th_sep[i], &seperation_junk);
		SetThrusterLevel(th_sep[i], 1.0);
	}

	//
	// Interstage seperation 'thruster'.
	//

	ph_sep2 = CreatePropellantResource(0.25);

	th_sep2[0] = CreateThruster (m_exhaust_pos10, _V( 1,1,0), 1.0, ph_sep2, 10.0, 10.0);
	th_sep2[1] = CreateThruster (m_exhaust_pos11, _V( 1,-1,0), 1.0, ph_sep2, 10.0, 10.0);
	th_sep2[2] = CreateThruster (m_exhaust_pos12, _V( -1,1,0), 1.0, ph_sep2, 10.0, 10.0);
	th_sep2[3] = CreateThruster (m_exhaust_pos13, _V( -1,-1,0), 1.0, ph_sep2, 10.0, 10.0);
	th_sep2[4] = CreateThruster (m_exhaust_pos6, _V( 0,1,0), 1.0, ph_sep2, 10.0, 10.0);
	th_sep2[5] = CreateThruster (m_exhaust_pos7, _V( 0,-1,0), 1.0, ph_sep2, 10.0, 10.0);
	th_sep2[6] = CreateThruster (m_exhaust_pos8, _V( 1,0,0), 1.0, ph_sep2, 10.0, 10.0);
	th_sep2[7] = CreateThruster (m_exhaust_pos9, _V( -1,0,0), 1.0, ph_sep2, 10.0, 10.0);

	for (i = 0; i < 8; i++) {
		AddExhaustStream (th_sep2[i], &seperation_junk);
	}

	//
	// orbiter main thrusters
	//
	th_main[0] = CreateThruster (m_exhaust_pos1, _V( 0,0,1), THRUST_SECOND_VAC , ph_2nd, ISP_SECOND_VAC, ISP_SECOND_SL);
	th_main[1] = CreateThruster (m_exhaust_pos2,_V( 0,0,1),  THRUST_SECOND_VAC , ph_2nd, ISP_SECOND_VAC, ISP_SECOND_SL);
	th_main[2] = CreateThruster (m_exhaust_pos3, _V( 0,0,1), THRUST_SECOND_VAC , ph_2nd, ISP_SECOND_VAC, ISP_SECOND_SL);
	th_main[3] = CreateThruster (m_exhaust_pos4, _V( 0,0,1), THRUST_SECOND_VAC , ph_2nd, ISP_SECOND_VAC, ISP_SECOND_SL);
	th_main[4] = CreateThruster (m_exhaust_pos5, _V( 0,0,1), THRUST_SECOND_VAC , ph_2nd, ISP_SECOND_VAC, ISP_SECOND_SL);

	thg_main = CreateThrusterGroup (th_main, SII_EngineNum, THGROUP_MAIN);

	for (i = 0; i < SII_EngineNum; i++)
		AddExhaust (th_main[i], 30.0, 2.9 , J2Tex);

	if (SII_UllageNum) {
		th_ull[0] = CreateThruster (m_exhaust_pos10, _V( 0,0,1),100000 , ph_2nd, 3000);
		th_ull[1] = CreateThruster (m_exhaust_pos11, _V( 0,0,1),100000, ph_2nd, 3000);
		th_ull[2] = CreateThruster (m_exhaust_pos12, _V( 0,0,1),100000, ph_2nd, 3000);
		th_ull[3] = CreateThruster (m_exhaust_pos13, _V( 0,0,1),100000, ph_2nd, 3000);
		th_ull[4] = CreateThruster (m_exhaust_pos6, _V( 0,0,1), 100000 , ph_2nd, 3000);
		th_ull[5] = CreateThruster (m_exhaust_pos7, _V( 0,0,1), 100000 , ph_2nd, 3000);
		th_ull[6] = CreateThruster (m_exhaust_pos8, _V( 0,0,1), 100000 , ph_2nd, 3000);
		th_ull[7] = CreateThruster (m_exhaust_pos9, _V( 0,0,1), 100000 , ph_2nd, 3000);

		for (i = 0; i < SII_UllageNum; i ++) {
			AddExhaust (th_ull[i], 5.0, 0.3, exhaust_tex);
			AddExhaustStream (th_ull[i], &solid_exhaust);
		}

		thg_ull = CreateThrusterGroup (th_ull, SII_UllageNum, THGROUP_USER);
	}

	SetSIICMixtureRatio(MixtureRatio);
}


void SaturnV::SetThirdStage ()

{
	ClearMeshes();
	TRACESETUP("SetThirdStage");

	SetSize (15.5);
	SetCOG_elev (15.225);
	SetEmptyMass (Stage3Mass);
	SetPMI (_V(53.5,53.5,5));
	SetCrossSections (_V(167,167,47));
	SetCW (0.1, 0.3, 1.4, 1.4);
	SetRotDrag (_V(0.7,0.7,1.2));
	SetPitchMomentScale (0);
	SetBankMomentScale (0);
	SetLiftCoeffFunc (0);

	//
	// Clear SII Sep light just in case the interstage hung up.
	//

	ClearSIISep();

	SetThirdStageMesh(-STG2O);
}

void SaturnV::SetThirdStageMesh (double offset)
{
	S4Offset = 2.0 + offset;

	VECTOR3 mesh_dir=_V(0, 0, S4Offset);

	AddMesh (hStage3Mesh, &mesh_dir);
	if (LEM_DISPLAY && (SIVBPayload == PAYLOAD_LEM)){
		mesh_dir=_V(0,0,12 + offset);
		AddMesh (hLMPKD, &mesh_dir);
	}

	mesh_dir=_V(-1.48,-1.48,14.55 + offset);
	AddMesh (hStageSLA1Mesh, &mesh_dir);
	mesh_dir=_V(1.48,-1.48,14.55 + offset);
	AddMesh (hStageSLA2Mesh, &mesh_dir);
	mesh_dir=_V(1.48,1.48,14.55 + offset);
    AddMesh (hStageSLA3Mesh, &mesh_dir);
	mesh_dir=_V(-1.48,1.48,14.55 + offset);
    AddMesh (hStageSLA4Mesh, &mesh_dir);

	AddSM(19.1 + offset, false);

	mesh_dir=_V(0, 0, 23.25 + offset);

	UINT meshidx;
	meshidx = AddMesh (hCM, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

	// And the Crew
	if (Crewed) {
		cmpidx = AddMesh (hCMP, &mesh_dir);
		crewidx = AddMesh (hCREW, &mesh_dir);
		SetCrewMesh();
	} else {
		cmpidx = -1;
		crewidx = -1;
	}

	meshidx = AddMesh (hCMInt, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_EXTERNAL);

	meshidx = AddMesh (hCMVC, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_VC);
	VCMeshOffset = mesh_dir;

	meshidx = AddMesh (hFHC, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

	probeidx = -1;
	probeextidx = -1;

	if (LESAttached)
	{
		TowerOffset = 28.2 + offset;
		mesh_dir=_V(0, 0, TowerOffset);
		meshidx = AddMesh (hsat5tower, &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);
	}
	else {
		if (HasProbe) {
			probeidx = AddMesh(hprobe, &mesh_dir);
			probeextidx = AddMesh(hprobeext, &mesh_dir);
			SetDockingProbeMesh();
		}
	}

	VECTOR3 dockpos = {0,0,24.8 + offset};
	VECTOR3 dockdir = {0,0,1};
	VECTOR3 dockrot = {0,1,0};
	SetDockParams(dockpos, dockdir, dockrot);

	SetCameraOffset (_V(-1,1.0,32.4 + offset));
	SetView(23.1 + offset, false);
}

void SaturnV::SetThirdStageEngines (double offset)
{
	DelThrusterGroup(THGROUP_MAIN, true);
    ClearThrusterDefinitions();

	// ************************* propellant specs **********************************

	if (!ph_3rd) 
		ph_3rd  = CreatePropellantResource(S4B_FuelMass); //3rd stage Propellant

	SetDefaultPropellantResource (ph_3rd); // display 3rd stage propellant level in generic HUD

	if (ph_2nd) {
		DelPropellantResource(ph_2nd);
		ph_2nd = 0;
	}

	if (ph_sep) 
	{
		DelPropellantResource(ph_sep);
		ph_sep = 0;
	}

	if (ph_sep2) 
	{
		DelPropellantResource(ph_sep2);
		ph_sep2 = 0;
	}

	//
	// *********************** thruster definitions ********************************
	//

	VECTOR3	s_exhaust_pos1= _V(2.55,2.55,-3.6 + offset);
	VECTOR3 s_exhaust_pos2= _V(2.55,-2.55,-3.6 + offset);
	VECTOR3	s_exhaust_pos3= _V(-2.55,2.55,-3.6 + offset);
	VECTOR3 s_exhaust_pos4= _V(-2.55,-2.55,-3.6 + offset);
	VECTOR3	s_exhaust_pos6= _V(0,3.6,-3.6 + offset);
	VECTOR3 s_exhaust_pos7= _V(0,-3.6,-3.6 + offset);
	VECTOR3	s_exhaust_pos8= _V(3.6,0,-3.6 + offset);
	VECTOR3 s_exhaust_pos9= _V(-3.6,0,-3.6 + offset);

	//
	// Seperation 'thrusters'.
	//

	if (viewpos != SATVIEW_ENG1 && viewpos != SATVIEW_ENG2) 
	{
		int i;

		ph_sep = CreatePropellantResource(0.25);

		th_sep[0] = CreateThruster (s_exhaust_pos1, _V( 1,1,0), 1.0, ph_sep, 10.0, 10.0);
		th_sep[1] = CreateThruster (s_exhaust_pos2, _V( 1,-1,0), 1.0, ph_sep, 10.0, 10.0);
		th_sep[2] = CreateThruster (s_exhaust_pos3, _V( -1,1,0), 1.0, ph_sep, 10.0, 10.0);
		th_sep[3] = CreateThruster (s_exhaust_pos4, _V( -1,-1,0), 1.0, ph_sep, 10.0, 10.0);
		th_sep[4] = CreateThruster (s_exhaust_pos6, _V( 0,1,0), 1.0, ph_sep, 10.0, 10.0);
		th_sep[5] = CreateThruster (s_exhaust_pos7, _V( 0,-1,0), 1.0, ph_sep, 10.0, 10.0);
		th_sep[6] = CreateThruster (s_exhaust_pos8, _V( 1,0,0), 1.0, ph_sep, 10.0, 10.0);
		th_sep[7] = CreateThruster (s_exhaust_pos9, _V( -1,0,0), 1.0, ph_sep, 10.0, 10.0);

		for (i = 0; i < 8; i++) {
			AddExhaustStream (th_sep[i], &seperation_junk);
		}
	}

	VECTOR3 m_exhaust_pos1= {0,0,-9+ offset};

	//
	// orbiter main thrusters
	//

	th_main[0] = CreateThruster (m_exhaust_pos1, _V( 0,0,1), THRUST_THIRD_VAC, ph_3rd, ISP_THIRD_VAC);
	thg_main = CreateThrusterGroup (th_main, 1, THGROUP_MAIN);

	AddExhaust (th_main[0], 30.0, 2.9 ,J2Tex);

	VECTOR3	u_exhaust_pos6= _V(3.6, -0.425, -3.6 + offset);
	VECTOR3 u_exhaust_pos7= _V(-3.6, 0.925, -3.6 + offset);

	th_ver[0] = CreateThruster (u_exhaust_pos6, _V( -0.4,0.0,1), 311 , ph_3rd, 45790.85);
	th_ver[1] = CreateThruster (u_exhaust_pos7, _V( 0.4,0.0,1), 311 , ph_3rd, 45790.85);

	for (int i = 0; i < 2; i++)
		AddExhaust (th_ver[i], 5.0, 0.25, exhaust_tex);

	thg_ver = CreateThrusterGroup (th_ver, 2, THGROUP_USER);
	SetSIVbCMixtureRatio(MixtureRatio);

}

void SaturnV::SeparateStage (int new_stage)

{
	VESSELSTATUS vs1;
	VESSELSTATUS vs2;

	VECTOR3 ofs1 = _V(0,0,0);
	VECTOR3 ofs2 = _V(0,0,0);
	VECTOR3 vel1 = _V(0,0,0);
	VECTOR3 vel2 = _V(0,0,0);

	GetStatus (vs1);
	GetStatus (vs2);

	vs1.eng_main = vs1.eng_hovr = 0.0;
	vs2.eng_main = vs2.eng_hovr = 0.0;

	if (stage == LAUNCH_STAGE_ONE && !bAbort)
	{
		ofs1 = OFS_STAGE1;
		vel1 = _V(0, 0, -4.0);
	}

	if ((stage == PRELAUNCH_STAGE || stage == LAUNCH_STAGE_ONE) && bAbort )
	{
		ofs1= OFS_ABORT;
		vel1 = _V(0,0,-4.0);
	}

	if (stage == LAUNCH_STAGE_TWO && !bAbort)
	{
		ofs1 = OFS_STAGE12;
		vel1 = _V(0,0,-4.0);
	}

	if ((stage == LAUNCH_STAGE_TWO || stage == LAUNCH_STAGE_TWO_ISTG_JET) && new_stage == CM_STAGE)
	{
		ofs1= OFS_ABORT2;
		vel1 = _V(0,0,-4.0);
	}

	if (stage == LAUNCH_STAGE_TWO_ISTG_JET && new_stage != CM_STAGE)
	{
	 	ofs1 = OFS_STAGE2;
		vel1 = _V(0,0,-6.0);
	}

	if (stage == LAUNCH_STAGE_SIVB || stage == STAGE_ORBIT_SIVB)
	{
	 	ofs1 = _V(0.0, 0.0, S4Offset); // OFS_STAGE3;
		vel1 = _V(0.0 ,0.0, 0.0);
	}

	if (stage == CSM_LEM_STAGE)
	{
	 	ofs1 = OFS_SM;
		vel1 = _V(0,0,-0.1);
	}

	if (stage == CM_STAGE)
	{
		ofs1 = OFS_CM_CONE;
		vel1 = _V(1.0,1.0,1.0);
	}

	VECTOR3 rofs1, rvel1 = {vs1.rvel.x, vs1.rvel.y, vs1.rvel.z};
	VECTOR3 rofs2, rvel2 = {vs2.rvel.x, vs2.rvel.y, vs2.rvel.z};

	Local2Rel (ofs1, vs1.rpos);
	Local2Rel (ofs2, vs2.rpos);

	GlobalRot (vel1, rofs1);
	GlobalRot (vel2, rofs2);

	vs1.rvel.x = rvel1.x+rofs1.x;
	vs1.rvel.y = rvel1.y+rofs1.y;
	vs1.rvel.z = rvel1.z+rofs1.z;
	vs2.rvel.x = rvel2.x+rofs2.x;
	vs2.rvel.y = rvel2.y+rofs2.y;
	vs2.rvel.z = rvel2.z+rofs2.z;

	if (stage == CM_ENTRY_STAGE_TWO)
	{
		SetChuteStage1 ();
	}

    if (stage == LAUNCH_STAGE_ONE && !bAbort )
	{
	    vs1.vrot.x = 0.0025;
		vs1.vrot.y = 0.0025;
		vs1.vrot.z = 0.0;
		vs2.vrot.x = 0.0;
		vs2.vrot.y = 0.0;
		vs2.vrot.z = 0.0;

		StageS.play(NOLOOP, 255);

		//
		// Seperate off the S1C stage and show it.
		//

		if (hstg1) {
			S1C *stage1 = (S1C *) oapiGetVesselInterface(hstg1);
			stage1->DefSetState(&vs1);

			S1CSettings S1Config;

			S1Config.SettingsType.S1C_SETTINGS_ENGINES = 1;
			S1Config.SettingsType.S1C_SETTINGS_FUEL = 1;
			S1Config.SettingsType.S1C_SETTINGS_GENERAL = 1;
			S1Config.SettingsType.S1C_SETTINGS_MASS = 1;

			S1Config.RetroNum = SI_RetroNum;
			S1Config.EmptyMass = SI_EmptyMass;
			S1Config.MainFuelKg = GetPropellantMass(ph_1st);
			S1Config.MissionTime = MissionTime;
			S1Config.Realism = Realism;
			S1Config.VehicleNo = VehicleNo;
			S1Config.ISP_FIRST_SL = ISP_FIRST_SL;
			S1Config.ISP_FIRST_VAC = ISP_FIRST_VAC;
			S1Config.THRUST_FIRST_VAC = THRUST_FIRST_VAC;
			S1Config.CurrentThrust = GetThrusterLevel(th_main[0]);
			S1Config.LowRes = LowRes;
			S1Config.Stretched = false;
			S1Config.S4Interstage = (SaturnType == SAT_INT20);
			S1Config.EngineNum = SI_EngineNum;

			stage1->SetState(S1Config);
		}

		ConfigureStageMeshes (new_stage);
		ConfigureStageEngines (new_stage);

		//
		// Fire 'seperation' thrusters.
		//

		if (viewpos != SATVIEW_ENG1 && viewpos != SATVIEW_ENG2) 
		{
			FireSeperationThrusters(th_sep);
		}

		if (viewpos == SATVIEW_ENG1) 
		{
			oapiSetFocusObject(hstg1);
			oapiCameraAttach(hstg1, CAM_COCKPIT);
		}

		if (SaturnType == SAT_INT20)
		{
			//
			// What's the correct value for the INT20?
			//
			ShiftCentreOfMass(_V(0, 0, STG0O + STG2O - 24.5));
		}
		else
		{
			ShiftCentreOfMass(_V(0, 0, STG0O + STG1O));
		}
	}

	if (stage == LAUNCH_STAGE_TWO && !bAbort )
	{
	    vs1.vrot.x = 0.025;
		vs1.vrot.y = 0.025;
		vs1.vrot.z = 0.0;
		vs2.vrot.x = 0.0;
		vs2.vrot.y = 0.0;
		vs2.vrot.z = 0.0;

		CrashBumpS.play(NOLOOP, 150);

		char VName[256], *CName;

		GetApolloName(VName);
		strcat (VName, "-INTSTG");

		switch (SII_UllageNum) {
		case 4:
			CName = "ProjectApollo/sat5intstg4";
			break;

		case 8:
			CName = "ProjectApollo/sat5intstg8";
			break;

		default:
			CName = "ProjectApollo/sat5intstg";
			break;
		}

		hintstg = oapiCreateVessel(VName, CName, vs1);
		InterstageAttached = false;

		//
		// Fire 'seperation' thrusters.
		//

		if (viewpos != SATVIEW_ENG1 && viewpos != SATVIEW_ENG2) 
		{
			FireSeperationThrusters(th_sep2);
		}

		ConfigureStageMeshes (new_stage);
	}

	if (stage == LAUNCH_STAGE_TWO_ISTG_JET && new_stage != CM_STAGE)
	{
	    vs1.vrot.x = 0.025;
		vs1.vrot.y = 0.025;
		vs1.vrot.z = 0.0;
		vs2.vrot.x = 0.0;
		vs2.vrot.y = 0.0;
		vs2.vrot.z = 0.0;

		StageS.play();

		//
		// Seperate off the SII stage and initialise it.
		//

		char VName[256];

		GetApolloName(VName);
		strcat (VName, "-STG2");
		hstg2 = oapiCreateVessel(VName,"ProjectApollo/sat5stg2",vs1);

		SIISettings S2Config;

		S2Config.SettingsType.SII_SETTINGS_ENGINES = 1;
		S2Config.SettingsType.SII_SETTINGS_FUEL = 1;
		S2Config.SettingsType.SII_SETTINGS_GENERAL = 1;
		S2Config.SettingsType.SII_SETTINGS_MASS = 1;

		S2Config.RetroNum = SII_RetroNum;
		S2Config.EmptyMass = SII_EmptyMass;
		S2Config.MainFuelKg = GetPropellantMass(ph_2nd);
		S2Config.MissionTime = MissionTime;
		S2Config.Realism = Realism;
		S2Config.VehicleNo = VehicleNo;
		S2Config.ISP_SECOND_SL = ISP_SECOND_SL;
		S2Config.ISP_SECOND_VAC = ISP_SECOND_VAC;
		S2Config.THRUST_SECOND_VAC = THRUST_SECOND_VAC;
		S2Config.CurrentThrust = GetThrusterLevel(th_main[0]);
		S2Config.LowRes = LowRes;

		SII *stage2 = static_cast<SII *> (oapiGetVesselInterface(hstg2));
		stage2->SetState(S2Config);

		ConfigureStageMeshes(new_stage);
		ConfigureStageEngines(new_stage);

		//
		// Fire 'seperation' thrusters.
		//

		if (viewpos != SATVIEW_ENG1 && viewpos != SATVIEW_ENG2)
		{
			FireSeperationThrusters(th_sep);
		}

		ShiftCentreOfMass(_V(0, 0, STG2O - STG1O));
	}

	if (stage == LAUNCH_STAGE_SIVB || stage == STAGE_ORBIT_SIVB)
	{
		vs1.vrot.x = 0.0;
		vs1.vrot.y = 0.0;
		vs1.vrot.z = 0.0;

		CreateSIVBStage("ProjectApollo/sat5stg3", vs1, true);

		SeparationS.play(NOLOOP,255);
		SetCSMStage();

		ShiftCentreOfMass(_V(0, 0, 13.15));
		SeparationSpeed = 0.15;
	}

	if (stage == CSM_LEM_STAGE)
	{
		char VName[256];
		vs1.vrot.x = 0.0;
		vs1.vrot.y = 0.0;
		vs1.vrot.z = 0.0;

		//
		// Play appropriate sound for SM seperation.
		//

		if (ApolloExploded) 
		{
			SSMSepExploded.play(NOLOOP, 200);
		}
		else
		{
			SMJetS.play();
		}
		SSMSepExploded.done();

		GetApolloName(VName); strcat (VName, "-SM");
		hSMJet = oapiCreateVessel(VName, "ProjectApollo/SM", vs1);

		SMSettings SMConfig;

		SMConfig.SettingsType.SM_SETTINGS_ENGINES = 1;
		SMConfig.SettingsType.SM_SETTINGS_FUEL = 1;
		SMConfig.SettingsType.SM_SETTINGS_GENERAL = 1;
		SMConfig.SettingsType.SM_SETTINGS_MASS = 1;

		SMConfig.EmptyMass = SM_EmptyMass;
		SMConfig.MainFuelKg = GetPropellantMass(ph_sps);
		SMConfig.RCSAFuelKg = GetPropellantMass(ph_rcs0);
		SMConfig.RCSBFuelKg = GetPropellantMass(ph_rcs1);
		SMConfig.RCSCFuelKg = GetPropellantMass(ph_rcs2);
		SMConfig.RCSDFuelKg = GetPropellantMass(ph_rcs3);
		SMConfig.MissionTime = MissionTime;
		SMConfig.Realism = Realism;
		SMConfig.VehicleNo = VehicleNo;
		SMConfig.LowRes = LowRes;
		SMConfig.showHGA = !NoHGA;
		SMConfig.A13Exploded = ApolloExploded;

		SM *SMVessel = (SM *) oapiGetVesselInterface(hSMJet);
		SMVessel->SetState(SMConfig);

		// Store CM Propellant 
		double cmprop1 = -1;
		double cmprop2 = -1;
		if (ph_rcs_cm_1) cmprop1 = GetPropellantMass(ph_rcs_cm_1);
		if (ph_rcs_cm_2) cmprop2 = GetPropellantMass(ph_rcs_cm_2);

		SetReentryStage();

		// Restore CM Propellant
		if (cmprop1 != -1) SetPropellantMass(ph_rcs_cm_1, cmprop1); 
		if (cmprop2 != -1) SetPropellantMass(ph_rcs_cm_2, cmprop2); 

		ShiftCentreOfMass(_V(0, 0, 2.1));
	}

	if (stage == CM_STAGE)
	{
		SetChuteStage2 ();
	}

	if (stage == CM_ENTRY_STAGE_TWO)
	{
		SetChuteStage3 ();
	}

	if (stage == CM_ENTRY_STAGE_FOUR)
	{
		SetChuteStage4 ();
	}

	if (stage == CM_ENTRY_STAGE_FIVE)
	{
		SetSplashStage ();
	}

	if ((stage == PRELAUNCH_STAGE || stage == LAUNCH_STAGE_ONE) && bAbort )
	{
		vs1.vrot.x = 0.0;
		vs1.vrot.y = 0.0;
		vs1.vrot.z = 0.0;

		StageS.play();

		char VName[256];
		GetApolloName(VName); strcat (VName, "-ABORT");
		habort = oapiCreateVessel (VName, "ProjectApollo/Saturn5Abort1", vs1);
		SetReentryStage();
		ShiftCentreOfMass(_V(0, 0, STG0O + 23.25));
	}

	if ((stage == LAUNCH_STAGE_TWO || stage == LAUNCH_STAGE_TWO_ISTG_JET) && new_stage == CM_STAGE)
	{
		vs1.vrot.x = 0.0;
		vs1.vrot.y = 0.0;
		vs1.vrot.z = 0.0;

		StageS.play();

		char VName[256];
		GetApolloName(VName); strcat (VName, "-ABORT");
		habort = oapiCreateVessel (VName, "ProjectApollo/Saturn5Abort2", vs1);
		SetReentryStage();
		ShiftCentreOfMass(_V(0, 0, -STG1O + 23.25));
	}
}

void SaturnV::ActivatePrelaunchVenting()

{
	//
	// "tank venting" particle streams
	//
	static double lvl = 1.0;

	if (!prelaunchvent[0]) prelaunchvent[0] = AddParticleStream(&prelaunchvent1_spec, _V(-5.5, -1,   -24.0 + STG0O), _V(-1,  0, 0), &lvl);
	if (!prelaunchvent[2]) prelaunchvent[2] = AddParticleStream(&prelaunchvent3_spec, _V(-3.5,  1,    -3.5 + STG0O), _V(-1,  0, 0), &lvl);
	if (!prelaunchvent[1]) prelaunchvent[1] = AddParticleStream(&prelaunchvent2_spec, _V(-3.7, -3.7, -38.0 + STG0O), _V(-1, -1, 0), &lvl);
}

void SaturnV::DeactivatePrelaunchVenting()

{
	// "tank venting" particle streams
	int i;

	for (i = 0; i < 3; i++) {
		if (prelaunchvent[i]) {
			DelExhaustStream(prelaunchvent[i]);
			prelaunchvent[i] = NULL;
		}
	}
}

void SaturnV::ActivateStagingVent()

{
	// "staging vent" particle streams
	static double lvl = 1.0;

	VECTOR3	m_exhaust_pos6= _V(0,5.07,-33.15-STG1O - 5.);
	VECTOR3 m_exhaust_pos7= _V(0,-5.07,-33.15-STG1O - 5.);
	VECTOR3	m_exhaust_pos8= _V(5.07,0,-33.15-STG1O - 5.);
	VECTOR3 m_exhaust_pos9= _V(-5.07,0,-33.15-STG1O - 5.);
	VECTOR3	m_exhaust_pos10= _V(3.55,3.7,-33.15-STG1O - 5.);
	VECTOR3 m_exhaust_pos11= _V(3.55,-3.7,-33.15-STG1O - 5.);
	VECTOR3	m_exhaust_pos12= _V(-3.55,3.7,-33.15-STG1O - 5.);
	VECTOR3 m_exhaust_pos13= _V(-3.55,-3.7,-33.15-STG1O - 5.);

	if (!stagingvent[0]) stagingvent[0] = AddParticleStream (&stagingvent_spec, m_exhaust_pos10, _V( 1, 1,-1), &lvl);
	if (!stagingvent[1]) stagingvent[1] = AddParticleStream (&stagingvent_spec, m_exhaust_pos11, _V( 1,-1,-1), &lvl);
	if (!stagingvent[2]) stagingvent[2] = AddParticleStream (&stagingvent_spec, m_exhaust_pos12, _V(-1, 1,-1), &lvl);
	if (!stagingvent[3]) stagingvent[3] = AddParticleStream (&stagingvent_spec, m_exhaust_pos13, _V(-1,-1,-1), &lvl);
	if (!stagingvent[4]) stagingvent[4] = AddParticleStream (&stagingvent_spec, m_exhaust_pos6,  _V( 0, 1,-1), &lvl);
	if (!stagingvent[5]) stagingvent[5] = AddParticleStream (&stagingvent_spec, m_exhaust_pos7,  _V( 0,-1,-1), &lvl);
	if (!stagingvent[6]) stagingvent[6] = AddParticleStream (&stagingvent_spec, m_exhaust_pos8,  _V( 1, 0,-1), &lvl);
	if (!stagingvent[7]) stagingvent[7] = AddParticleStream (&stagingvent_spec, m_exhaust_pos9,  _V(-1, 0,-1), &lvl);
}

void SaturnV::DeactivateStagingVent()

{
	//
	// "staging vent" particle streams
	//
	int i;

	for (i = 0; i < 8; i++) {
		if (stagingvent[i]) {
			DelExhaustStream(stagingvent[i]);
			stagingvent[i] = NULL;
		}
	}
}

void SaturnV::CreateStageOne() {

	// Create hidden SIC vessel
	char VName[256];
	VESSELSTATUS vs;

	GetStatus(vs);
	GetApolloName(VName);
	strcat (VName, "-STG1");
	hstg1 = oapiCreateVessel(VName,"ProjectApollo/sat5stg1", vs);

	// Load only the necessary meshes
	S1C *stage1 = (S1C *) oapiGetVesselInterface(hstg1);
	stage1->LoadMeshes(LowRes);
}

