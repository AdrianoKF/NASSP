/***************************************************************************
  This file is part of Project Apollo - NASSP
  Copyright 2004-2007 Mark Grant

  ORBITER vessel module: CSM caution and warning system code.

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
  *	Revision 1.21  2007/07/17 14:33:07  tschachim
  *	Added entry and post landing stuff.
  *	
  *	Revision 1.20  2007/01/14 13:02:42  dseagrav
  *	CM AC bus feed reworked. Inverter efficiency now varies, AC busses are 3-phase all the way to the inverter, inverter switching logic implemented to match the CM motor-switch lockouts. Original AC bus feeds deleted. Inverter overload detection enabled and correct.
  *	
  *	Revision 1.19  2007/01/06 04:44:49  dseagrav
  *	Corrected CREW ALARM command behavior, PCM downtelemetry generator now draws power
  *	
  *	Revision 1.18  2006/11/13 14:47:30  tschachim
  *	New SPS engine.
  *	New ProjectApolloConfigurator.
  *	Fixed and changed camera and FOV handling.
  *	
  *	Revision 1.17  2006/10/26 18:48:50  movieman523
  *	Fixed up CM RCS 1 and 2 warning lights to make the 'C&WS Operational Check' work.
  *	
  *	Revision 1.16  2006/05/30 14:40:21  tschachim
  *	Fixed fuel cell - dc bus connectivity, added battery charger
  *	
  *	Revision 1.15  2006/02/28 20:40:32  quetalsi
  *	Bugfix and added CWS FC BUS DISCONNECT. Reset DC switches now work.
  *	
  *	Revision 1.14  2006/02/23 22:46:41  quetalsi
  *	Added AC ovevoltage control and Bugfix
  *	
  *	Revision 1.13  2006/02/22 20:14:46  quetalsi
  *	C&W  AC_BUS1/2 light and AC RESET SWITCH now woks.
  *	
  *	Revision 1.12  2006/02/18 21:39:42  tschachim
  *	Bugfix
  *	
  *	Revision 1.11  2006/02/13 21:42:31  tschachim
  *	C/W ISS light.
  *	
  *	Revision 1.10  2006/01/14 20:58:16  movieman523
  *	Revised PowerSource code to ensure that classes which must be called each timestep are registered with the Panel SDK code.
  *	
  *	Revision 1.9  2006/01/07 01:34:08  movieman523
  *	Added AC bus overvoltage and main bus undervolt lights.
  *	
  *	Revision 1.8  2005/11/18 20:38:59  movieman523
  *	Revised condensor output from fuel cell to eliminate master alarms.
  *	
  *	Revision 1.7  2005/11/16 23:50:31  movieman523
  *	More updates to CWS operation. Still not completely correct, but closer.
  *	
  *	Revision 1.6  2005/11/16 23:14:01  movieman523
  *	Initial support for wiring in the circuit breakers.
  *	
  *	Revision 1.5  2005/10/11 16:50:01  tschachim
  *	Added more alarms.
  *	
  *	Revision 1.4  2005/08/21 16:23:32  movieman523
  *	Added more alarms.
  *	
  *	Revision 1.3  2005/08/21 13:13:43  movieman523
  *	Wired in a few caution and warning lights.
  *	
  *	Revision 1.2  2005/08/21 11:51:59  movieman523
  *	Initial version of CSM caution and warning lights: light test switch now works.
  *	
  *	Revision 1.1  2005/08/13 14:59:24  movieman523
  *	Added initial null implementation of CSM caution and warning system, and removed 'master alarm' flag from Saturn class.
  *	
  **************************************************************************/


#if !defined(_PA_CSMCAUTIONWARNING_H)
#define _PA_CSMCAUTIONWARNING_H

// moved from Saturn.h as "foreward reference" because of FuelCellBad

///
/// Fuel cell status structure. This is used to determine whether the fuel cell
/// is working OK.
///
/// \ingroup InternalInterface
///
typedef struct {
	double H2FlowLBH;			///< H2 flow in pounds per hour.
	double O2FlowLBH;			///< O2 flow in pounds per hour.
	double TempF;				///< Temperature in fahrenheit.
	double CondenserTempF;		///< Condensor temperature in fahrenheit.
	double CoolingTempF;		///< Cooling temperature in fahrenheit.
	double Voltage;				///< Output voltage.
	double Current;				///< Output current.
	double PowerOutput;			///< Output power.
} FuelCellStatus;

///
/// AC bus status. This is used to determine whether the bus status is OK.
///
/// \ingroup InternalInterface
///
typedef struct {
	double ACBusVoltage;		///< Bus voltage.
	double ACBusCurrent;		///< Bus current.
	double Phase1Current;		///< Bus phase 1 current.
	double Phase2Current;		///< Bus phase 2 current.
	double Phase3Current;		///< Bus phase 3 current.
	double Phase1Voltage;		///< Bus phase 1 voltage.
	double Phase2Voltage;		///< Bus phase 2 voltage.
	double Phase3Voltage;		///< Bus phase 3 voltage.
	bool Enabled_AC_CWS;
	bool Reset_AC_CWS;
} ACBusStatus;

///
/// \brief The CSM-specific Caution and Warning System.
/// \ingroup InternalSystems
///
class CSMCautionWarningSystem : public CautionWarningSystem {

public:
	///
	/// \brief Constructor.
	/// \param mastersound Master Alarm sound.
	/// \param buttonsound Button press sound.
	/// \param p Panel SDK describing spacecraft systems.
	///
	CSMCautionWarningSystem(Sound &mastersound, Sound &buttonsound, PanelSDK &p);

	///
	/// \brief Timestep processing.
	/// \param simt Current Mission Time.
	///
	void TimeStep(double simt);

	///
	/// \brief Render the CWS lights.
	/// \param surf Surface to render to.
	/// \param lightsurf Surface for the light bitmaps.
	/// \param leftpanel Is this the left or right panel?
	///
	void RenderLights(SURFHANDLE surf, SURFHANDLE lightsurf, bool leftpanel);

protected:

	//
	// Don't need to be saved.
	//

	///
	/// Mission time of the next update.
	///
	double NextUpdateTime;

	///
	/// How many timesteps have we processed? Since the system state can take a few timesteps to
	/// settle down, we ignore the first few timesteps after starting up.
	///
	int TimeStepCount;

	///
	/// Next time to check the O2 flow rate.
	///
	double NextO2FlowCheckTime;

	///
	/// Was the last O2 flow rate high?
	///
	bool LastO2FlowCheckHigh;

	///
	/// How many times have we checked the O2 flow rate?
	///
	int O2FlowCheckCount;
	int SPSPressCheckCount;
	int CryoPressCheckCount;
	int GlycolTempCheckCount;
	int FuelCellCheckCount[4];
	bool ACBus1Alarm, ACBus2Alarm;
	bool ACBus1Reset, ACBus2Reset;

	//
	// Helper functions.
	//

	///
	/// \brief Render one of the light panels.
	///
	/// This function renders an entire light panel as appropriate for the current light state and
	/// testing state.
	///
	/// \param surf Surface to render to.
	/// \param lightsurf Light panel image surface.
	/// \param LightState Pointer to an array of light states for the panel to render.
	/// \param LightTest Is this a light test?
	/// \param sdx X offset for light bitmaps in the panel image surface.
	/// \param sdy Y offset for light bitmaps in the panel image surface.
	/// \param base Light number base for this panel.
	///
	void RenderLightPanel(SURFHANDLE surf, SURFHANDLE lightsurf, bool *LightState, bool LightTest, int sdx, int sdy, int base);

	///
	/// Check the fuel cell status to determine whether it's in a 'bad' state that we
	/// should warn the crew about.
	///
	/// \param fc Fuel cell status information.
	/// \param index This fuel cell index.
	/// \return True if the cell is in a 'bad' state.
	///
	bool FuelCellBad(FuelCellStatus &fc, int index);

	///
	/// Check the specified AC bus to determine whether it's overloaded.
	///
	/// \param bus AC bus to check.
	/// \return True if the bus is overloaded.
	///
	bool ACOverloaded(int bus);

	///
	/// Check for an AC bus undervoltage.
	///
	/// \param as AC bus status information.
	/// \return True if the bus voltage is too low.
	///
	bool ACUndervoltage(ACBusStatus &as);

	///
	/// Check whether the AC bus has overvoltage.
	///
	/// \param as AC bus status information.
	/// \return True if the bus voltage is too high.
	///
	bool ACOvervoltage(ACBusStatus &as);

	///
	/// Check whether a light is powered.
	///
	/// \param i Light number.
	/// \return True if the light is powered.
	///
	bool LightPowered(int i);
};

//
// Caution and warning light numbers for the CM panel.
//

#define CSM_CWS_CO2_LIGHT			3			///< CSM CO2 warning light.
#define CSM_CWS_PITCH_GIMBAL1		4			///< CSM Gimbal 1 pitch warning light.
#define CSM_CWS_YAW_GIMBAL1			5			///< CSM Gimbal 1 yaw warning light.
#define CSM_CWS_HIGAIN_LIMIT		6			///< CSM High-Gain Antenna limit warning light.
#define CSM_CWS_PITCH_GIMBAL2		8			///< CSM Gimbal 2 pitch warning light.
#define CSM_CWS_YAW_GIMBAL2			9			///< CSM Gimbal 2 yaw warning light.
#define CSM_CWS_CRYO_PRESS_LIGHT	10			///< CSM low cryogenic pressure warning light.
#define CSM_CWS_GLYCOL_TEMP_LOW		11			///< CSM low glycol temperature warning light.
#define CSM_CWS_CM_RCS_1			12			///< CM RCS system 1 warning light.
#define CSM_CWS_CM_RCS_2			13			///< CM RCS system 2 warning light.
#define CSM_CWS_SM_RCS_A			16			///< CSM RCS quad A warning light.
#define CSM_CWS_SM_RCS_B			17			///< CSM RCS quad B warning light.
#define CSM_CWS_SM_RCS_C			18			///< CSM RCS quad C warning light.
#define CSM_CWS_SM_RCS_D			19			///< CSM RCS quad D warning light.
#define CSM_CWS_FC1_LIGHT			31			///< CSM Fuel Cell 1 warning light.
#define CSM_CWS_FC2_LIGHT			32			///< CSM Fuel Cell 2 warning light.
#define CSM_CWS_FC3_LIGHT			33			///< CSM Fuel Cell 3 warning light.
#define CSM_CWS_SPS_PRESS			38			///< CSM Service Propulsion System warning light.
#define CSM_CWS_AC_BUS1_LIGHT		40			///< CSM AC Bus 1 warning light.
#define CSM_CWS_AC_BUS2_LIGHT		41			///< CSM AC Bus 2 warning light.
#define CSM_CWS_FC_BUS_DISCONNECT   43			///< CSM Fuel Cell bus disconnect warning light.
#define CSM_CWS_AC_BUS1_OVERLOAD	44			///< CSM AC Bus 1 overload warning light.
#define CSM_CWS_AC_BUS2_OVERLOAD	45			///< CSM AC Bus 2 overload warning light.
#define CSM_CWS_CMC_LIGHT			46			///< CSM CMC warning light.
#define CSM_CWS_CREW_ALERT			47			///< CSM Crew Alert light.
#define CSM_CWS_BUS_A_UNDERVOLT		48			///< CSM Main Bus A undervoltage warning light.
#define CSM_CWS_BUS_B_UNDERVOLT		49			///< CSM Main Bus B undervoltage warning light.
#define CSM_CWS_ISS_LIGHT			50			///< CSM AGC ISS warning light.
#define CSM_CWS_CWS_POWER			51			///< CSM Caution and Warning System power warning light.
#define CSM_CWS_O2_FLOW_HIGH_LIGHT	52			///< CSM high oxygen flow rate warning light.
#define CSM_CWS_SUIT_COMPRESSOR		53			///< CSM suit compressor warning light.

#endif
