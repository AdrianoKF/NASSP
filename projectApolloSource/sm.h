/***************************************************************************
  This file is part of Project Apollo - NASSP
  Copyright 2004-2005 Jean-Luc Rocca-Serra, Mark Grant

  ORBITER vessel module: Service Module class

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
  *	Revision 1.11  2006/06/28 02:08:11  movieman523
  *	Full workaround for SM deletion crash: though the focus still tends to jump to something other than the CM!
  *	
  *	Revision 1.10  2006/06/28 01:23:02  movieman523
  *	Made SM break up on re-entry. Unfortunately sound doesn't work and if I try to delete the 'parts' when the SM is deleted. Orbiter blows away.
  *	
  *	Revision 1.9  2006/06/27 22:29:43  movieman523
  *	HGA now breaks off of the SM on re-entry, and fixed a bug on CM/SM seperation.
  *	
  *	Revision 1.8  2006/06/26 19:05:36  movieman523
  *	More doxygen, made Lunar EVA a VESSEL2, made SM breakup, made LRV use VESSEL2 save/load functions.
  *	
  *	Revision 1.7  2006/06/25 21:19:45  movieman523
  *	Lots of Doxygen updates.
  *	
  *	Revision 1.6  2006/05/30 22:34:33  movieman523
  *	Various changes. Panel switches now need power, APO and PER correctly placed in scenario fle, disabled some warnings, moved 'window' sound message to the correct place, added heat measurement to SM DLL for re-entry.
  *	
  *	Revision 1.5  2006/05/04 20:46:50  movieman523
  *	Added re-entry texture and started heat tracking.
  *	
  *	Revision 1.4  2006/04/05 19:33:49  movieman523
  *	Support low-res RCS mesh, saved umbilical animation state so it only happens once, revised Apollo 13 support.
  *	
  *	Revision 1.3  2006/04/04 22:00:54  jasonims
  *	Apollo Spacecraft Mesh offset corrections and SM Umbilical Animation.
  *	
  *	Revision 1.2  2006/03/30 01:59:37  movieman523
  *	Added RCS to SM DLL.
  *	
  *	Revision 1.1  2006/03/30 00:21:37  movieman523
  *	Pass empty mass correctly and remember to check in SM files :).
  *	
  *	
  **************************************************************************/

///
/// Flags structure indicating which of the SM  settings are valid.
///
/// \brief SM settings flags.
/// \ingroup SepStageSettings
///
union SMSettingFlags
{
	struct {
		unsigned SM_SETTINGS_MASS:1; 		///< Mass settings are valid.
		unsigned SM_SETTINGS_FUEL:1;		///< Fuel mass settings are valid.
		unsigned SM_SETTINGS_GENERAL:1;		///< General settings (e.g. Mission Time) are valid.
		unsigned SM_SETTINGS_ENGINES:1;		///< Engine settings are valid.
	};
	unsigned int word;						///< Set to zero to clear all flags.

	///
	/// \brief Constructor: clear all flags by default.
	///
	SMSettingFlags() { word = 0; };
};

///
/// Data structure passed from main vessel to SM to configure it after staging.
///
/// \brief SM setup structure.
/// \ingroup SepStageSettings
///
typedef struct {

	SMSettingFlags SettingsType;		///< Which parts are valid.

	int VehicleNo;						///< Saturn vehicle number.
	int Realism;						///< Realism level.

	double MissionTime;					///< Current MET in seconds.
	double EmptyMass;					///< Empty mass in kg.
	double MainFuelKg;					///< SPS fuel in kg.
	double RCSAFuelKg;					///< RCS Quad A fuel in kg.
	double RCSBFuelKg;					///< RCS Quad B fuel in kg.
	double RCSCFuelKg;					///< RCS Quad C fuel in kg.
	double RCSDFuelKg;					///< RCS Quad B fuel in kg.

	bool LowRes;						///< Using low-res meshes?
	bool showHGA;						///< Do we have an HGA?
	bool A13Exploded;					///< Did the SM explode on Apollo 13 mission?

} SMSettings;

//
// Stage states.
//

///
/// Specifies the main state of the SM
///
/// \brief SM state.
/// \ingroup SepStageSettings
///
typedef enum SMState
{
	SM_STATE_SETUP = -1,				///< SM is waiting for setup call.
	SM_UMBILICALDETACH_PAUSE,			///< SM umbilical is detaching
	SM_STATE_RCS_START,					///< SM is starting the RCS motors to jettison.
	SM_STATE_RCS_ROLL_START,			///< SM is starting RCS roll.
	SM_STATE_RCS_ROLL_STOP,				///< SM is stopping RCS roll.
	SM_STATE_WAITING					///< SM is idle. RCS may still be operating.
};

///
/// \brief Speed at which the SM to CM umbilical moves away from the CM.
///
const double UMBILICAL_SPEED = 0.5;

///
/// This code simulates the seperated Service Module. If the SM RCS has power and fuel it fires the RCS to push
/// the SM away from the CM and set it rotating. Then it disintegrates during re-entry.
///
/// \brief SM stage simulation.
/// \ingroup SepStages
///
class SM : public VESSEL2 {

public:
	///
	/// \brief Standard constructor with the usual Orbiter parameters.
	///
	SM (OBJHANDLE hObj, int fmodel);
	virtual ~SM();

	///
	/// \brief Orbiter state saving function.
	/// \param scn Scenario file to save to.
	///
	void clbkSaveState (FILEHANDLE scn);

	///
	/// \brief Orbiter timestep function.
	/// \param simt Current simulation time, in seconds since Orbiter was started.
	/// \param simdt Time in seconds since last timestep.
	/// \param mjd Current MJD.
	///
	void clbkPreStep(double simt, double simdt, double mjd);

	///
	/// \brief Orbiter state loading function.
	/// \param scn Scenario file to load from.
	/// \param status Pointer to current vessel status.
	///
	void clbkLoadStateEx (FILEHANDLE scn, void *status);

	///
	/// \brief Orbiter class configuration function.
	/// \param cfg File to load configuration defaults from.
	///
	void clbkSetClassCaps (FILEHANDLE cfg);

	///
	/// \brief Orbiter dock state function. Does the SM need this?
	///
	void clbkDockEvent(int dock, OBJHANDLE connected);

	///
	/// Pass settings from the main DLL to the jettisoned SM. This call must be virtual 
	/// so it can be called from other DLLs without building in the SM code.
	/// \brief Setup jettisoned SM.
	/// \param state SM state settings.
	///
	virtual void SetState(SMSettings &state);

	///
	/// The CSM should call this function to tidy up seperated meshes before deleting the
	/// SM. This is virtual so it can be called from other DLLs without builing in the
	/// SM code.
	///
	/// We have to delete each mesh one at a time to avoid an Orbiter bug where it crashes
	/// if you delete multiple vessels in the same timestep.
	///
	/// \brief Tidy up seperated meshes.
	/// \param hCamera Saturn handle: probably not really needed.
	/// \return True if it deleted a mesh, false if all meshes were deleted already.
	///
	virtual bool TidyUpMeshes(OBJHANDLE hCamera);

protected:

	void SetSM();
	void InitSM();
	void AddEngines();
	void DefineAnimations();

	///
	/// \brief Setup variables on first timestep.
	///
	void DoFirstTimestep();

	///
	/// \brief Get the Apollo vehicle name.
	///
	void GetApolloName(char *s);

	///
	/// \brief If the handle isn't zero, delete the vessel.
	/// \param handle Vessel handle to delete.
	/// \param hCamera Camera handle.
	///
	bool TryToDelete(OBJHANDLE &handle, OBJHANDLE hCamera);

	///
	/// \brief If the handle isn't zero, add a re-entry texture to the object.
	/// \param handle Vessel handle to update.
	///
	void AddReentryTextureToObject(OBJHANDLE handle);

	int GetMainState();
	void SetMainState(int s);

	///
	/// \brief Umbilical animation state (0.0 = attached, 1.0 = open).
	///
	double umbilical_proc;

	///
	/// \brief Umbilical animation ID.
	///
	UINT anim_umbilical;

	///
	/// \brief Apollo mission number.
	///
	int MissionNo;

	///
	/// \brief Saturn vehicle number.
	///
	int VehicleNo;

	///
	/// \brief SM mesh identifier
	///
	int SMMeshIndex;

	///
	/// \brief Main state.
	///
	SMState State;

	///
	/// \brief Realism level.
	///
	int Realism;

	bool RetrosFired;

	///
	/// \brief Using low-res meshes?
	///
	bool LowRes;

	///
	/// \brief Is this the first timestep?
	///
	bool FirstTimestep;

	//
	// Which parts to display?
	//

	///
	/// \brief Show the SPS engine bell.
	///
	bool showSPS;

	///
	/// \brief Show the RCS panels.
	///
	bool showRCS;

	///
	/// \brief Show exterior panel 1.
	///
	bool showPanel1;

	///
	/// \brief Show exterior panel 2.
	///
	bool showPanel2;

	///
	/// \brief Show exterior panel 3.
	///
	bool showPanel3;

	///
	/// \brief Show exterior panel 4.
	///
	bool showPanel4;

	///
	/// \brief Show exterior panel 5.
	///
	bool showPanel5;

	///
	/// \brief Show exterior panel 6.
	///
	bool showPanel6;

	///
	/// \brief Show the High-Gain Antenna.
	///
	bool showHGA;

	///
	/// \brief Show the interior cryogenics tanks.
	///
	bool showCRYO;

	///
	/// \brief This is an Apollo 13 SM which exploded in flight.
	///
	bool A13Exploded;

	///
	/// \brief Empty mass in kg.
	///
	double EmptyMass;

	///
	/// \brief SPS fuel in kg.
	///
	double MainFuel;

	///
	/// \brief RCS quad A fuel in kg.
	///
	double RCSAFuel;

	///
	/// \brief RCS quad B fuel in kg.
	///
	double RCSBFuel;

	///
	/// \brief RCS quad C fuel in kg.
	///
	double RCSCFuel;

	///
	/// \brief RCS quad D fuel in kg.
	///
	double RCSDFuel;

	///
	/// The current Mission Elapsed Time. This is the main variable used for timing
	/// automated events during the mission, giving the time in seconds from launch
	/// (negative for the pre-launch countdown).
	/// \brief Mission Elapsed Time.
	///
	double MissionTime;

	///
	/// The time in seconds of the next automated event to occur in the mission. This 
	/// is a generic value used by the autopilot code.
	/// \brief Time of next event.
	///
	double NextMissionEventTime;

	///
	/// The time in seconds of the previous automated event that occur in the mission. This 
	/// is a generic value used by the autopilot code.
	/// \brief Time of last event.
	///
	double LastMissionEventTime;

	///
	/// \brief Temperature buildup during re-entry.
	///
	double Temperature;

	///
	/// \brief Re-entry texture.
	///
	SURFHANDLE CMTex;

	///
	/// \brief RCS quad fuel tanks.
	///
	PROPELLANT_HANDLE ph_rcsa, ph_rcsb, ph_rcsc, ph_rcsd;

	///
	/// \brief RCS thrusters.
	///
	THRUSTER_HANDLE th_att_lin[24], th_att_rot[24], th_rcs_a[5], th_rcs_b[5], th_rcs_c[5], th_rcs_d[5];

	///
	/// \brief Seperated HGA handle.
	///
	OBJHANDLE hHGA;

	///
	/// \brief Seperated SPS handle.
	///
	OBJHANDLE hSPS;

	///
	/// \brief Seperated Panel 1 handle.
	///
	OBJHANDLE hPanel1;

	///
	/// \brief Seperated Panel 2 handle.
	///
	OBJHANDLE hPanel2;

	///
	/// \brief Seperated Panel 3 handle.
	///
	OBJHANDLE hPanel3;

	///
	/// \brief Seperated Panel 4 handle.
	///
	OBJHANDLE hPanel4;

	///
	/// \brief Seperated Panel 5 handle.
	///
	OBJHANDLE hPanel5;

	///
	/// \brief Seperated Panel 6 handle.
	///
	OBJHANDLE hPanel6;

	///
	/// \brief Sound library.
	///
	SoundLib soundlib;

	///
	/// \brief Breakup sound.
	///
	Sound BreakS;
};
