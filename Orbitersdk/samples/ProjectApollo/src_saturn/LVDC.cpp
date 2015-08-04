/***************************************************************************
  This file is part of Project Apollo - NASSP
  Copyright 2004-2005

  Launch Vehicle Digital Computer (C++ Implementation)

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

  **************************************************************************/

#pragma once
#include "OrbiterAPI.h"
#include "Orbitersdk.h"
#include "stdio.h"
#include "math.h"
#include "resource.h"

#include "nasspdefs.h"
#include "nasspsound.h"

#include "soundlib.h"
#include "toggleswitch.h"

#include "apolloguidance.h"
#include "csmcomputer.h"
#include "dsky.h"
#include "IMU.h"
#include "papi.h"
#include "saturn.h"
#include "saturn1b.h"
#include "s1b.h"
#include "LVDC.h"

LVDC1B::LVDC1B(){
	Initialized = false;					// Reset cloberness flag
}

void LVDC1B::init(Saturn1b* own){
	if(Initialized == true){ 
		if(owner == own){
			fprintf(lvlog,"init called after init, ignored\r\n");
			fflush(lvlog);
			return;
		}else{
			fprintf(lvlog,"init called after init with new owner, proceeding\r\n");
			fflush(lvlog);
		}
	}
	owner = own;
	lvimu.Init();							// Initialize IMU
	lvrg.Init(owner);						// LV Rate Gyro Package
	lvimu.SetVessel(owner);					// set vessel pointer
	lvimu.CoarseAlignEnableFlag = false;	// Clobber this
	//presettings in order of boeing listing for easier maintainece
	//GENERAL
	e = 0;
	f = 0;
	C_3 = -60731530.2; // Stored as twice the etc etc.
	  //C_3 = -60903382.7388059;
	GATE = false;							// 'chi-freeze-gate': freezes steering commands when true
	GATE5 = false;							// allows single pass through HSL initialization when false
	INH = false;							// inhibits restart preparations; set by x-lunar inject/inhibit switch
	INH1 = false;							// inhibits first EPO roll/pitch maneuver
	INH2 = false;							// inhibits second EPO roll/pitch maneuver
	TA1 = 9780;								// time to start pitchdown maneuver
	TA2 = 10275;							// time for attitude hold
	TA3 = 11820;							// time for final attitude
	T_LET = 25;								// LET Jettison Time, i.e. the time IGM starts after start of TB3
	UP = 0;									// switching variable for Tt_t parameter update
	//alpha _d_op?
	i_op = true;							// flag for selecting method of EPO inclination calculation
	theta_N_op = true;						// flag for selecting method of EPO descending node calculation
	//PRE_IGM GUIDANCE
	B_11 = -0.62;							// Coefficients for determining freeze time after S1C engine failure
	B_12 = 40.9;							// dto.
	B_21 = -0.3611;							// dto.
	B_22 = 29.25;							// dto.
	// PITCH POLYNOMIAL (Apollo 9)
	Fx[1][0] =  3.19840;
	Fx[1][1] =  -0.544236;
	Fx[1][2] =  0.0351605;
	Fx[1][3] =  -0.00116379;
	Fx[1][4] =  0.000011386;
	Fx[2][0] =  -10.9607;
	Fx[2][1] =  0.946620;
	Fx[2][2] =  -0.0294206;
	Fx[2][3] =  0.000207717;
	Fx[2][4] =  -0.000000439036;
	Fx[3][0] =  78.7826;
	Fx[3][1] =  -2.83749;
	Fx[3][2] =  0.0289710;
	Fx[3][3] =  -0.000178363;
	Fx[3][4] =  0.000000463029;
	Fx[4][0] =  69.9191;
	Fx[4][1] =  -2.007490;
	Fx[4][2] =  0.0105367;
	Fx[4][3] =  -0.0000233163;
	Fx[4][4] =  0.0000000136702;
	t_1 = 10;								// Backup timer for Pre-IGM pitch maneuver
	t_2 = 25;								// Time to initiate pitch freeze for S1C engine failure
	t_3 = 36;								// Constant pitch freeze for S1C engine failure prior to t_2
	t_3i=0;									// Clock time at S4B ignition
	t_4 = 45;								// Upper bound of validity for first segment of pitch freeze
	t_5 = 81;								// Upper bound of validity for second segment of pitch freeze
	t_6 = 0;								// Time to terminate pitch freeze after S1C engine failure
	T_ar = 134.3;								// S1C Tilt Arrest Time	   
	T_S1 = 33.6;							// Pitch Polynomial Segment Times
	T_S2 = 68.6;							// dto.
	T_S3 = 95.6;							// dto.
	T_EO1 = 0;								// switching constant for SIC engine-out handling
	T_EO2 = 0;								// allows single pass through IGM engine-out presettings when 0
	dt = 1.7;								// Nominal powered-flight or coast-guidance computation-cycle interval
	dT_F=0;									// Period of frozen pitch in S1C
	dt_LET = 25;							// Nominal time between SII ign and LET jet
	t_fail =0;								// S1C Engine Failure time
	CommandRateLimits=_V(1*RAD,1*RAD,1*RAD);// Radians per second
	//IGM BOOST TO ORBIT
	cos_phi_L = 0.878635524;					// cos of the Geodetic Launch site latitude
	// Inclination from azimuth polynomial
	fx[0] = 32.55754;  fx[1] = -15.84615; fx[2] = 11.64780; fx[3] = 9.890970;
	fx[4] = -5.111430; fx[5] = 0;         fx[6] = 0;
	// Descending Node Angle from azimuth polynomial
	gx[0] = 123.1935; gx[1] = -55.06485; gx[2] = -35.26208; gx[3] = 26.01324;
	gx[4] = -1.47591; gx[5] = 0;         gx[6] = 0;			
	MRS = false;							// MR Shift
	dotM_1 = 242.7976615;						// Mass flowrate of SIVB from approximately LET jettison to second MRS
	dotM_2 = 183.3909139;						// Mass flowrate of SIVB after second MRS
	dV_B = 0.4; // AP11// dV_B = 2.0275; // AP9// Velocity cutoff bias for orbital insertion
	ROV = 1.11706196363037;
	ROVs = 1.5;
	sin_phi_L = 0.477493054;					// sin of the Geodetic Launch site latitude
	SMCG = 0.05*RAD;
	TSMC1 = 20; TSMC2 = 5; // AP9
	// TSMC1 = 60.6 TSMC2 = 15 // AP11
	T_1 = 286;								// Time left in first-stage IGM
	T_2 = 0;								// Time left in second and fourth stage IGM
	Tt_2 = 150;								// Estimated third or fifth stage burn time
	Tt_T = T_1 + Tt_2;						// Time-To-Go computed using Tt_3
	t = 0;									// Time from accelerometer reading to next steering command
	t_B1 = 2;								// Transition time for the S2 mixture ratio to shift from 5.5 to 4.7
	t_B3 = 0;								// Time from second S2 MRS signal
	//dt: not set; dependend on cycle time
	V_ex1 = 4145.76977;
	V_ex2 = 4178.68462;
	V_TC = 300;
	
	eps_2 = 35;								// Time to begin chi bar steering
	eps_3 = 10000;							// Terminal condition freeze time
	eps_4 = 3;								// Time to enable HSL loop & chi freeze
	mu =398600420000000;					// Product of G and Earth's mass
	tau2 = 324;//tau2 = 722.67;		// Time to consume all fuel between MRS and S2 Cutoff
	//rate limits: set in pre-igm
	alpha_1 = 0;									//orbital guidance pitch
	alpha_2 = 0;									//orbital guidance yaw
	K_P1 = 4.3 * RAD;							// restart attitude coefficients
	K_P2 = 0;
	K_Y1 = 0;
	K_Y2 = 0;

	//Not in boeing doc, but needed for nav:
	a = 6378137;							//earth's equatorial radius
	J = 0.0010826;							//first coefficient of earth's gravity

	//'real' software variable, i.e. those are computed at runtime
	// Software flags
	GRR_init = false;
	liftoff = false;
	poweredflight = false;
	HSL=false;								// High-Speed Loop flag
	BOOST=false;							// Boost To Orbit
	S4B_IGN=false;							// SIVB Ignition
	LVDC_GRR = false;
	tau1=0;									// Time to consume all fuel before S2 MRS
	Fm=0;									// sensed total accel
	Azimuth = 72.124;
	Inclination=0;							// Inclination
	DescNodeAngle=0;						// Descending Node Angle -- THETA_N
	Azo=0; Azs=0;							// Variables for scaling the -from-azimuth polynomials
	CommandedAttitude=_V(0,0,0);			// Commanded Attitude (RADIANS)
	PCommandedAttitude=_V(0,0,0);			// Previous Commanded Attitude (RADIANS)
	CurrentAttitude=_V(0,0,0);				// Current Attitude   (RADIANS)
	F=0;									// Force in Newtons, I assume.	
	K_Y=0; K_P=0; D_P=0; D_Y=0;				// Intermediate variables in IGM
	P_1=0; P_2=0; 
	L_1=0; L_2=0; L_P=0; L_Y=0; Lt_Y=0;
	J_1=0; J_2=0; J_Y=0; J_P=0; 
	S_1=0; S_2=0; S_P=0; S_Y=0; 
	U_1=0; U_2=0;  
	Q_1=0; Q_2=0; Q_Y=0; Q_P=0; 
	d2=0;
	f=0;									// True anomaly of the predicted cutoff radius vector
	e=0;									// Eccentricity of the transfer ellipse
	C_2=0; C_4=0;							// IGM coupling terms for pitch steering
	p=0;									// semilatus rectum of terminal ellipse
	K_1=0; K_2=0; K_3=0; K_4=0;				// Correction to chi-tilde steering angles, K_i
	K_5=0;									// IGM terminal velocity constant
	R=0;									// Instantaneous Radius Magnitude
	R_T=0;									// Desired terminal radius
	V=0;									// Instantaneous vehicle velocity
	V_T=0;									// Desired terminal velocity
	V_i=0; V_0=0; V_1=0; V_2=0;				// Parameters for cutoff velocity computation
	ups_T=0;								// Desired terminal flight-path angle
	MX_A=_M(0,0,0,0,0,0,0,0,0);				// Transform matrix from earth-centered plumbline to equatorial
	MX_B=_M(0,0,0,0,0,0,0,0,0);				// Transform matrix from equatorial to orbital coordinates
	MX_G=_M(0,0,0,0,0,0,0,0,0);				// Transform matrix from earth-centered plumbline to orbital
	MX_K=_M(0,0,0,0,0,0,0,0,0);				// Transform matrix from earth-centered plumbline to terminal
	MX_phi_T=_M(0,0,0,0,0,0,0,0,0);			// Matrix made from phi_T
	phi_T=0;								// Angle used to estimate location of terminal radius in orbital plane
	Pos4=_V(0,0,0);							// Position in the orbital reference system
	PosS=_V(0,0,0);							// Position in the earth-centered plumbline system. SPACE-FIXED.
	DotS=_V(0,0,0);							// VELOCITY in the earth-centered plumbline system
	DotM_act=_V(0,0,0);						// actual sensed velocity from platform
	ddotM_act =_V(0,0,0);
	ddotM_last =_V(0,0,0);
	ddotG_act=_V(0,0,0);					// actual computed acceleration from gravity
	DotG_act=_V(0,0,0);						// actual computed velocity from gravity
	DotM_last=_V(0,0,0);					// last sensed velocity from platform
	ddotG_last=_V(0,0,0);					// last computed acceleration from gravity
	DotG_last=_V(0,0,0);					// last computed velocity from gravity
	alpha_D=0;								// Angle from perigee to DN vector
	alpha_D_op=true;						// Option to determine alpha_D or load it
	G_T=0;									// Magnitude of desired terminal gravitational acceleration
	xi_T=0; eta_T=0; zeta_T=0;				// Desired position components in the terminal reference system
	PosXEZ=_V(0,0,0);						// Position components in the terminal reference system
	DotXEZ=_V(0,0,0);						// Instantaneous something
	deta=0; dxi=0;							// Position components to be gained in this axis
	T_T=0;									// Time-To-Go computed using T_2
	tchi_y=0; tchi_p=0;						// Angles to null velocity deficiencies without regard to terminal data
	dot_zeta_T=0; dot_xi_T=0; dot_eta_T=0;	// I don't know.
	ddot_zeta_GT=0; ddot_xi_GT=0;
	DDotXEZ_G=_V(0,0,0);					// ???
	ddot_xi_G=0; ddot_eta_G=0; ddot_zeta_G=0;								
	dot_dxit=0; dot_detat=0; dot_dzetat=0; 	// Intermediate velocity deficiency used in time-to-go computation
	dot_dxi=0; dot_deta=0; dot_dzeta=0; 	// More Deltas
	Xtt_y=0; Xtt_p=0; 						// IGM computed steering angles in terminal system
	X_S1=0; X_S2=0; X_S3=0; 				// Direction cosines of the thrust vector
	sin_ups=0; cos_ups=0;					// Sine and cosine of upsilon (flight-path angle)
	dot_phi_1=0; dot_phi_T=0; 				// ???
	dtt_1=0; dtt_2=0;						// Used in TGO determination
	dt = 1.7;								// Nominal powered-flight or coast-guidance computation-cycle interval
	a_1=0; a_2=0;							// Acceleration terms used to determine TGO
	T_GO=0;									// Time before S4B shutdown
	T_CO=0;									// Predicted time of S4B shutdown, from GRR
	dV=0;
	TAS=0;
	t_clock = 0;
	// TABLE15 and TABLE25 (see saturnv.h)
			TABLE15_f = 360;								// EPO
			TABLE15_e = 0;									// EPO
			TABLE15_C_3 = -60731521.02;						// EPO
//			TABLE15_f = 0.08050500;							// Apollo 11 (1st Opty, Constant)
//			TABLE15_e = 0.9762203;							// Apollo 11 (1st Opty, Index 0)
//			TABLE15_C_3 = -1437084;							// Apollo 11 (1st Opty, Index 0)
//			TABLE15_e = 0.9762098;							// Apollo 11 (1st Opty, Index 1)
//			TABLE15_C_3 = -1437508;							// Apollo 11 (1st Opty, Index 1)
//			TABLE15_e = 0.9761908;							// Apollo 11 (1st Opty, Index 2)
//			TABLE15_C_3 = -1438535;							// Apollo 11 (1st Opty, Index 2)
//			TABLE15_e = 0.9761679;							// Apollo 11 (1st Opty, Index 3)
//			TABLE15_C_3 = -1439902;							// Apollo 11 (1st Opty, Index 3)
//			TABLE15_e = 0.9761432;							// Apollo 11 (1st Opty, Index 4)
//			TABLE15_C_3 = -1441497;							// Apollo 11 (1st Opty, Index 4)


	// Set up remainder
	LVDC_Timebase = -1;										// Start up halted in pre-PTL wait
	LVDC_TB_ETime = 0;
	LVDC_Stop = false;
	IGMCycle = 0;
	sinceLastIGM = 0;
	// INTERNAL (NON-REAL-LVDC) FLAGS
	LVDC_EI_On = false;
	S1B_Sep_Time = 0;
	CountPIPA = false;
	if(!Initialized){ lvlog = fopen("lvlog1b.txt","w+"); } // Don't reopen the log if it's already open
	fprintf(lvlog,"init complete\r\n");
	fflush(lvlog);
	Initialized = true;
}
	
// DS20070205 LVDC++ EXECUTION
void LVDC1B::timestep(double simt, double simdt) {
	// Bail if uninitialized
	if(owner == NULL){ return; }
	// Update timebase ET
	LVDC_TB_ETime += simdt;
	
	// Note that GenericTimestep will update MissionTime.
	if(LVDC_Stop == false){
		/* **** LVDC GUIDANCE PROGRAM **** */
		switch(LVDC_Timebase){
			case -1: // LOOP WAITING FOR PTL
				// Lock time accel to 100x
				if (oapiGetTimeAcceleration() > 100){ oapiSetTimeAcceleration(100); } 

				// Prelaunch tank venting between -3:00h and engine ignition
				// No clue if the venting start time is correct
				if (owner->MissionTime < -10800){
					owner->DeactivatePrelaunchVenting();
				}else{
					owner->ActivatePrelaunchVenting();
				}

				// BEFORE PTL COMMAND (T-00:20:00) STOPS HERE
				{
					double Source  = fabs(owner->MissionTime);
					double Minutes = Source/60;
					double Hours   = (int)Minutes/60;				
					double Seconds = Source - ((int)Minutes*60);
					Minutes       -= Hours*60;
					if (owner->MissionTime < -1200){
						sprintf(oapiDebugString(),"LVDC: T - %d:%d:%.2f | AWAITING PTL INTERRUPT",(int)Hours,(int)Minutes,Seconds);
						lvimu.ZeroIMUCDUFlag = true;					// Zero IMU CDUs
						break;
					}else{
						sprintf(oapiDebugString(),"LVDC: T - %d:%d:%.2f | AWAITING GRR",(int)Hours,(int)Minutes,Seconds);
					}
				}
			
				// WAIT FOR GRR
				// Engine lights on at T-00:04:10
				if (owner->MissionTime >= -250 && LVDC_EI_On == false) { LVDC_EI_On = true; }

				// Between PTL signal and GRR, we monitor the IMU for any failure signals and do vehicle self-tests.
				// At GRR we transfer control to the flight program and start TB0.

				// BEFORE GRR (T-00:00:17) STOPS HERE
				if (owner->MissionTime >= -17){
					lvimu.ZeroIMUCDUFlag = false;					// Release IMU CDUs
					lvimu.DriveGimbals((Azimuth - 100)*RAD,0,0);	// Now bring to alignment 
					lvimu.SetCaged(false);							// Release IMU
					CountPIPA = true;								// Enable PIPA storage			
					BOOST = true;
					LVDC_GRR = true;								// Mark event
					poweredflight = true;
					oapiSetTimeAcceleration (1);					// Set time acceleration to 1
					owner->SetThrusterGroupLevel(owner->thg_main, 0);	// Ensure off
					{
						int i;
						for (i = 0; i < 5; i++) {					// Reconnect fuel to S1C engines						
							owner->SetThrusterResource(owner->th_main[i], owner->ph_1st);
						}
					}
					owner->CreateStageOne();						// Create hidden stage one, for later use in staging
					LVDC_Timebase = 0;								// Start TB0
					LVDC_TB_ETime = 0;
				}
				break;
			case 0: // MORE TB0
				double thrst[4];	// Thrust Settings for 2-2-2-2 start (see below)

				// At 10 seconds, play the countdown sound.
				if (owner->MissionTime >= -10.3) { // Was -10.9
					if (!owner->UseATC && owner->Scount.isValid()) {
						owner->Scount.play();
						owner->Scount.done();
					}
				}

				// Shut down venting at T - 9
				if(owner->MissionTime > -9 && owner->prelaunchvent[0] != NULL) { owner->DeactivatePrelaunchVenting(); }

				// Engine startup was staggered 2-2-2-2, with engine 7+5 starting first, then 6+8, then 2+4, then 3+1
			
				// Engine 7+5 combustion chamber ignition was at T-2.998,  6+8 at T-2.898, 2+4 at T-2.798, 1+3 at T-2.698
				// The engines idled in low-range thrust (about 2.5% thrust) for about 0.3 seconds
				// and then rose to 93% thrust in 0.085 seconds.
				// The rise from 93 to 100 percent thrust took 0.75 second.
				// Total engine startup time was 1.9 seconds.

				// Source: Apollo 7 LV Flight Evaluation

				// Transition from seperate throttles to single throttle
				if(owner->MissionTime < -0.715){ 
					int x=0; // Start Sequence Index
					double tm_1,tm_2,tm_3,tm_4; // CC light, 1st rise start, and 2nd rise start, and 100% thrust times.
					double SumThrust=0;

					while(x < 4){
						thrst[x] = 0;
						switch(x){
							case 0: // Engine 7+5
								tm_1 = -2.998; break;
							case 1: // Engine 6+8
								tm_1 = -2.898; break;
							case 2: // Engine 2+4
								tm_1 = -2.798; break;
							case 3: // Engine 1+3
								tm_1 = -2.698; break;
						}
						tm_2 = tm_1 + 0.3;  // Start of 1st rise
						tm_3 = tm_2 + 0.085; // Start of 2nd rise
						tm_4 = tm_3 + 0.75; // End of 2nd rise
						if(owner->MissionTime >= tm_1){
							// Light CC
							if(owner->MissionTime < tm_2){
								// Idle at 2.5% thrust
								thrst[x] = 0.025;
							}else{
								if(owner->MissionTime < tm_3){
									// the actual rise is so fast that any 'smoothing' is pointless
									thrst[x] = 0.93;
								}else{
									if(owner->MissionTime < tm_4){
										// Rise to 100% at a rate of 9 percent per second.
										thrst[x] = 0.93+(0.09*(owner->MissionTime-tm_3));
									}else{
										// Hold 100%
										thrst[x] = 1;
									}
								}
							}
						}
						x++; // Do next
					}
					SumThrust = (thrst[0]*2)+(thrst[1]*2)+(thrst[2]*2)+(thrst[3]*2);
	//				sprintf(oapiDebugString(),"LVDC: T %f | TB0 + %f | TH 0/1/2 = %f %f %f Sum %f",
	//					MissionTime,LVDC_TB_ETime,thrst[0],thrst[1],thrst[2],SumThrust);
					if(SumThrust > 0){ //let's hope that those numberings are right...
						owner->SetThrusterLevel(owner->th_main[0],thrst[3]); // Engine 1
						owner->SetThrusterLevel(owner->th_main[1],thrst[2]); // Engine 2
						owner->SetThrusterLevel(owner->th_main[2],thrst[3]); // Engine 3
						owner->SetThrusterLevel(owner->th_main[3],thrst[2]); // Engine 4
						owner->SetThrusterLevel(owner->th_main[4],thrst[0]); // Engine 5
						owner->SetThrusterLevel(owner->th_main[5],thrst[1]); // Engine 6
						owner->SetThrusterLevel(owner->th_main[6],thrst[0]); // Engine 7
						owner->SetThrusterLevel(owner->th_main[7],thrst[1]); // Engine 8

						owner->contrailLevel = SumThrust/8;
						owner->AddForce(_V(0, 0, -10. * owner->THRUST_FIRST_VAC), _V(0, 0, 0)); // Maintain hold-down lock
					}
				}else{
					// Get 100% thrust on all engines.
					sprintf(oapiDebugString(),"LVDC: T %f | TB0 + %f | TH = 100%",owner->MissionTime,LVDC_TB_ETime);
					owner->SetThrusterGroupLevel(owner->thg_main,1);
					owner->contrailLevel = 1;				
					owner->AddForce(_V(0, 0, -10. * owner->THRUST_FIRST_VAC), _V(0, 0, 0));
				}

				if(owner->MissionTime >= 0){
					LVDC_Timebase = 1;
					LVDC_TB_ETime = 0;	
				}
				break;

			case 1: // LIFTOFF TIME
				if(liftoff == false){
					liftoff = true;
					owner->SetLiftoffLight();										// And light liftoff lamp
					owner->SetStage(LAUNCH_STAGE_ONE);								// Switch to stage one
					// Start mission and event timers
					owner->MissionTimerDisplay.Reset();
					owner->MissionTimerDisplay.SetEnabled(true);
					owner->EventTimerDisplay.Reset();
					owner->EventTimerDisplay.SetEnabled(true);
					owner->EventTimerDisplay.SetRunning(true);
					owner->agc.SetInputChannelBit(030, 5, true);					// Inform AGC of liftoff
					owner->SetThrusterGroupLevel(owner->thg_main, 1.0);				// Set full thrust, just in case
					owner->contrailLevel = 1.0;
					if (owner->LaunchS.isValid() && !owner->LaunchS.isPlaying()){	// And play launch sound
						owner->LaunchS.play(NOLOOP,255);
						owner->LaunchS.done();
					}			
					sinceLastIGM = 1.7-simdt; // Rig to pass on fall-in
				}

				// Soft-Release Pin Dragging
				if(owner->MissionTime < 0.5){
					double PinDragFactor = 1 - (owner->MissionTime*2);
					owner->AddForce(_V(0, 0, -(owner->THRUST_FIRST_VAC * PinDragFactor)), _V(0, 0, 0));
				}

				// Below here are timed events that must not be dependent on the iteration delay.

				/*
				// ENGINE FAIL TEST:
				if(MissionTime > 22.5 && S1C_Engine_Out == false){
					SetThrusterResource(th_main[1], NULL); // Should stop the engine
					S1C_Engine_Out = true;
				}
				*/

				// S1B CECO TRIGGER:
				if(owner->MissionTime > 140.86){ // Apollo 7
					owner->SetThrusterResource(owner->th_main[4], NULL);
					owner->SetThrusterResource(owner->th_main[5], NULL);
					owner->SetThrusterResource(owner->th_main[6], NULL);
					owner->SetThrusterResource(owner->th_main[7], NULL);
					owner->SShutS.play(NOLOOP,235);
					owner->SShutS.done();
					// Clear liftoff light now - Apollo 15 checklist item
					owner->ClearLiftoffLight();
					S1B_Engine_Out = true;
					// Begin timebase 2
					LVDC_Timebase = 2;
					LVDC_TB_ETime = 0;
				}
				break;

			case 2:
				// S1B OECO TRIGGER
				// Done by low-level sensor.
				if (owner->stage == LAUNCH_STAGE_ONE && owner->GetFuelMass() <= 0){
					// For S1C thruster calibration
					fprintf(lvlog,"[T+%f] S1C OECO - Thrust %f N @ Alt %f\r\n\r\n",
						owner->MissionTime,owner->GetThrusterMax(owner->th_main[0]),owner->GetAltitude());

					// Move hidden S1B
					if (owner->hstg1) {
						VESSELSTATUS vs;
						owner->GetStatus(vs);
						S1B *stage1 = (S1B *) oapiGetVesselInterface(owner->hstg1);
						stage1->DefSetState(&vs);
					}				
					// Set timer
					S1B_Sep_Time = owner->MissionTime;
					// Engine Shutdown
					int i;
					for (i = 0; i < 5; i++){
						owner->SetThrusterResource(owner->th_main[i], NULL);
					}
					// Begin timebase 3
					LVDC_Timebase = 3;
					LVDC_TB_ETime = 0;
				}
				break;

			case 3:
				// S1B SEPARATION TRIGGER
				if(owner->stage == LAUNCH_STAGE_ONE && LVDC_TB_ETime >= 0.5){
					// Drop old stage
					owner->ClearEngineIndicators();
					owner->SeparateStage(LAUNCH_STAGE_SIVB);
					owner->SetStage(LAUNCH_STAGE_SIVB);
					owner->AddRCS_S4B();
					owner->SetSIVBThrusters(true);
					owner->SetThrusterGroupLevel(owner->thg_ver,1.0);
					owner->SetThrusterResource(owner->th_main[0], owner->ph_3rd);
					owner->SetSIVBMixtureRatio(5.5);				
				}
						
				if(LVDC_TB_ETime >= 2 && LVDC_TB_ETime < 6.8 && owner->stage == LAUNCH_STAGE_SIVB){
					owner->SetThrusterGroupLevel(owner->thg_main, ((LVDC_TB_ETime-4)*0.36));
					if(LVDC_TB_ETime >= 5){ owner->SetThrusterGroupLevel(owner->thg_ver,0); }
				}
				if(LVDC_TB_ETime >= 8.6 && S4B_IGN == false && owner->stage == LAUNCH_STAGE_SIVB){
					owner->SetThrusterGroupLevel(owner->thg_main, 1.0);
					S4B_IGN=true;
				}
				if(LVDC_TB_ETime > 20 && owner->LESAttached){
					T_LET = LVDC_TB_ETime;	// Update this. If the LET jettison never happens, the placeholder value
											// will start IGM anyway.
					owner->JettisonLET();				
				}
				if(LVDC_TB_ETime > 311.5 && MRS == false){
					// MR Shift
					fprintf(lvlog,"[TB%d+%f] MR Shift\r\n",LVDC_Timebase,LVDC_TB_ETime);
					// sprintf(oapiDebugString(),"LVDC: EMR SHIFT"); LVDC_GP_PC = 30; break;
					owner->SetSIVBMixtureRatio (4.5); // Is this 4.7 or 4.2? AP8 says 4.5
					owner->SPUShiftS.play(NOLOOP,255); 
					owner->SPUShiftS.done();
					MRS = true;
				}
				break;

			case 4:
				if (LVDC_TB_ETime >= 10 && LVDC_EI_On == true){
					owner->SetStage(STAGE_ORBIT_SIVB);
					fprintf(lvlog,"[TB%d+%f] Set STAGE_ORBIT_SIVB\r\n",LVDC_Timebase,LVDC_TB_ETime);
					LVDC_EI_On = false;
				}
				if(LVDC_TB_ETime > 100){
					poweredflight = false; //powered flight nav off
				}
				// Orbital stage timed events
				if(owner->stage != STAGE_ORBIT_SIVB){ break; } // Stop here until enabled			
				// Venting			
				if (LVDC_TB_ETime >= 5773) {				
					if (owner->GetThrusterLevel(owner->th_main[0]) > 0) {
						owner->SetJ2ThrustLevel(0);
						if (owner->Realism){ owner->EnableDisableJ2(false); }
					}
				}else{
					if (LVDC_TB_ETime >= 5052) {					
						if (owner->GetThrusterLevel(owner->th_main[0]) == 0) {
							owner->EnableDisableJ2(true);
							owner->SetJ2ThrustLevel(1);
						}
					}
				}
				// Fuel boiloff every ten seconds.
				if (owner->MissionTime >= owner->NextMissionEventTime){
					if (owner->GetThrusterLevel(owner->th_main[0]) < 0.5){
						owner->SIVBBoiloff();					
					}
					owner->NextMissionEventTime = owner->MissionTime+10.0;				
				}
				// CSM/LV separation
				if(owner->CSMLVPyros.Blown()){
					owner->SeparateStage(CSM_LEM_STAGE);
					owner->SetStage(CSM_LEM_STAGE);
				}
				break;
		}
		lvimu.Timestep(simt);								// Give a timestep to the LV IMU
		lvrg.Timestep(simdt);								// and RG
		CurrentAttitude = lvimu.GetTotalAttitude();			// Get current attitude	
		AttRate = lvrg.GetRates();							// Get rates	
		//This is the actual LVDC code & logic; has to be independent from any of the above events
		if(LVDC_GRR && GRR_init == false){			
			fprintf(lvlog,"[T%f] GRR received!\r\n",owner->MissionTime);

			// Initial Position & Velocity
			MATRIX3 rot;
			owner->GetRelativePos(oapiGetGbodyByName ("Earth"), PosS);
			owner->GetRelativeVel(oapiGetGbodyByName ("Earth"), Dot0);
			
			oapiGetPlanetObliquityMatrix(oapiGetGbodyByName("Earth"),&rot);
			PosS = tmul(rot,PosS);
			Dot0 = tmul(rot,Dot0);
			fprintf(lvlog,"EarthRel Position: %f %f %f \r\n",PosS.x,PosS.y,PosS.z);
			fprintf(lvlog,"EarthRel Velocity: %f %f %f \r\n",Dot0.x,Dot0.y,Dot0.z);
			double rad      = sqrt  (PosS.x*PosS.x + PosS.y*PosS.y + PosS.z*PosS.z);
			phi_lng    = atan2 (PosS.z, PosS.x);
			phi_lat    = asin  (PosS.y/rad);
			cos_phi_L = cos(phi_lat);
			sin_phi_L = sin(phi_lat);
			fprintf(lvlog, "Latitude = %f, Longitude = %f\r\n", phi_lat*DEG, phi_lng*DEG);
			fprintf(lvlog, "cos_phi_l = %f, sin_phi_l = %f\r\n", cos_phi_L, sin_phi_L);
			rot.m11 = cos(phi_lng); rot.m12 = 0; rot.m13 = sin(phi_lng);
			rot.m21 = 0; rot.m22 = 1; rot.m23 = 0;
			rot.m31 = -sin(phi_lng); rot.m32 = 0; rot.m33 = cos(phi_lng);
			PosS = mul(rot,PosS);
			Dot0 = mul(rot,Dot0);
			fprintf(lvlog,"Rot:longitude\r\n");
			fprintf(lvlog,"EarthRel Position: %f %f %f \r\n",PosS.x,PosS.y,PosS.z);
			fprintf(lvlog,"EarthRel Velocity: %f %f %f \r\n",Dot0.x,Dot0.y,Dot0.z);
			rot.m11 = cos(-phi_lat); rot.m12 = -sin(-phi_lat); rot.m13 = 0;
			rot.m21 = sin(-phi_lat); rot.m22 = cos(-phi_lat); rot.m23 = 0;
			rot.m31 = 0; rot.m32 = 0; rot.m33 = 1;
			PosS = mul(rot,PosS);
			Dot0 = mul(rot,Dot0);
			fprintf(lvlog,"Rot:latitude\r\n");
			fprintf(lvlog,"EarthRel Position: %f %f %f \r\n",PosS.x,PosS.y,PosS.z);
			fprintf(lvlog,"EarthRel Velocity: %f %f %f \r\n",Dot0.x,Dot0.y,Dot0.z);
			
			// Time into launch window = launch time from midnight - reference time of launch from midnight
			// azimuth = coeff. of azimuth polynomial * time into launch window

			// preset to fixed value to be independent from any external stuff
			Azimuth = 72.124;
			fprintf(lvlog,"Azimuth = %f\r\n",Azimuth);
			rot.m11 = 1; rot.m12 = 0; rot.m13 = 0;
			rot.m21 = 0; rot.m22 = cos((90-Azimuth)*RAD); rot.m23 = -sin((90-Azimuth)*RAD);
			rot.m31 = 0; rot.m32 = sin((90-Azimuth)*RAD); rot.m33 = cos((90-Azimuth)*RAD);
			PosS = mul(rot,PosS);
			Dot0 = mul(rot,Dot0);
			fprintf(lvlog,"Rot:azimuth\r\n");
			fprintf(lvlog,"EarthRel Position: %f %f %f \r\n",PosS.x,PosS.y,PosS.z);
			fprintf(lvlog,"EarthRel Velocity: %f %f %f \r\n",Dot0.x,Dot0.y,Dot0.z);
			PosS.y = -PosS.y;
			Dot0.y = -Dot0.y;
			// Azo and Azs are used to scale the polys below. These numbers are from Apollo 11.
			// Dunno if this actually works. The numbers are in "PIRADS", whatever that is.
			Azo = 4; 
			Azs = 2;

			if(i_op == true){
				// CALCULATE INCLINATION FROM AZIMUTH
				Inclination = 0;
				int x=0;
				while(x < 7){
					Inclination += fx[x] * pow((Azimuth-Azo)/Azs,x);
					x++;
				}
			}else{
				// CALCULATE INCLINATION FROM TIME INTO LAUNCH WINDOW
				// inclination = coeff. for inclination-from-time polynomial * Time into launch window
			}
			// Let's cheat a little. (Apollo 7)
			Inclination = 31.605;
			fprintf(lvlog,"Inclination = %f\r\n",Inclination);

			if(theta_N_op == true){
				// CALCULATE DESCENDING NODAL ANGLE FROM AZIMUTH
				DescNodeAngle = 0;
				int x=0;
				while(x < 7){
					DescNodeAngle += gx[x] * pow((Azimuth-Azo)/Azs,x);
					x++;
				}
			}else{
				// CALCULATE DESCENDING NODAL ANGLE FROM TIME INTO LAUNCH WINDOW
				// DNA = coeff. for DNA-from-time polynomial * Time into launch window
			}
			
			// Cheat a little more. (Apollo 7)
			DescNodeAngle = 118.912; 
			fprintf(lvlog,"DescNodeAngle = %f\r\n",DescNodeAngle);

			// Need to make those into radians
			Azimuth *= RAD;
			Inclination *= RAD;
			DescNodeAngle *= RAD;

			fprintf(lvlog,"Rad Convert: Az / Inc / DNA = %f %f %f\r\n",Azimuth,Inclination,DescNodeAngle);

			// p is the semi-latus rectum of the desired terminal ellipse.
			p = (mu/C_3)*(pow(e,2)-1);
			fprintf(lvlog,"p = %f, mu = %f, e2 = %f, mu/C_3 = %f\r\n",p,mu,pow(e,2),mu/C_3);

			// K_5 is the IGM terminal velocity constant
			K_5 = sqrt(mu/p);
			fprintf(lvlog,"K_5 = %f\r\n",K_5);

			R_T = p/(1+(e*(cos(f))));
			V_T = K_5*sqrt((1+((2*e)*(cos(f)))+pow(e,2)));
			ups_T = atan((e*(sin(f)))/(1+(e*(cos(f)))));
			G_T = -mu/pow(R_T,2);
			fprintf(lvlog,"R_T = %f (Expecting 6,563,366), V_T = %f (Expecting 7793.0429), ups_T = %f\r\n",R_T,V_T,ups_T);

			// G MATRIX CALCULATION
			MX_A.m11 = cos_phi_L;  MX_A.m12 = sin_phi_L*sin(Azimuth); MX_A.m13 = -(sin_phi_L*cos(Azimuth));
			MX_A.m21 = -sin_phi_L; MX_A.m22 = cos_phi_L*sin(Azimuth); MX_A.m23 = -(cos_phi_L*cos(Azimuth));
			MX_A.m31 = 0;  MX_A.m32 = cos(Azimuth);  MX_A.m33 = sin(Azimuth);

			MX_B.m11 = cos(DescNodeAngle); MX_B.m12 = 0; MX_B.m13 = sin(DescNodeAngle);
			MX_B.m21 = sin(DescNodeAngle)*sin(Inclination); MX_B.m22 = cos(Inclination); 
			MX_B.m23 = -cos(DescNodeAngle)*sin(Inclination);
			MX_B.m31 = -sin(DescNodeAngle)*cos(Inclination); MX_B.m32 = sin(Inclination);
			MX_B.m33 = cos(DescNodeAngle)*cos(Inclination);

			MX_G = mul(MX_B,MX_A); // Matrix Multiply
			Y_u= -(PosS.x*MX_A.m21+PosS.y*MX_A.m22+PosS.z*MX_A.m23); //position component south of equator
			R = pow(pow(PosS.x,2)+pow(PosS.y,2)+pow(PosS.z,2),0.5);  //instantaneous distance from earth's center
			S = (-mu/pow(R,3))*(1+J*pow(a/R,2)*(1-5*pow(Y_u/R,2)));
			P = (mu/pow(R,2))*pow(a/R,2) *((2*J*Y_u)/R);
			ddotG_last.x = PosS.x*S+MX_A.m21*P; //gravity acceleration vector
			ddotG_last.y = PosS.y*S+MX_A.m22*P;
			ddotG_last.z = PosS.z*S+MX_A.m23*P;
			PCommandedAttitude.x = (1.5* PI) + Azimuth;
			PCommandedAttitude.y = 0;
			PCommandedAttitude.z = 0;
			lvimu.ZeroPIPACounters();
			sinceLastIGM = 0;
			GRR_init = true;
			fprintf(lvlog,"Initialization completed.\r\n\r\n",owner->MissionTime);
			goto minorloop;
		}
		// various clocks the LVDC needs...
		if(LVDC_GRR == true){ TAS += simdt; } //time since GRR
		if(liftoff == true){ t_clock += simdt; } //time since liftoff
		sinceLastIGM += simdt;
		if(sinceLastIGM < 1.7){ goto minorloop; }
		dt_c = sinceLastIGM;
		IGMInterval = sinceLastIGM;
		sinceLastIGM = 0;
		IGMCycle++;				// For debugging
		fprintf(lvlog,"[%d+%f] *** Major Loop %d ***\r\n",LVDC_Timebase,LVDC_TB_ETime,IGMCycle);
		//powered flight nav
		if(LVDC_GRR == true){
			if(poweredflight == true){
				DotM_act.x += (lvimu.CDURegisters[LVRegPIPAX]);//read the PIPA CDUs
				DotM_act.y += (lvimu.CDURegisters[LVRegPIPAY]);
				DotM_act.z += (lvimu.CDURegisters[LVRegPIPAZ]);
			}
			Fm = pow((pow(((DotM_act.x - DotM_last.x)/dt_c),2)+ pow(((DotM_act.y - DotM_last.y)/dt_c),2)+ pow(((DotM_act.z - DotM_last.z)/dt_c),2)),0.5);
			PosS.x += (DotM_act.x + DotM_last.x) * dt_c / 2 + (DotG_last.x + ddotG_last.x * dt_c / 2)*dt_c + Dot0.x * dt_c;//position vector
			PosS.y += (DotM_act.y + DotM_last.y) * dt_c / 2 + (DotG_last.y + ddotG_last.y * dt_c / 2)*dt_c + Dot0.y * dt_c;
			PosS.z += (DotM_act.z + DotM_last.z) * dt_c / 2 + (DotG_last.z + ddotG_last.z * dt_c / 2)*dt_c + Dot0.z * dt_c;
			Y_u= -(PosS.x*MX_A.m21+PosS.y*MX_A.m22+PosS.z*MX_A.m23);//position component south of equator
			R = pow(pow(PosS.x,2)+pow(PosS.y,2)+pow(PosS.z,2),0.5);//instantaneous distance from earth's center
			S = (-mu/pow(R,3))*(1+J*pow(a/R,2)*(1-5*pow(Y_u/R,2)));
			P = (mu/pow(R,2))*pow(a/R,2) *((2*J*Y_u)/R);
			ddotG_act.x = PosS.x*S+MX_A.m21*P;//gravity acceleration vector
			ddotG_act.y = PosS.y*S+MX_A.m22*P;
			ddotG_act.z = PosS.z*S+MX_A.m23*P;
			CG = pow((pow(ddotG_act.x,2)+ pow(ddotG_act.y,2)+ pow(ddotG_act.z,2)),0.5);
			DotG_act.x = DotG_last.x + (ddotG_act.x  + ddotG_last.x) * dt_c / 2;//gravity velocity vector
			DotG_act.y = DotG_last.y + (ddotG_act.y  + ddotG_last.y) * dt_c / 2;
			DotG_act.z = DotG_last.z + (ddotG_act.z  + ddotG_last.z) * dt_c / 2;
			DotS.x = DotM_act.x + DotG_act.x + Dot0.x;//total velocity vector 
			DotS.y = DotM_act.y + DotG_act.y + Dot0.y;
			DotS.z = DotM_act.z + DotG_act.z + Dot0.z;
			V = pow(pow(DotS.x,2)+pow(DotS.y,2)+pow(DotS.z,2),0.5);
			//save the 'actual' variables as 'last' variables for the next step
			DotM_last = DotM_act;
			DotG_last = DotG_act;
			ddotG_last = ddotG_act;
			fprintf(lvlog,"Navigation \r\n");
			fprintf(lvlog,"Inertial Attitude: %f %f %f \r\n",CurrentAttitude.x*DEG,CurrentAttitude.y*DEG,CurrentAttitude.z*DEG);
			fprintf(lvlog,"DotM: %f %f %f \r\n", DotM_act.x,DotM_act.y,DotM_act.z);
			fprintf(lvlog,"Gravity velocity: %f %f %f \r\n", DotG_act.x,DotG_act.y,DotG_act.z);
			fprintf(lvlog,"EarthRel Position: %f %f %f \r\n",PosS.x,PosS.y,PosS.z);
			fprintf(lvlog,"EarthRel Velocity: %f %f %f \r\n",DotS.x,DotS.y,DotS.z);
			fprintf(lvlog,"Sensed Acceleration: %f \r\n",Fm);	
			fprintf(lvlog,"Gravity Acceleration: %f \r\n",CG);	
			fprintf(lvlog,"Total Velocity: %f \r\n",V);
			fprintf(lvlog,"Dist. from Earth's Center: %f \r\n",R);
			fprintf(lvlog,"S: %f \r\n",S);
			fprintf(lvlog,"P: %f \r\n",P);
			lvimu.ZeroPIPACounters();
		}
		if(liftoff == false){//liftoff not received; initial roll command for FCC
			CommandedAttitude.x =  (360-100)*RAD + Azimuth;
			CommandedAttitude.y =  0;
			CommandedAttitude.z =  0;
			fprintf(lvlog,"[%d+%f] Initial roll command: %f\r\n",LVDC_Timebase,LVDC_TB_ETime,CommandedAttitude.x*DEG);
			goto minorloop;
		}
		if(BOOST == false){//i.e. we're either in orbit or boosting out of orbit
			if(LVDC_Timebase == 4 && (LVDC_TB_ETime > 20)){
				goto orbitalguidance;
			}else{
				goto minorloop;
			}
		} //TBD: 
		if(LVDC_Timebase < 3 || (LVDC_TB_ETime - T_LET < 0)){ 
			//Pre-IGM guidance:
			if(S1B_Engine_Out == true && T_EO1 == 0){
				//S1B engine out interrupt handling
				T_EO1 = 1;
				t_fail = t_clock;
				fprintf(lvlog,"[%d+%f] S1B engine out interrupt received! t_fail = %f\r\n",LVDC_Timebase,LVDC_TB_ETime,t_fail);
			}
			if(t_clock > t_1){
				//roll/pitch program
				if(t_clock >= t_2 && T_EO1 > 0){
					//time to re-calculate freeze time?
					T_EO1 = -1; //allow only one pass thru re-calculation
					if(t_fail <= t_2){ dT_F = t_3; }
					if(t_2 < t_clock && t_clock <= t_4){ dT_F = (B_11 * t_fail) + B_12; }
					if(t_4 < t_clock && t_clock <= t_5){ dT_F = (B_21 * t_fail) + B_22; }
					if(t_5 < t_clock){ dT_F = 0; }
					t_6 = t_clock + dT_F;
					T_ar = T_ar + (0.25*(T_ar - t_fail));
					fprintf(lvlog,"[%d+%f] Freeze time recalculated! t_6 = %f T_ar = %f\r\n",LVDC_Timebase,LVDC_TB_ETime,t_6,T_ar);
				}
				if(t_clock >= t_6){
					if (t_clock > T_ar){
						//time for pitch freeze?
						fprintf(lvlog,"[%d+%f] Pitch freeze! \r\n",LVDC_Timebase,LVDC_TB_ETime,T_ar);
						CommandedAttitude.y = PCommandedAttitude.y;
						CommandedAttitude.x = 360 * RAD;
						CommandedAttitude.z = 0;
						goto limittest;
					}else{
						//Pre-IGM steering
						int x=0,y=0;						
						double Tc = t_clock - dT_F, cmd = 0;  
						if(Tc < T_S1){               x = 1; }
						if(T_S1 <= Tc && Tc < T_S2){ x = 2; }
						if(T_S2 <= Tc && Tc < T_S3){ x = 3; }
						if(T_S3 <= Tc){              x = 4; }
						while(y <= 4){
							cmd += (Fx[x][y] * ((double)pow(Tc,y)));
							y++;
						}
						CommandedAttitude.y = cmd * RAD;
						CommandedAttitude.x = 360 * RAD;
						CommandedAttitude.z = 0;
						fprintf(lvlog,"[%d+%f] Roll/pitch programm %f \r\n",LVDC_Timebase,LVDC_TB_ETime,cmd);
						goto limittest;
					}
				}else{
					CommandedAttitude.y = PCommandedAttitude.y;
					CommandedAttitude.x = 360 * RAD;
					CommandedAttitude.z = 0;
					goto limittest;
				}
			}else{
				CommandedAttitude.z = 0;//no yaw
				CommandedAttitude.y = 0;//no pitch
				CommandedAttitude.x = (360-100)*RAD + Azimuth;;//no roll
				goto limittest;
			}
			//end of pre igm
		}
		if(HSL == false){		
			// If we are not in the high-speed loop
			fprintf(lvlog,"HSL False\r\n");
			// IGM STAGE LOGIC
			if(MRS == true){
				fprintf(lvlog,"Post-MRS\n");
				if(t_B1 <= t_B3){
					tau2 = V_ex2/Fm;
					fprintf(lvlog,"Normal Tau: tau2 = %f, F/m = %f, m = %f \r\n",tau2,Fm,owner->GetMass());
				}else{
					// This is the "ARTIFICIAL TAU" code.
					t_B3 += dt_c; 
					tau2 = tau2+(T_1*(dotM_1/dotM_2));
					fprintf(lvlog,"Art. Tau: tau2 = %f, T_1 = %f, dotM_1 = %f dotM_2 = %f \r\n",tau2,T_1,dotM_1,dotM_2);
					fprintf(lvlog,"Diff: %f \r\n",(tau2-(V_ex2/Fm)));
					T_2 = T_2+T_1*(dotM_1/dotM_2);
					T_1 = 0;
					fprintf(lvlog,"T_1 = 0\r\nT_2 = %f, dotM_1 = %f, dotM_2 = %f \r\n",T_2,dotM_1,dotM_2);
				}					
			}else{
				fprintf(lvlog,"Pre-MRS\n");
				if(T_1 < 0){	// If we're out of first-stage IGM time
					// Artificial Tau
					tau2 = tau2+(T_1*(dotM_1/dotM_2));
					fprintf(lvlog,"Art. Tau: tau2 = %f, T_1 = %f, dotM_1 = %f, dotM_2 = %f \r\n",tau2,T_1,dotM_1,dotM_2);
					T_2 = T_2+T_1*(dotM_1/dotM_2);
					T_1 = 0;
					fprintf(lvlog,"T_2 = %f, T_1 = %f, dotM_1 = %f, dotM_2 = %f \r\n",T_2,T_1,dotM_1,dotM_2);		
				}else{															
					tau1 = V_ex1/Fm; 
					fprintf(lvlog,"Normal Tau: tau1 = %f, F/m = %f m = %f\r\n",tau1,Fm, owner->GetMass());
				}
			}
			fprintf(lvlog,"--- STAGE INTEGRAL LOGIC ---\r\n");
			// CHI-TILDE LOGIC
			// STAGE INTEGRAL CALCULATIONS				
			Pos4 = mul(MX_G,PosS);
			fprintf(lvlog,"Pos4 = %f, %f, %f\r\n",Pos4.x,Pos4.y,Pos4.z);
			fprintf(lvlog,"T_1 = %f,T_2 = %f\r\n",T_1,T_2);
			L_1 = V_ex1 * log(tau1 / (tau1-T_1));
			J_1 = (L_1 * tau1) - (V_ex1 * T_1);
			S_1 = (L_1 * T_1) - J_1;
			Q_1 = (S_1 * tau1) - ((V_ex1 * pow(T_1,2)) / 2);
			P_1 = (J_1 * tau1) - ((V_ex1 * pow(T_1,2)) / 2);
			U_1 = (Q_1 * tau1) - ((V_ex1 * pow(T_1,3)) / 6);
			fprintf(lvlog,"L_1 = %f, J_1 = %f, S_1 = %f, Q_1 = %f, P_1 = %f, U_1 = %f\r\n",L_1,J_1,S_1,Q_1,P_1,U_1);

			Lt_2 = V_ex2 * log(tau2 / (tau2-Tt_2));
			fprintf(lvlog,"Lt_2 = %f, tau2 = %f, Tt_2 = %f\r\n",Lt_2,tau2,Tt_2);

			Jt_2 = (Lt_2 * tau2) - (V_ex2 * Tt_2);
			fprintf(lvlog,"Jt_2 = %f",Jt_2);
			Lt_Y = (L_1 + Lt_2);
			fprintf(lvlog,", Lt_Y = %f\r\n",Lt_Y);

			// SELECT RANGE OPTION				
gtupdate:	// Target of jump from further down
			fprintf(lvlog,"--- GT UPDATE ---\r\n");

			// RANGE ANGLE 1
			fprintf(lvlog,"RANGE ANGLE\r\n");
			d2 = (V * Tt_T) - Jt_2 + (Lt_Y * Tt_2) - (ROV / V_ex2) * 
				((tau1 - T_1) * L_1 + (tau2 - Tt_2) * Lt_2) *
				(Lt_Y + V - V_T);
			phi_T = ((atan2(Pos4.z,Pos4.x))+(((1/R_T)*(S_1+d2))*(cos(ups_T))));
			fprintf(lvlog,"V = %f, d2 = %f, phi_T = %f\r\n",V,d2,phi_T);				

			// FREEZE TERMINAL CONDITIONS TEST
			if(!(Tt_T <= eps_3)){
				// UPDATE TERMINAL CONDITIONS
				fprintf(lvlog,"UPDATE TERMINAL CONDITIONS\r\n");
				f = phi_T + alpha_D;
				R_T = p/(1+((e*(cos(f)))));
				fprintf(lvlog,"f = %f, R_T = %f\r\n",f,R_T);
				V_T = K_5 * pow(1+((2*e)*(cos(f)))+pow(e,2),0.5);
				ups_T = atan((e*(sin(f)))/(1+(e*(cos(f)))));
				G_T = -mu/pow(R_T,2);
				fprintf(lvlog,"V_T = %f, ups_T = %f, G_T = %f\r\n",V_T,ups_T,G_T);
			}

			// UNROTATED TERMINAL CONDITIONS
			fprintf(lvlog,"UNROTATED TERMINAL CONDITIONS\r\n");
			xi_T = R_T;					
			dot_zeta_T = V_T * (cos(ups_T));
			dot_xi_T = V_T * (sin(ups_T));
			ddot_zeta_GT = 0;
			ddot_xi_GT = G_T;
			fprintf(lvlog,"xi_T = %f, dot_zeta_T = %f, dot_xi_T = %f\r\n",xi_T,dot_zeta_T,dot_xi_T);
			fprintf(lvlog,"ddot_zeta_GT = %f, ddot_xi_GT = %f\r\n",ddot_zeta_GT,ddot_xi_GT);
				
			// ROTATION TO TERMINAL COORDINATES
			fprintf(lvlog,"--- ROTATION TO TERMINAL COORDINATES ---\r\n");
			// This is the last time PosS is referred to.
			MX_phi_T.m11 = (cos(phi_T));    MX_phi_T.m12 = 0; MX_phi_T.m13 = ((sin(phi_T)));
			MX_phi_T.m21 = 0;               MX_phi_T.m22 = 1; MX_phi_T.m23 = 0;
			MX_phi_T.m31 = (-sin(phi_T)); MX_phi_T.m32 = 0; MX_phi_T.m33 = (cos(phi_T));
			fprintf(lvlog,"MX_phi_T R1 = %f %f %f\r\n",MX_phi_T.m11,MX_phi_T.m12,MX_phi_T.m13);
			fprintf(lvlog,"MX_phi_T R2 = %f %f %f\r\n",MX_phi_T.m21,MX_phi_T.m22,MX_phi_T.m23);
			fprintf(lvlog,"MX_phi_T R3 = %f %f %f\r\n",MX_phi_T.m31,MX_phi_T.m32,MX_phi_T.m33);

			MX_K = mul(MX_phi_T,MX_G);
			fprintf(lvlog,"MX_K R1 = %f %f %f\r\n",MX_K.m11,MX_K.m12,MX_K.m13);
			fprintf(lvlog,"MX_K R2 = %f %f %f\r\n",MX_K.m21,MX_K.m22,MX_K.m23);
			fprintf(lvlog,"MX_K R3 = %f %f %f\r\n",MX_K.m31,MX_K.m32,MX_K.m33);

			PosXEZ = mul(MX_K,PosS);
			DotXEZ = mul(MX_K,DotS);	
			fprintf(lvlog,"PosXEZ = %f %f %f\r\n",PosXEZ.x,PosXEZ.y,PosXEZ.z);
			fprintf(lvlog,"DotXEZ = %f %f %f\r\n",DotXEZ.x,DotXEZ.y,DotXEZ.z);

			VECTOR3 RTT_T1,RTT_T2;
			RTT_T1.x = ddot_xi_GT; RTT_T1.y = 0;        RTT_T1.z = ddot_zeta_GT;
			RTT_T2 = ddotG_act;
			fprintf(lvlog,"RTT_T1 = %f %f %f\r\n",RTT_T1.x,RTT_T1.y,RTT_T1.z);
			fprintf(lvlog,"RTT_T2 = %f %f %f\r\n",RTT_T2.x,RTT_T2.y,RTT_T2.z);

			RTT_T2 = mul(MX_K,RTT_T2);
			fprintf(lvlog,"RTT_T2 (mul) = %f %f %f\r\n",RTT_T2.x,RTT_T2.y,RTT_T2.z);

			RTT_T1 = RTT_T1+RTT_T2;	  
			fprintf(lvlog,"RTT_T1 (add) = %f %f %f\r\n",RTT_T1.x,RTT_T1.y,RTT_T1.z);

			ddot_xi_G   = 0.5*RTT_T1.x;
			ddot_eta_G  = 0.5*RTT_T1.y;
			ddot_zeta_G = 0.5*RTT_T1.z;
			fprintf(lvlog,"ddot_XEZ_G = %f %f %f\r\n",ddot_xi_G,ddot_eta_G,ddot_zeta_G);

			// ESTIMATED TIME-TO-GO
			fprintf(lvlog,"--- ESTIMATED TIME-TO-GO ---\r\n");

			dot_dxit   = dot_xi_T - DotXEZ.x - (ddot_xi_G*Tt_T);
			dot_detat  = -DotXEZ.y - (ddot_eta_G * Tt_T);
			dot_dzetat = dot_zeta_T - DotXEZ.z - (ddot_zeta_G * Tt_T);
			fprintf(lvlog,"dot_XEZt = %f %f %f\r\n",dot_dxit,dot_detat,dot_dzetat);
			dV = pow((pow(dot_dxit,2)+pow(dot_detat,2)+pow(dot_dzetat,2)),0.5);
			dL_2 = (((pow(dot_dxit,2)+pow(dot_detat,2)+pow(dot_dzetat,2))/Lt_Y)-Lt_Y)/2;
			// if(dL_3 < 0){ sprintf(oapiDebugString(),"Est TTG: dL_3 %f (X/E/Z %f %f %f) @ Cycle %d (TB%d+%f)",dL_3,dot_dxit,dot_detat,dot_dzetat,IGMCycle,LVDC_Timebase,LVDC_TB_ETime);
			//	LVDC_GP_PC = 30; break; } 

			dT_2 = (dL_2*(tau2-Tt_2))/V_ex2;
			T_2 = Tt_2 + dT_2;
			T_T = Tt_T + dT_2;
			fprintf(lvlog,"dV = %f, dL_2 = %f, dT_2 = %f, T_2 = %f, T_T = %f\r\n",dV,dL_2,dT_2,T_2,T_T);

			// TARGET PARAMETER UPDATE
			if(!(UP > 0)){	
				fprintf(lvlog,"--- TARGET PARAMETER UPDATE ---\r\n");
				UP = 1; 
				Tt_2 = T_2;
				Tt_T = T_T;
				fprintf(lvlog,"UP = 1, Tt_2 = %f, Tt_T = %f\r\n",Tt_2,Tt_T);
				Lt_2 = Lt_2 + dL_2;
				Lt_Y = Lt_Y + dL_2;
				Jt_2 = Jt_2 + (dL_2*T_2);
				fprintf(lvlog,"Lt_2 = %f, Lt_Y = %f, Jt_2 = %f\r\n",Lt_2,Lt_Y,Jt_2);

				// NOTE: This is perfectly valid. Just because Dijkstra and Wirth think otherwise
				// does not mean it's gospel. I shouldn't have to defend my choice of instructions
				// because a bunch of people read the title of the paper with no context and take
				// it as a direct revelation from God with no further study into the issue.
				fprintf(lvlog,"RECYCLE\r\n");
				goto gtupdate; // Recycle. 
			}

			// tchi_y AND tchi_p CALCULATIONS
			fprintf(lvlog,"--- tchi_y/p CALCULATION ---\r\n");

			L_2 = Lt_2 + dL_2;
			J_2 = Jt_2 + (dL_2*T_2);
			S_2 = (L_2*T_2)-J_2;
			Q_2 = (S_2*tau2)-((V_ex2*pow(T_2,2))/2);
			P_2 = (J_2*(tau2+(2*T_2)))-((V_ex2*pow(T_2,2))/2);
			U_2 = (Q_2*(tau2+(2*T_2)))-((V_ex2*pow(T_2,3))/6);
			fprintf(lvlog,"L_2 = %f, J_2 = %f, S_2 = %f, Q_2 = %f, P_2 = %f, U_2 = %f\r\n",L_2,J_2,S_2,Q_2,P_2,U_2);

			// This is where velocity-to-be-gained is generated.

			dot_dxi   = dot_dxit   - (ddot_xi_G   * dT_2);
			dot_deta  = dot_detat  - (ddot_eta_G  * dT_2);
			dot_dzeta = dot_dzetat - (ddot_zeta_G * dT_2);
			fprintf(lvlog,"dot_dXEZ = %f %f %f\r\n",dot_dxi,dot_deta,dot_dzeta);

				
	//		sprintf(oapiDebugString(),".dxi = %f | .deta %f | .dzeta %f | dT3 %f",
	//			dot_dxi,dot_deta,dot_dzeta,dT_3);

			L_Y = L_1 + L_2;
			tchi_y_last = tchi_y;
			tchi_p_last = tchi_p;
			tchi_y = atan2(dot_deta,pow(pow(dot_dxi,2)+pow(dot_dzeta,2),0.5));
			tchi_p = atan2(dot_dxi,dot_dzeta);				
			UP = -1;
			fprintf(lvlog,"L_Y = %f, tchi_y = %f, tchi_p = %f, UP = -1\r\n",L_Y,tchi_y,tchi_p);

			// *** END OF CHI-TILDE LOGIC ***
			// Is it time for chi-tilde mode?
			if(Tt_T <= eps_2){
				fprintf(lvlog,"CHI BAR STEERING ON, REMOVE ALTITUDE CONSTRAINS (K_1-4 = 0)\r\n");
				// Yes
				// Go to the test that we would be testing if HSL was true
				K_1 = 0; K_2 = 0; K_3 = 0; K_4 = 0;
				// See the note above if the presence of this goto bothers you.
				// sprintf(oapiDebugString(),"LVDC: HISPEED LOOP ENTRY: Tt_T %f eps_2 %f", Tt_T,eps_2); LVDC_GP_PC = 30; break; // STOP
				goto hsl;
			}else{
				// No.
				// YAW STEERING PARAMETERS
				fprintf(lvlog,"--- YAW STEERING PARAMETERS ---\r\n");

				J_Y = J_1 + J_2 + (L_2*T_2);
				S_Y = S_1 - J_2 + (L_Y*T_2);
				Q_Y = Q_1 + Q_2 + (S_2*T_2) + ((T_2)*J_1);
				K_Y = L_Y/J_Y;
				D_Y = S_Y - (K_Y*Q_Y);
				fprintf(lvlog,"J_Y = %f, S_Y = %f, Q_Y = %f, K_Y = %f, D_Y = %f\r\n",J_Y,S_Y,Q_Y,K_Y,D_Y);

				deta = PosXEZ.y + (DotXEZ.y*T_T) + ((ddot_eta_G*pow(T_T,2))/2) + (S_Y*(sin(tchi_y)));
				K_3 = deta/(D_Y*(cos(tchi_y)));
				K_4 = K_Y*K_3;
				fprintf(lvlog,"deta = %f, K_3 = %f, K_4 = %f\r\n",deta,K_3,K_4);

				// PITCH STEERING PARAMETERS
				fprintf(lvlog,"--- PITCH STEERING PARAMETERS ---\r\n");

				L_P = L_Y*cos(tchi_y);
				C_2 = cos(tchi_y)+(K_3*sin(tchi_y));
				C_4 = K_4*sin(tchi_y);
				J_P = (J_Y*C_2) - (C_4*(P_1+P_2+(pow(T_2,2)*L_2)));
				fprintf(lvlog,"L_P = %f, C_2 = %f, C_4 = %f, J_P = %f\r\n",L_P,C_2,C_4,J_P);

				S_P = (S_Y*C_2) - (C_4*Q_Y);
				Q_P = (Q_Y*C_2) - (C_4*(U_1+U_2+(pow(T_2,2)*S_2)+((T_2)*P_1)));
				K_P = L_P/J_P;
				D_P = S_P - (K_P*Q_P);
				fprintf(lvlog,"S_P = %f, Q_P = %f, K_P = %f, D_P = %f\r\n",S_P,Q_P,K_P,D_P);

				dxi = PosXEZ.x - xi_T + (DotXEZ.x*T_T) + ((ddot_xi_G*pow(T_T,2))/2) + (S_P*(sin(tchi_p)));
				K_1 = dxi/(D_P*cos(tchi_p));
				K_2 = K_P*K_1;
				fprintf(lvlog,"dxi = %f, K_1 = %f, K_2 = %f, cos(tchi_p) = %f\r\n",dxi,K_1,K_2,cos(tchi_p));
			}
		}else{
hsl:		// HIGH-SPEED LOOP ENTRY				
			// CUTOFF VELOCITY EQUATIONS
			fprintf(lvlog,"--- CUTOFF VELOCITY EQUATIONS ---\r\n");
			V_0 = V_1;
			V_1 = V_2;
			//V_2 = 0.5 * (V+(pow(V_1,2)/V));
			V_2 = V;
			dtt_1 = dtt_2;
			dtt_2 = dt_c;					
			fprintf(lvlog,"V = %f, Tt_t = %f\r\n",V,Tt_T);
			fprintf(lvlog,"V = %f, V_0 = %f, V_1 = %f, V_2 = %f, dtt_1 = %f, dtt_2 = %f\r\n",V,V_0,V_1,V_2,dtt_1,dtt_2);
			if(Tt_T <= eps_4 && V + V_TC >= V_T){
				fprintf(lvlog,"--- HI SPEED LOOP ---\r\n");
				// TGO CALCULATION
				fprintf(lvlog,"--- TGO CALCULATION ---\r\n");
				if(GATE5 == false){
					fprintf(lvlog,"CHI FREEZE\r\n");
					// CHI FREEZE
					tchi_y = tchi_y_last;
					tchi_p = tchi_p_last;
					HSL = true;
					GATE5 = true;
					T_GO = T_2;
					fprintf(lvlog,"HSL = true, GATE5 = true, T_GO = %f\r\n",T_GO);
				}
					
				// TGO DETERMINATION
				fprintf(lvlog,"--- TGO DETERMINATION ---\r\n");

				a_2 = (((V_2-V_1)*dtt_1)-((V_1-V_0)*dtt_2))/(dtt_2*dtt_1*(dtt_2+dtt_1));
				a_1 = ((V_2-V_1)/dtt_2)+(a_2*dtt_2);
				T_GO = ((V_T-dV_B)-V_2)/(a_1+a_2*T_GO);
				T_CO = TAS+T_GO;
				fprintf(lvlog,"a_2 = %f, a_1 = %f, T_GO = %f, T_CO = %f, V_T = %f\r\n",a_2,a_1,T_GO,T_CO,V_T);

				// Done, go to navigation
				//sprintf(oapiDebugString(),"TB%d+%f | CP/Y %f %f | -HSL- TGO %f",
				//	LVDC_Timebase,LVDC_TB_ETime,PITCH,YAW,T_GO);

				goto minorloop;
			}
			// End of HSL
		}
		// GUIDANCE TIME UPDATE
		fprintf(lvlog,"--- GUIDANCE TIME UPDATE ---\r\n");

		if(BOOST){
			if(MRS == false){
				T_1 = T_1 - dt_c; 
			}else{
				if(t_B1 <= t_B3){
					T_2 = T_2 - dt_c;
				}else{
					// Here if t_B1 is bigger.
					fprintf(lvlog,"t_B1 = %f, t_B3 = %f\r\n",t_B1,t_B3);
					T_1 = (((dotM_1*(t_B3-t_B1))-(dotM_2*t_B3))*dt)/(dotM_1*t_B1);
				}
			}
		}
		fprintf(lvlog,"T_1 = %f, T_2 = %f, dt_c = %f\r\n",T_1,T_2,dt_c);
		Tt_2 = T_2;
		Tt_T = T_1+Tt_2;
		fprintf(lvlog,"Tt_2 = %f, Tt_T = %f\r\n",Tt_2,Tt_T);
	
		// IGM STEERING ANGLES
		fprintf(lvlog,"--- IGM STEERING ANGLES ---\r\n");

		//sprintf(oapiDebugString(),"IGM: K_1 %f K_2 %f K_3 %f K_4 %f",K_1,K_2,K_3,K_4);
		Xtt_y = ((tchi_y) - K_3 + (K_4 * t));
		Xtt_p = ((tchi_p) - K_1 + (K_2 * t));
		fprintf(lvlog,"Xtt_y = %f, Xtt_p = %f\r\n",Xtt_y,Xtt_p);

		// -- COMPUTE INVERSE OF [K] --
		// Get Determinate
		double det = MX_K.m11 * ((MX_K.m22*MX_K.m33) - (MX_K.m32*MX_K.m23))
					- MX_K.m12 * ((MX_K.m21*MX_K.m33) - (MX_K.m31*MX_K.m23))
					+ MX_K.m13 * ((MX_K.m21*MX_K.m32) - (MX_K.m31*MX_K.m22));
		// If the determinate is less than 0.0005, this is invalid.
		fprintf(lvlog,"det = %f (LESS THAN 0.0005 IS INVALID)\r\n",det);

		MATRIX3 MX_Ki; // TEMPORARY: Inverse of [K]
		MX_Ki.m11 =   ((MX_K.m22*MX_K.m33) - (MX_K.m23*MX_K.m32))  / det;
		MX_Ki.m12 =   ((MX_K.m13*MX_K.m32) - (MX_K.m12*MX_K.m33))  / det;
		MX_Ki.m13 =   ((MX_K.m12*MX_K.m23) - (MX_K.m13*MX_K.m22))  / det;
		MX_Ki.m21 =   ((MX_K.m23*MX_K.m31) - (MX_K.m21*MX_K.m33))  / det;
		MX_Ki.m22 =   ((MX_K.m11*MX_K.m33) - (MX_K.m13*MX_K.m31))  / det;
		MX_Ki.m23 =   ((MX_K.m13*MX_K.m21) - (MX_K.m11*MX_K.m23))  / det;
		MX_Ki.m31 =   ((MX_K.m21*MX_K.m32) - (MX_K.m22*MX_K.m31))  / det;
		MX_Ki.m32 =   ((MX_K.m12*MX_K.m31) - (MX_K.m11*MX_K.m32))  / det;
		MX_Ki.m33 =   ((MX_K.m11*MX_K.m22) - (MX_K.m12*MX_K.m21))  / det;
		fprintf(lvlog,"MX_Ki R1 = %f %f %f\r\n",MX_Ki.m11,MX_Ki.m12,MX_Ki.m13);
		fprintf(lvlog,"MX_Ki R2 = %f %f %f\r\n",MX_Ki.m21,MX_Ki.m22,MX_Ki.m23);
		fprintf(lvlog,"MX_Ki R3 = %f %f %f\r\n",MX_Ki.m31,MX_Ki.m32,MX_Ki.m33);

		// Done
		VECTOR3 VT; 
		VT.x = (sin(Xtt_p)*cos(Xtt_y));
		VT.y = (sin(Xtt_y));
		VT.z = (cos(Xtt_p)*cos(Xtt_y));
		fprintf(lvlog,"VT (set) = %f %f %f\r\n",VT.x,VT.y,VT.z);

		VT = mul(MX_Ki,VT);
		fprintf(lvlog,"VT (mul) = %f %f %f\r\n",VT.x,VT.y,VT.z);

		X_S1 = VT.x;
		X_S2 = VT.y;
		X_S3 = VT.z;
		fprintf(lvlog,"X_S1-3 = %f %f %f\r\n",X_S1,X_S2,X_S3);

		// FINALLY - COMMANDS!
		X_Zi = asin(X_S2);			// Yaw
		X_Yi = atan2(-X_S3,X_S1);	// Pitch
		fprintf(lvlog,"*** COMMAND ISSUED ***\r\n");
		fprintf(lvlog,"PITCH = %f, YAW = %f\r\n\r\n",X_Yi*DEG,X_Zi*DEG);
			
		// IGM is supposed to generate attitude directly.
		CommandedAttitude.x = 360 * RAD;    // ROLL
		CommandedAttitude.y = X_Yi; // PITCH
		CommandedAttitude.z = X_Zi; // YAW;				
		goto limittest;

orbitalguidance: //orbital guidance logic;

		fprintf(lvlog,"*** ORBITAL GUIDANCE ***\r\n");
		if(TAS-TA3 < 0){ //time for maneuver after CSM sep		
			if(TAS-TA1 > 0){ //1st maneuver to -20 pitch LVLH prior to CSM sep
				if(TAS-TA2 > 0){ //time for attitude hold
					if(INH2){
						alpha_1 = -20 * RAD;//if INH2: maintain orb rate
						CommandedAttitude.x = 180 * RAD;
						fprintf(lvlog,"inhibit attitude hold, maintain pitchdown\r\n");
						goto orbatt;
					}else{
						CommandedAttitude = PCommandedAttitude;//hold attitude for CSM sep
						fprintf(lvlog,"Attitude hold\r\n");
						goto minorloop;
					}
				}else{
					if(INH1){
						alpha_1 = 0 * RAD;//if INH1: no pitchdown
						CommandedAttitude.x = 360 * RAD;
						fprintf(lvlog,"inhibit pitchdown");
						goto orbatt;
					}else{
						alpha_1 = -20 * RAD;//from GRR +9780 till GRR+10275
						CommandedAttitude.x = 360 * RAD;
						fprintf(lvlog,"pitchdown");
						goto orbatt;
					}
				}
			}else{
				alpha_1 = 360 * RAD;//from TB4+20 till GRR +9780 0
				CommandedAttitude.x = 360 * RAD;
				fprintf(lvlog,"TB4+20\r\n");
				goto orbatt;
			}						
		}else{
			alpha_1 = 180 * RAD;//tail forward
			CommandedAttitude.x = 180 * RAD;//heads up
			fprintf(lvlog,"post sep attitude\r\n");
			goto orbatt;
		}

orbatt: Pos4 = mul(MX_G,PosS); //here we compute the steering angles...
		sin_chi_Yit = (Pos4.x * cos(alpha_1) + Pos4.z * sin(alpha_1))/(-R);
		cos_chi_Yit = (Pos4.z * cos(alpha_1) - Pos4.x * sin(alpha_1))/(-R);
		sin_chi_Zit = sin(alpha_2);
		cos_chi_Zit = cos(alpha_2);
		// -- COMPUTE INVERSE OF [G] -what an effort for those stupid angles!
		// Get Determinate
		double det1 = MX_G.m11 * ((MX_G.m22*MX_G.m33) - (MX_G.m32*MX_G.m23))
					- MX_G.m12 * ((MX_G.m21*MX_G.m33) - (MX_G.m31*MX_G.m23))
					+ MX_G.m13 * ((MX_G.m21*MX_G.m32) - (MX_G.m31*MX_G.m22));
		// If the determinate is less than 0.0005, this is invalid.
		MATRIX3 MX_Gi; // TEMPORARY: Inverse of [K]
		MX_Gi.m11 =   ((MX_G.m22*MX_G.m33) - (MX_G.m23*MX_G.m32))  / det1;
		MX_Gi.m12 =   ((MX_G.m13*MX_G.m32) - (MX_G.m12*MX_G.m33))  / det1;
		MX_Gi.m13 =   ((MX_G.m12*MX_G.m23) - (MX_G.m13*MX_G.m22))  / det1;
		MX_Gi.m21 =   ((MX_G.m23*MX_G.m31) - (MX_G.m21*MX_G.m33))  / det1;
		MX_Gi.m22 =   ((MX_G.m11*MX_G.m33) - (MX_G.m13*MX_G.m31))  / det1;
		MX_Gi.m23 =   ((MX_G.m13*MX_G.m21) - (MX_G.m11*MX_G.m23))  / det1;
		MX_Gi.m31 =   ((MX_G.m21*MX_G.m32) - (MX_G.m22*MX_G.m31))  / det1;
		MX_Gi.m32 =   ((MX_G.m12*MX_G.m31) - (MX_G.m11*MX_G.m32))  / det1;
		MX_Gi.m33 =   ((MX_G.m11*MX_G.m22) - (MX_G.m12*MX_G.m21))  / det1;
		VECTOR3 VT1; 
		VT1.x = (cos_chi_Yit * cos_chi_Zit);
		VT1.y = (sin_chi_Zit);
		VT1.z = (-sin_chi_Yit * cos_chi_Zit);
		fprintf(lvlog,"VT (set) = %f %f %f\r\n",VT1.x,VT1.y,VT1.z);

		VT1 = mul(MX_Gi,VT1);
		fprintf(lvlog,"VT (mul) = %f %f %f\r\n",VT1.x,VT1.y,VT1.z);

		X_S1 = VT1.x;
		X_S2 = VT1.y;
		X_S3 = VT1.z;
		fprintf(lvlog,"X_S1-3 = %f %f %f\r\n",X_S1,X_S2,X_S3);

		// COMMANDS
		X_Zi = asin(X_S2);			// Yaw
		X_Yi = atan2(-X_S3,X_S1);	// Pitch
		fprintf(lvlog,"*** COMMAND ISSUED ***\r\n");
		fprintf(lvlog,"PITCH = %f, YAW = %f\r\n\r\n",X_Yi*DEG,X_Zi*DEG);
		CommandedAttitude.y = X_Yi; // PITCH
		CommandedAttitude.z = X_Zi; // YAW;				

limittest:
		//command rate test; part of major loop;
		//if(CommandedAttitude.z < -45 * RAD && CommandedAttitude.z >= -180 * RAD){CommandedAttitude.z = -45 * RAD;}
		//if(CommandedAttitude.z > 45 * RAD && CommandedAttitude.z <= 180 * RAD){CommandedAttitude.z = 45 * RAD;}
		double diff;//aux variable for limit test
		diff = fmod((CommandedAttitude.x - PCommandedAttitude.x + TWO_PI),TWO_PI);
		if(diff > PI){ diff -= TWO_PI; }
		if(abs(diff/dt_c) > CommandRateLimits.x){
			if(diff > 0){
				CommandedAttitude.x = fmod(PCommandedAttitude.x + CommandRateLimits.x * dt_c,TWO_PI);
			}else{
				CommandedAttitude.x = fmod(PCommandedAttitude.x - CommandRateLimits.x * dt_c,TWO_PI);
			}
		}
		diff = fmod((CommandedAttitude.y - PCommandedAttitude.y + TWO_PI),TWO_PI);
		if(diff > PI){ diff -= TWO_PI; }
		if(abs(diff/dt_c) > CommandRateLimits.y){
			if(diff > 0){
				CommandedAttitude.y = fmod(PCommandedAttitude.y + CommandRateLimits.y * dt_c,TWO_PI);
			}else{
				CommandedAttitude.y = fmod(PCommandedAttitude.y - CommandRateLimits.y * dt_c,TWO_PI);
			}
		}
		diff = fmod((CommandedAttitude.z - PCommandedAttitude.z + TWO_PI),TWO_PI);
		if(diff > PI) {diff -= TWO_PI;} ;
		if(abs(diff/dt_c) > CommandRateLimits.z){
			if(diff > 0){
				CommandedAttitude.z = fmod(PCommandedAttitude.z + CommandRateLimits.z * dt_c,TWO_PI);
			}else{
				CommandedAttitude.z = fmod(PCommandedAttitude.z - CommandRateLimits.z * dt_c,TWO_PI);
			}
		}
		PCommandedAttitude = CommandedAttitude;

minorloop: //minor loop;
		if(T_GO - sinceLastIGM <= 0 && HSL == true && S4B_IGN == true){
			//Time for S4B cutoff? We need to check that here -IGM runs every 2 sec only, but cutoff has to be on the second
			owner->SetThrusterLevel(owner->th_main[0], 0);
			S4B_IGN = false;
			HSL = false;
			BOOST = false;
			LVDC_Timebase = 4;
			LVDC_TB_ETime = 0;
			fprintf(lvlog,"SIVB CUTOFF! TAS = %f \r\n",TAS);
		};
		//calculate delta attitude
		DeltaAtt.x = fmod((CurrentAttitude.x - CommandedAttitude.x + TWO_PI),TWO_PI);
		if (DeltaAtt.x > PI){ DeltaAtt.x -= TWO_PI; }	
		DeltaAtt.y = fmod((CurrentAttitude.y - CommandedAttitude.y + TWO_PI),TWO_PI);
		if (DeltaAtt.y > PI){ DeltaAtt.y -= TWO_PI; }
		DeltaAtt.z = fmod((CurrentAttitude.z - CommandedAttitude.z + TWO_PI),TWO_PI);
		if (DeltaAtt.z > PI){ DeltaAtt.z -= TWO_PI; }

		//-euler correction-
		//calculate correction factors
		A1 = cos(CurrentAttitude.x) * cos(CurrentAttitude.z);
		A2 = sin(CurrentAttitude.x);
		A3 = sin(CurrentAttitude.z);
		A4 = sin(CurrentAttitude.x) * cos(CurrentAttitude.z);
		A5 = cos(CurrentAttitude.x);
		// ROLL ERROR
		AttitudeError.x =-(DeltaAtt.x + A3 * DeltaAtt.y);
		// PITCH ERROR
		AttitudeError.y = -(A1 * DeltaAtt.y + A2 * DeltaAtt.z); 
		// YAW ERROR
		AttitudeError.z = -(-A4 * DeltaAtt.y + A5 * DeltaAtt.z);
	
		// S/C takeover function
		if(LVDC_Timebase == 4 && (owner->LVGuidanceSwitch.IsDown() && owner->agc.GetInputChannelBit(012,9))){
			//scaling factor seems to be 31.6; didn't find any source for it, but at least it leads to the right rates
			//note that any 'threshold solution' is pointless: ARTEMIS supports EMEM-selectable saturn rate output
			AttitudeError.x = owner->gdc.fdai_err_x * RAD / 31.6;
			AttitudeError.y = owner->gdc.fdai_err_y * RAD / 31.6;
			AttitudeError.z = owner->gdc.fdai_err_z * RAD / 31.6;
		}
		/* **** FLIGHT CONTROL COMPUTER OPERATIONS **** */
		if(LVDC_Timebase == 1 && LVDC_TB_ETime < 60){
			a_0p = a_0y = 2;
			a_0r = 0.3;
			a_1p = a_1y = 1.6;
			a_1r = 0.25;
		}
		if(LVDC_Timebase == 1 && LVDC_TB_ETime >= 60 && LVDC_TB_ETime < 90){
			a_0p = a_0y = 3;
			a_0r = 0.3;
			a_1p = a_1y = 1.6;
			a_1r = 0.25;
		}
		if(LVDC_Timebase == 1 && LVDC_TB_ETime >= 90 && LVDC_TB_ETime < 120){
			a_0p = a_0y = 2;
			a_0r = 0.3;
			a_1p = a_1y = 1.6;
			a_1r = 0.25;
		}
		if(LVDC_Timebase == 1 && LVDC_TB_ETime >= 120){
			a_0p = a_0y = 0.7;
			a_0r = 0.2;
			a_1p = a_1y = 0.75;
			a_1r = 0.15;
		}
		if(LVDC_Timebase == 3){
			a_0p = a_0y = 0.6;
			a_0r = 1;
			a_1p = a_1y = 0.5;
			a_1r = 5;
		}
		if(LVDC_Timebase == 4){
			a_0p = a_0y = 1;
			a_0r = 1;
			a_1p = a_1y = 5;
			a_1r = 5;
		}
		beta_pc = a_0p * AttitudeError.y + a_1p * AttRate.y;
		beta_yc = a_0y * AttitudeError.z + a_1y * AttRate.z;
		beta_rc = a_0r * AttitudeError.x + a_1r * AttRate.x;
		if(LVDC_Timebase < 3){
			beta_y1c = beta_yc + beta_rc/pow(2,0.5);//orbiter's engines are gimballed differently then the real one
			beta_p1c = beta_pc - beta_rc/pow(2,0.5);
			beta_y2c = beta_yc - beta_rc/pow(2,0.5);
			beta_p2c = beta_pc - beta_rc/pow(2,0.5);
			beta_y3c = beta_yc - beta_rc/pow(2,0.5);
			beta_p3c = beta_pc + beta_rc/pow(2,0.5);
			beta_y4c = beta_yc + beta_rc/pow(2,0.5);
			beta_p4c = beta_pc + beta_rc/pow(2,0.5);
			owner->SetThrusterDir(owner->th_main[0],_V(beta_y1c,beta_p1c,1)); 
			owner->SetThrusterDir(owner->th_main[1],_V(beta_y2c,beta_p2c,1));
			owner->SetThrusterDir(owner->th_main[2],_V(beta_y3c,beta_p3c,1)); 
			owner->SetThrusterDir(owner->th_main[3],_V(beta_y4c,beta_p4c,1)); 
		}
		if(LVDC_Timebase == 3){
			//SIVB powered flight
			beta_p1c = beta_pc;//gimbal angles
			beta_y1c = beta_yc;
			owner->SetThrusterDir(owner->th_main[0],_V(beta_y1c,beta_p1c,1));
			eps_p = 0;//we want neither the APS pitch thrusters to fire
			eps_ymr = -(a_0r * AttitudeError.x * DEG) - (a_1r * AttRate.x * DEG);//nor the yaw thrusters
			eps_ypr = (a_0r * AttitudeError.x * DEG) + (a_1r * AttRate.x * DEG);
		}
		if(LVDC_Timebase == 4){
			//SIVB coast flight; full APS control
			eps_p   = (a_0p * AttitudeError.y * DEG) + (a_1p * AttRate.y * DEG);//pitch thruster demand
			eps_ymr = (a_0y * AttitudeError.z * DEG) - (a_0r * AttitudeError.x * DEG) + (a_1y * AttRate.z * DEG) - (a_1r * AttRate.x * DEG);//yaw minus roll
			eps_ypr = (a_0y * AttitudeError.z * DEG) + (a_0r * AttitudeError.x * DEG) + (a_1y * AttRate.z * DEG) + (a_1r * AttRate.x * DEG);//yaw plus roll
		}
		if((LVDC_Timebase == 3 && S4B_IGN == true)|| LVDC_Timebase == 4){
			//APS thruster on/off control
			if(eps_p > 1){
				//fire+pitch
				if(eps_p >= 1.6){owner->SetThrusterLevel(owner->th_att_rot[1],1);}else{owner->SetThrusterLevel(owner->th_att_rot[1],(eps_p-1)/0.6);}
			}else{
				owner->SetThrusterLevel(owner->th_att_rot[1],0);
			}
			if(eps_p < -1){
				//fire-pitch
				if(eps_p <= -1.6){owner->SetThrusterLevel(owner->th_att_rot[0],1);}else{owner->SetThrusterLevel(owner->th_att_rot[0],(-eps_p-1)/0.6);}
			}else{
				owner->SetThrusterLevel(owner->th_att_rot[0],0);
			}
			if(eps_ymr > 1){
				//fire+yaw-roll;
				if(eps_ymr >= 1.6){owner->SetThrusterLevel(owner->th_att_rot[3],1);}else{owner->SetThrusterLevel(owner->th_att_rot[3],(eps_ymr-1)/0.6);}
			}else{
				owner->SetThrusterLevel(owner->th_att_rot[3],0);
			}
			if(eps_ymr < -1){
				//fire-yaw+roll;
				if(eps_ymr <= -1.6){owner->SetThrusterLevel(owner->th_att_rot[5],1);}else{owner->SetThrusterLevel(owner->th_att_rot[5],(-eps_ymr-1)/0.6);}
			}else{
				owner->SetThrusterLevel(owner->th_att_rot[5],0);
			}
			if(eps_ypr > 1){
				//fire+yaw+roll;
				if(eps_ypr >= 1.6){owner->SetThrusterLevel(owner->th_att_rot[4],1);}else{owner->SetThrusterLevel(owner->th_att_rot[4],(eps_ypr-1)/0.6);}
			}else{
				owner->SetThrusterLevel(owner->th_att_rot[4],0);
			}
			if(eps_ypr < -1){
				//fire-yaw-roll;
				if(eps_ypr <= -1.6){owner->SetThrusterLevel(owner->th_att_rot[2],1);}else{owner->SetThrusterLevel(owner->th_att_rot[2],(-eps_ypr-1)/0.6);}
			}else{
				owner->SetThrusterLevel(owner->th_att_rot[2],0);
			}
		}
		// Debug if we're launched
		if(LVDC_Timebase > -1){
			if(LVDC_Timebase < 4){
				sprintf(oapiDebugString(),"TB%d+%f | T1 = %f | T2 = %f | Tt_T = %f | ERR %f %f %f | eps %f %f %f | V = %f R = %f",
					LVDC_Timebase,LVDC_TB_ETime,
					T_1,Tt_2,Tt_T,
					AttitudeError.x*DEG,AttitudeError.y*DEG,AttitudeError.z*DEG,
					eps_p, eps_ymr, eps_ypr,V,R/1000);
			}else{
				// On-Orbit
				sprintf(oapiDebugString(),"TB%d+%.2f | ATT %.3f %.3f %.3f | CMD %.3f %.3f %.3f | ERR %.3f %.3f %.3f | eps %.3f %.3f %.3f | RATE %.3f %.3f %.3f | V = %.2f R= %.2f",
					LVDC_Timebase,LVDC_TB_ETime,
					CurrentAttitude.x*DEG,CurrentAttitude.y*DEG,CurrentAttitude.z*DEG,
					CommandedAttitude.x*DEG,CommandedAttitude.y*DEG,CommandedAttitude.z*DEG,
					AttitudeError.x*DEG,AttitudeError.y*DEG,AttitudeError.z*DEG,
					AttRate.x * DEG, AttRate.y * DEG, AttRate.z * DEG,
					eps_p, eps_ymr, eps_ypr,V,R/1000);
			}
		}
	}

	/*
		sprintf(oapiDebugString(),"LVDC: TB%d + %f | PS %f %f %f | VS %f %f %f",
			LVDC_Timebase,LVDC_TB_ETime,
			PosS.x,PosS.y,PosS.z,
			DotS.x,DotS.y,DotS.z);
			*/
//	CurrentAttitude.x*DEG,CurrentAttitude.y*DEG,CurrentAttitude.z*DEG,V);								

	// Update engine indicators and failure flags
	if(LVDC_EI_On == true){
		int i=0;
		switch(owner->stage){
			// 5-engine stages
			case PRELAUNCH_STAGE:
			case LAUNCH_STAGE_ONE:
				while (i<8){
				if(owner->GetThrusterLevel(owner->th_main[i]) >= 0.65  && owner->ENGIND[i] == true){  owner->ENGIND[i] = false; } 
				if(owner->GetThrusterLevel(owner->th_main[i]) < 0.65 && owner->ENGIND[i] == false){  owner->ENGIND[i] = true; }
				i++;
				}
				break;
			// S4B only
			case LAUNCH_STAGE_SIVB:
			case STAGE_ORBIT_SIVB:				
				if(owner->GetThrusterLevel(owner->th_main[0]) >= 0.65  && owner->ENGIND[0] == true){  owner->ENGIND[0] = false; } // UNLIGHT
				if(owner->GetThrusterLevel(owner->th_main[0]) < 0.65 && owner->ENGIND[0] == false){  owner->ENGIND[0] = true; }   // LIGHT
				break;	
			// Error
			default:
				LVDC_EI_On = false;
				break;
		}
	}else{
		owner->ENGIND[0] = false;
		owner->ENGIND[1] = false;
		owner->ENGIND[2] = false;
		owner->ENGIND[3] = false;
		owner->ENGIND[4] = false;
		owner->ENGIND[5] = false;
		owner->ENGIND[6] = false;
		owner->ENGIND[7] = false;
	}
	if(owner->stage == LAUNCH_STAGE_ONE && owner->MissionTime < 12.5){
		// Control contrail
		if(owner->MissionTime > 12){
			owner->contrailLevel = 0;
		}else{
			if (owner->MissionTime > 7){
				owner->contrailLevel = (12.0 - owner->MissionTime) / 100.0;
			}else{
				if(owner->MissionTime > 2){
					owner->contrailLevel = 1.38 - 0.95 / 5.0 * owner->MissionTime;
				}else{
					owner->contrailLevel = 1;
				}
			}
		}
	}

	/* **** ABORT HANDLING **** */
	// The abort PB will be pressed during prelaunch testing, but shouldn't actually trigger an abort before Mode 1 enabled.
	if(owner->bAbort && owner->MissionTime > -300){				
		owner->SetEngineLevel(ENGINE_MAIN, 0);					// Kill the engines
		owner->agc.SetInputChannelBit(030, 4, true);			// Notify the AGC of the abort
		owner->agc.SetInputChannelBit(030, 5, true);			// and the liftoff, if it's not set already
		sprintf(oapiDebugString(),"");							// Clear the LVDC debug line
		LVDC_Stop = true;										// Stop LVDC program
		// ABORT MODE 1 - Use of LES to extract CM
		// Allowed from T - 5 minutes until LES jettison.
		if(owner->MissionTime > -300 && owner->LESAttached){			
			owner->SetEngineLevel(ENGINE_MAIN, 0);
			owner->SeparateStage(CM_STAGE);
			owner->SetStage(CM_STAGE);
			owner->StartAbort();			// Resets MT, fires LET if attached
			owner->bAbort = false;			// No further processing required
			return; 
		}
		// ABORT MODE 2/3/4 - Eject CSM from LV
		if(owner->stage == LAUNCH_STAGE_ONE){
			// The only way we will get here is if the LET was jettisoned early for some reason.
			owner->SeparateStage(LAUNCH_STAGE_TWO);
			owner->SetStage(LAUNCH_STAGE_TWO);
			return;
		}
		if(owner->stage == LAUNCH_STAGE_TWO){
			// The only way we will get here is if the LET was jettisoned early for some reason.
			owner->SeparateStage (LAUNCH_STAGE_TWO_ISTG_JET);
			owner->SetStage(LAUNCH_STAGE_TWO_ISTG_JET);
			return;
		}
		if(owner->stage == LAUNCH_STAGE_TWO_ISTG_JET){
			// This is the most likely entry point
			owner->SeparateStage(LAUNCH_STAGE_SIVB);
			owner->SetStage(LAUNCH_STAGE_SIVB);
		}
		if(owner->stage == LAUNCH_STAGE_SIVB || owner->stage == STAGE_ORBIT_SIVB){
			// Eject CSM
			owner->SeparateStage(CSM_LEM_STAGE);
			owner->SetStage(CSM_LEM_STAGE);
			// Staging finished.
			owner->StartAbort();			// Resets MT, sets abort light, resets engine lights, etc.
		}
		// Done with the abort request.
		owner->bAbort = false;
	}
}

void LVDC1B::SaveState(FILEHANDLE scn) {
	// Saving state is only necessary if we are launched.
	if(LVDC_Timebase < 1){ return; }
	// OLD STUFF
	oapiWriteLine(scn, LVDC_START_STRING);
	/*
	papiWriteScenario_double(scn, "phi_lat", phi_lat);
	papiWriteScenario_double(scn, "phi_lng", phi_lng);
	papiWriteScenario_double(scn, "Azimuth", Azimuth);
	papiWriteScenario_double(scn, "DescNodeAngle", DescNodeAngle);
	papiWriteScenario_double(scn, "Inclination", Inclination);
	oapiWriteScenario_int(scn, "TB", LVDC_Timebase);
	papiWriteScenario_double(scn, "TB_ETime", LVDC_TB_ETime);
	papiWriteScenario_double(scn, "TAS", TAS);
	papiWriteScenario_double(scn, "t_clock", t_clock);
	papiWriteScenario_double(scn, "sinceLastIGM", sinceLastIGM);
	papiWriteScenario_vec(scn,"DotM_act",DotM_act);
	papiWriteScenario_vec(scn,"DotM_last",DotM_last);
	papiWriteScenario_vec(scn,"PsnS",PosS);
	papiWriteScenario_vec(scn,"Dot0",Dot0);
	papiWriteScenario_vec(scn,"DotG_last",DotG_last);
	papiWriteScenario_vec(scn,"ddotG_last",ddotG_last);
	papiWriteScenario_vec(scn,"PCommandedAttitude",PCommandedAttitude);
	*/
	// NEW STUFF
	// Thank heaven for text processing.
	// bool
	oapiWriteScenario_int(scn, "LVDC_alpha_D_op", alpha_D_op);
	oapiWriteScenario_int(scn, "LVDC_BOOST", BOOST);
	oapiWriteScenario_int(scn, "LVDC_CountPIPA", CountPIPA);
	oapiWriteScenario_int(scn, "LVDC_GATE", GATE);
	oapiWriteScenario_int(scn, "LVDC_GATE5", GATE5);
	oapiWriteScenario_int(scn, "LVDC_GRR_init", GRR_init);
	oapiWriteScenario_int(scn, "LVDC_HSL", HSL);
	oapiWriteScenario_int(scn, "LVDC_INH", INH);
	oapiWriteScenario_int(scn, "LVDC_INH1", INH1);
	oapiWriteScenario_int(scn, "LVDC_INH2", INH2);
	oapiWriteScenario_int(scn, "LVDC_i_op", i_op);
	oapiWriteScenario_int(scn, "LVDC_liftoff", liftoff);
	oapiWriteScenario_int(scn, "LVDC_LVDC_EI_On", LVDC_EI_On);
	oapiWriteScenario_int(scn, "LVDC_LVDC_GRR", LVDC_GRR);
	oapiWriteScenario_int(scn, "LVDC_LVDC_Stop", LVDC_Stop);
	oapiWriteScenario_int(scn, "LVDC_MRS", MRS);
	oapiWriteScenario_int(scn, "LVDC_poweredflight", poweredflight);
	oapiWriteScenario_int(scn, "LVDC_S1B_Engine_Out", S1B_Engine_Out);
	oapiWriteScenario_int(scn, "LVDC_S4B_IGN", S4B_IGN);
	oapiWriteScenario_int(scn, "LVDC_theta_N_op", theta_N_op);
	// int
	oapiWriteScenario_int(scn, "LVDC_IGMCycle", IGMCycle);
	oapiWriteScenario_int(scn, "LVDC_LVDC_Timebase", LVDC_Timebase);
	oapiWriteScenario_int(scn, "LVDC_T_EO1", T_EO1);
	oapiWriteScenario_int(scn, "LVDC_T_EO2", T_EO2);
	oapiWriteScenario_int(scn, "LVDC_UP", UP);
	// double
	papiWriteScenario_double(scn, "LVDC_a", a);
	papiWriteScenario_double(scn, "LVDC_a_0p", a_0p);
	papiWriteScenario_double(scn, "LVDC_a_0r", a_0r);
	papiWriteScenario_double(scn, "LVDC_a_0y", a_0y);
	papiWriteScenario_double(scn, "LVDC_a_1", a_1);
	papiWriteScenario_double(scn, "LVDC_a_2", a_2);
	papiWriteScenario_double(scn, "LVDC_A1", A1);
	papiWriteScenario_double(scn, "LVDC_A2", A2);
	papiWriteScenario_double(scn, "LVDC_A3", A3);
	papiWriteScenario_double(scn, "LVDC_A4", A4);
	papiWriteScenario_double(scn, "LVDC_A5", A5);
	papiWriteScenario_double(scn, "LVDC_a_1p", a_1p);
	papiWriteScenario_double(scn, "LVDC_a_1r", a_1r);
	papiWriteScenario_double(scn, "LVDC_a_1y", a_1y);
	papiWriteScenario_double(scn, "LVDC_alpha_1", alpha_1);
	papiWriteScenario_double(scn, "LVDC_alpha_2", alpha_2);
	papiWriteScenario_double(scn, "LVDC_alpha_D", alpha_D);
	papiWriteScenario_double(scn, "LVDC_Azimuth", Azimuth);
	papiWriteScenario_double(scn, "LVDC_Azo", Azo);
	papiWriteScenario_double(scn, "LVDC_Azs", Azs);
	papiWriteScenario_double(scn, "LVDC_B_11", B_11);
	papiWriteScenario_double(scn, "LVDC_B_21", B_21);
	papiWriteScenario_double(scn, "LVDC_B_12", B_12);
	papiWriteScenario_double(scn, "LVDC_B_22", B_22);
	papiWriteScenario_double(scn, "LVDC_beta_p1c", beta_p1c);
	papiWriteScenario_double(scn, "LVDC_beta_p2c", beta_p2c);
	papiWriteScenario_double(scn, "LVDC_beta_p3c", beta_p3c);
	papiWriteScenario_double(scn, "LVDC_beta_p4c", beta_p4c);
	papiWriteScenario_double(scn, "LVDC_beta_pc", beta_pc);
	papiWriteScenario_double(scn, "LVDC_beta_rc", beta_rc);
	papiWriteScenario_double(scn, "LVDC_beta_y1c", beta_y1c);
	papiWriteScenario_double(scn, "LVDC_beta_y2c", beta_y2c);
	papiWriteScenario_double(scn, "LVDC_beta_y3c", beta_y3c);
	papiWriteScenario_double(scn, "LVDC_beta_y4c", beta_y4c);
	papiWriteScenario_double(scn, "LVDC_beta_yc", beta_yc);
	papiWriteScenario_double(scn, "LVDC_C_2", C_2);
	papiWriteScenario_double(scn, "LVDC_C_4", C_4);
	papiWriteScenario_double(scn, "LVDC_C_3", C_3);
	papiWriteScenario_double(scn, "LVDC_CG", CG);
	papiWriteScenario_double(scn, "LVDC_cos_chi_Yit", cos_chi_Yit);
	papiWriteScenario_double(scn, "LVDC_cos_chi_Zit", cos_chi_Zit);
	papiWriteScenario_double(scn, "LVDC_cos_phi_L", cos_phi_L);
	papiWriteScenario_double(scn, "LVDC_d2", d2);
	papiWriteScenario_double(scn, "LVDC_ddot_xi_G", ddot_xi_G);
	papiWriteScenario_double(scn, "LVDC_ddot_eta_G", ddot_eta_G);
	papiWriteScenario_double(scn, "LVDC_ddot_zeta_G", ddot_zeta_G);
	papiWriteScenario_double(scn, "LVDC_ddot_zeta_GT", ddot_zeta_GT);
	papiWriteScenario_double(scn, "LVDC_ddot_xi_GT", ddot_xi_GT);
	papiWriteScenario_double(scn, "LVDC_DescNodeAngle", DescNodeAngle);
	papiWriteScenario_double(scn, "LVDC_deta", deta);
	papiWriteScenario_double(scn, "LVDC_dxi", dxi);
	papiWriteScenario_double(scn, "LVDC_dot_dxi", dot_dxi);
	papiWriteScenario_double(scn, "LVDC_dot_deta", dot_deta);
	papiWriteScenario_double(scn, "LVDC_dot_dzeta", dot_dzeta);
	papiWriteScenario_double(scn, "LVDC_dot_dxit", dot_dxit);
	papiWriteScenario_double(scn, "LVDC_dot_detat", dot_detat);
	papiWriteScenario_double(scn, "LVDC_dot_dzetat", dot_dzetat);
	papiWriteScenario_double(scn, "LVDC_dotM_1", dotM_1);
	papiWriteScenario_double(scn, "LVDC_dotM_2", dotM_2);
	papiWriteScenario_double(scn, "LVDC_dot_phi_1", dot_phi_1);
	papiWriteScenario_double(scn, "LVDC_dot_phi_T", dot_phi_T);
	papiWriteScenario_double(scn, "LVDC_dot_zeta_T", dot_zeta_T);
	papiWriteScenario_double(scn, "LVDC_dot_xi_T", dot_xi_T);
	papiWriteScenario_double(scn, "LVDC_dot_eta_T", dot_eta_T);
	papiWriteScenario_double(scn, "LVDC_dt", dt);
	papiWriteScenario_double(scn, "LVDC_dT_2", dT_2);
	papiWriteScenario_double(scn, "LVDC_dt_c", dt_c);
	papiWriteScenario_double(scn, "LVDC_dT_F", dT_F);
	papiWriteScenario_double(scn, "LVDC_dt_LET", dt_LET);
	papiWriteScenario_double(scn, "LVDC_dtt_1", dtt_1);
	papiWriteScenario_double(scn, "LVDC_dtt_2", dtt_2);
	papiWriteScenario_double(scn, "LVDC_dV", dV);
	papiWriteScenario_double(scn, "LVDC_dV_B", dV_B);
	papiWriteScenario_double(scn, "LVDC_e", e);
	papiWriteScenario_double(scn, "LVDC_eps_2", eps_2);
	papiWriteScenario_double(scn, "LVDC_eps_3", eps_3);
	papiWriteScenario_double(scn, "LVDC_eps_4", eps_4);
	papiWriteScenario_double(scn, "LVDC_eps_p", eps_p);
	papiWriteScenario_double(scn, "LVDC_eps_ymr", eps_ymr);
	papiWriteScenario_double(scn, "LVDC_eps_ypr", eps_ypr);
	papiWriteScenario_double(scn, "LVDC_f", f);
	papiWriteScenario_double(scn, "LVDC_F", F);
	papiWriteScenario_double(scn, "LVDC_Fm", Fm);
	papiWriteScenario_double(scn, "LVDC_Fx[5][5]", Fx[5][5]);
	papiWriteScenario_double(scn, "LVDC_fx[7]", fx[7]);
	papiWriteScenario_double(scn, "LVDC_G_T", G_T);
	papiWriteScenario_double(scn, "LVDC_gx[7]", gx[7]);
	papiWriteScenario_double(scn, "LVDC_IGMInterval", IGMInterval);
	papiWriteScenario_double(scn, "LVDC_Inclination", Inclination);
	papiWriteScenario_double(scn, "LVDC_J", J);
	papiWriteScenario_double(scn, "LVDC_J_1", J_1);
	papiWriteScenario_double(scn, "LVDC_J_2", J_2);
	papiWriteScenario_double(scn, "LVDC_Jt_2", Jt_2);
	papiWriteScenario_double(scn, "LVDC_J_Y", J_Y);
	papiWriteScenario_double(scn, "LVDC_J_P", J_P);
	papiWriteScenario_double(scn, "LVDC_K_1", K_1);
	papiWriteScenario_double(scn, "LVDC_K_2", K_2);
	papiWriteScenario_double(scn, "LVDC_K_3", K_3);
	papiWriteScenario_double(scn, "LVDC_K_4", K_4);
	papiWriteScenario_double(scn, "LVDC_K_5", K_5);
	papiWriteScenario_double(scn, "LVDC_K_P1", K_P1);
	papiWriteScenario_double(scn, "LVDC_K_P2", K_P2);
	papiWriteScenario_double(scn, "LVDC_K_p", K_p);
	papiWriteScenario_double(scn, "LVDC_K_y", K_y);
	papiWriteScenario_double(scn, "LVDC_K_r", K_r);
	papiWriteScenario_double(scn, "LVDC_K_Y1", K_Y1);
	papiWriteScenario_double(scn, "LVDC_K_Y2", K_Y2);
	papiWriteScenario_double(scn, "LVDC_K_Y", K_Y);
	papiWriteScenario_double(scn, "LVDC_K_P", K_P);
	papiWriteScenario_double(scn, "LVDC_D_P", D_P);
	papiWriteScenario_double(scn, "LVDC_D_Y", D_Y);
	papiWriteScenario_double(scn, "LVDC_L_1", L_1);
	papiWriteScenario_double(scn, "LVDC_L_2", L_2);
	papiWriteScenario_double(scn, "LVDC_dL_2", dL_2);
	papiWriteScenario_double(scn, "LVDC_Lt_2", Lt_2);
	papiWriteScenario_double(scn, "LVDC_L_P", L_P);
	papiWriteScenario_double(scn, "LVDC_L_Y", L_Y);
	papiWriteScenario_double(scn, "LVDC_Lt_Y", Lt_Y);
	papiWriteScenario_double(scn, "LVDC_LVDC_TB_ETime", LVDC_TB_ETime);
	papiWriteScenario_double(scn, "LVDC_mu", mu);
	papiWriteScenario_double(scn, "LVDC_p", p);
	papiWriteScenario_double(scn, "LVDC_P_1", P_1);
	papiWriteScenario_double(scn, "LVDC_P_2", P_2);
	papiWriteScenario_double(scn, "LVDC_phi_lat", phi_lat);
	papiWriteScenario_double(scn, "LVDC_phi_lng", phi_lng);
	papiWriteScenario_double(scn, "LVDC_phi_T", phi_T);
	papiWriteScenario_double(scn, "LVDC_Q_1", Q_1);
	papiWriteScenario_double(scn, "LVDC_Q_2", Q_2);
	papiWriteScenario_double(scn, "LVDC_Q_Y", Q_Y);
	papiWriteScenario_double(scn, "LVDC_Q_P", Q_P);
	papiWriteScenario_double(scn, "LVDC_R", R);
	papiWriteScenario_double(scn, "LVDC_ROV", ROV);
	papiWriteScenario_double(scn, "LVDC_ROVs", ROVs);
	papiWriteScenario_double(scn, "LVDC_R_T", R_T);
	papiWriteScenario_double(scn, "LVDC_S1B_Sep_Time", S1B_Sep_Time);
	papiWriteScenario_double(scn, "LVDC_S_1", S_1);
	papiWriteScenario_double(scn, "LVDC_S_2", S_2);
	papiWriteScenario_double(scn, "LVDC_S_P", S_P);
	papiWriteScenario_double(scn, "LVDC_S_Y", S_Y);
	papiWriteScenario_double(scn, "LVDC_sinceLastIGM", sinceLastIGM);
	papiWriteScenario_double(scn, "LVDC_sin_chi_Yit", sin_chi_Yit);
	papiWriteScenario_double(scn, "LVDC_sin_chi_Zit", sin_chi_Zit);
	papiWriteScenario_double(scn, "LVDC_sin_phi_L", sin_phi_L);
	papiWriteScenario_double(scn, "LVDC_sin_ups", sin_ups);
	papiWriteScenario_double(scn, "LVDC_cos_ups", cos_ups);
	papiWriteScenario_double(scn, "LVDC_SMCG", SMCG);
	papiWriteScenario_double(scn, "LVDC_S", S);
	papiWriteScenario_double(scn, "LVDC_P", P);
	papiWriteScenario_double(scn, "LVDC_t", t);
	papiWriteScenario_double(scn, "LVDC_t_1", t_1);
	papiWriteScenario_double(scn, "LVDC_T_1", T_1);
	papiWriteScenario_double(scn, "LVDC_t_2", t_2);
	papiWriteScenario_double(scn, "LVDC_T_2", T_2);
	papiWriteScenario_double(scn, "LVDC_t_3", t_3);
	papiWriteScenario_double(scn, "LVDC_t_3i", t_3i);
	papiWriteScenario_double(scn, "LVDC_t_4", t_4);
	papiWriteScenario_double(scn, "LVDC_t_5", t_5);
	papiWriteScenario_double(scn, "LVDC_t_6", t_6);
	papiWriteScenario_double(scn, "LVDC_TA1", TA1);
	papiWriteScenario_double(scn, "LVDC_TA2", TA2);
	papiWriteScenario_double(scn, "LVDC_TA3", TA3);
	papiWriteScenario_double(scn, "LVDC_TABLE15_C_3", TABLE15_C_3);
	papiWriteScenario_double(scn, "LVDC_TABLE15_e", TABLE15_e);
	papiWriteScenario_double(scn, "LVDC_TABLE15_f", TABLE15_f);
	papiWriteScenario_double(scn, "LVDC_T_ar", T_ar);
	papiWriteScenario_double(scn, "LVDC_TAS", TAS);
	papiWriteScenario_double(scn, "LVDC_tau1", tau1);
	papiWriteScenario_double(scn, "LVDC_tau2", tau2);
	papiWriteScenario_double(scn, "LVDC_t_B1", t_B1);
	papiWriteScenario_double(scn, "LVDC_t_B3", t_B3);
	papiWriteScenario_double(scn, "LVDC_tchi_y_last", tchi_y_last);
	papiWriteScenario_double(scn, "LVDC_tchi_p_last", tchi_p_last);
	papiWriteScenario_double(scn, "LVDC_tchi_y", tchi_y);
	papiWriteScenario_double(scn, "LVDC_tchi_p", tchi_p);
	papiWriteScenario_double(scn, "LVDC_t_clock", t_clock);
	papiWriteScenario_double(scn, "LVDC_T_CO", T_CO);
	papiWriteScenario_double(scn, "LVDC_t_fail", t_fail);
	papiWriteScenario_double(scn, "LVDC_T_GO", T_GO);
	papiWriteScenario_double(scn, "LVDC_T_LET", T_LET);
	papiWriteScenario_double(scn, "LVDC_T_S1", T_S1);
	papiWriteScenario_double(scn, "LVDC_T_S2", T_S2);
	papiWriteScenario_double(scn, "LVDC_T_S3", T_S3);
	papiWriteScenario_double(scn, "LVDC_TSMC1", TSMC1);
	papiWriteScenario_double(scn, "LVDC_TSMC2", TSMC2);
	papiWriteScenario_double(scn, "LVDC_T_T", T_T);
	papiWriteScenario_double(scn, "LVDC_Tt_2", Tt_2);
	papiWriteScenario_double(scn, "LVDC_Tt_T", Tt_T);
	papiWriteScenario_double(scn, "LVDC_U_1", U_1);
	papiWriteScenario_double(scn, "LVDC_U_2", U_2);
	papiWriteScenario_double(scn, "LVDC_ups_T", ups_T);
	papiWriteScenario_double(scn, "LVDC_V", V);
	papiWriteScenario_double(scn, "LVDC_V_ex1", V_ex1);
	papiWriteScenario_double(scn, "LVDC_V_ex2", V_ex2);
	papiWriteScenario_double(scn, "LVDC_V_i", V_i);
	papiWriteScenario_double(scn, "LVDC_V_0", V_0);
	papiWriteScenario_double(scn, "LVDC_V_1", V_1);
	papiWriteScenario_double(scn, "LVDC_V_2", V_2);
	papiWriteScenario_double(scn, "LVDC_V_T", V_T);
	papiWriteScenario_double(scn, "LVDC_V_TC", V_TC);
	papiWriteScenario_double(scn, "LVDC_xi_T", xi_T);
	papiWriteScenario_double(scn, "LVDC_eta_T", eta_T);
	papiWriteScenario_double(scn, "LVDC_zeta_T", zeta_T);
	papiWriteScenario_double(scn, "LVDC_X_S1", X_S1);
	papiWriteScenario_double(scn, "LVDC_X_S2", X_S2);
	papiWriteScenario_double(scn, "LVDC_X_S3", X_S3);
	papiWriteScenario_double(scn, "LVDC_Xtt_y", Xtt_y);
	papiWriteScenario_double(scn, "LVDC_Xtt_p", Xtt_p);
	papiWriteScenario_double(scn, "LVDC_X_Zi", X_Zi);
	papiWriteScenario_double(scn, "LVDC_X_Yi", X_Yi);
	papiWriteScenario_double(scn, "LVDC_Y_u", Y_u);
	// VECTOR3
	papiWriteScenario_vec(scn, "LVDC_ACommandedAttitude", ACommandedAttitude);
	papiWriteScenario_vec(scn, "LVDC_AttitudeError", AttitudeError);
	papiWriteScenario_vec(scn, "LVDC_AttRate", AttRate);
	papiWriteScenario_vec(scn, "LVDC_CommandedAttitude", CommandedAttitude);
	papiWriteScenario_vec(scn, "LVDC_CommandRateLimits", CommandRateLimits);
	papiWriteScenario_vec(scn, "LVDC_CurrentAttitude", CurrentAttitude);
	papiWriteScenario_vec(scn, "LVDC_ddotG_act", ddotG_act);
	papiWriteScenario_vec(scn, "LVDC_ddotG_last", ddotG_last);
	papiWriteScenario_vec(scn, "LVDC_ddotM_act", ddotM_act);
	papiWriteScenario_vec(scn, "LVDC_ddotM_last", ddotM_last);
	papiWriteScenario_vec(scn, "LVDC_DDotXEZ_G", DDotXEZ_G);
	papiWriteScenario_vec(scn, "LVDC_DeltaAtt", DeltaAtt);
	papiWriteScenario_vec(scn, "LVDC_Dot0", Dot0);
	papiWriteScenario_vec(scn, "LVDC_DotG_act", DotG_act);
	papiWriteScenario_vec(scn, "LVDC_DotG_last", DotG_last);
	papiWriteScenario_vec(scn, "LVDC_DotM_act", DotM_act);
	papiWriteScenario_vec(scn, "LVDC_DotM_last", DotM_last);
	papiWriteScenario_vec(scn, "LVDC_DotS", DotS);
	papiWriteScenario_vec(scn, "LVDC_DotXEZ", DotXEZ);
	papiWriteScenario_vec(scn, "LVDC_PCommandedAttitude", PCommandedAttitude);
	papiWriteScenario_vec(scn, "LVDC_Pos4", Pos4);
	papiWriteScenario_vec(scn, "LVDC_PosS", PosS);
	papiWriteScenario_vec(scn, "LVDC_PosXEZ", PosXEZ);
	// MATRIX3
	papiWriteScenario_mx(scn, "LVDC_MX_A", MX_A);
	papiWriteScenario_mx(scn, "LVDC_MX_B", MX_B);
	papiWriteScenario_mx(scn, "LVDC_MX_G", MX_G);
	papiWriteScenario_mx(scn, "LVDC_MX_K", MX_K);
	papiWriteScenario_mx(scn, "LVDC_MX_phi_T", MX_phi_T);
	// Done
	oapiWriteLine(scn, LVDC_END_STRING);
	lvimu.SaveState(scn);
}
void LVDC1B::LoadState(FILEHANDLE scn){
	char *line;	
	int tmp = 0; // Used in boolean type loader

	if(Initialized){
		fprintf(lvlog,"LoadState() called\r\n");
		fflush(lvlog);
	}
	while (oapiReadScenario_nextline (scn, line)) {
		if (!strnicmp(line, LVDC_END_STRING, sizeof(LVDC_END_STRING))){
			break;
		}

		// OLD STUFF
		/*
		if (!strnicmp (line, "phi_lat", 7)) {
			sscanf(line + 7, "%lf", &phi_lat);
		} else if (!strnicmp (line, "phi_lng", 7)) {
			sscanf(line + 7, "%lf", &phi_lng);
		}else if (!strnicmp (line, "Azimuth", 7)) {
			sscanf(line + 7, "%lf", &Azimuth);
		}else if (!strnicmp (line, "DescNodeAngle", 13)) {
			sscanf(line + 13, "%lf", &DescNodeAngle);
		}else if (!strnicmp (line, "Inclination", 11)) {
			sscanf(line + 11, "%lf", &Inclination);
		}else if (!strnicmp (line, "TB", 2)) {
			sscanf(line + 2, "%i", &LVDC_Timebase);
		}else if (!strnicmp (line, "TB_ETime", 8)) {
			sscanf(line + 8, "%lf", &LVDC_TB_ETime);
		}else if (!strnicmp (line, "TAS", 3)) {
			sscanf(line + 3, "%lf", &TAS);
		}else if (!strnicmp (line, "t_clock", 7)) {
			sscanf(line + 7, "%lf", &t_clock);
		}else if (!strnicmp (line, "sinceLastIGM", 12)) {
			sscanf(line + 12, "%lf", &sinceLastIGM);
		}else if (!strnicmp (line, "DotM_act", 8)) {
			sscanf(line + 8, "%lf %lf %lf", &DotM_act.x,&DotM_act.y,&DotM_act.z);
		}else if (!strnicmp (line, "DotM_last", 9)) {
			sscanf(line + 9, "%lf %lf %lf", &DotM_last.x,&DotM_last.y,&DotM_last.z);
		}else if (!strnicmp (line, "PsnS", 4)) {
			sscanf(line + 4, "%lf %lf %lf", &PosS.x,&PosS.y,&PosS.z);
		}else if (!strnicmp (line, "Dot0", 4)) {
			sscanf(line + 4, "%lf %lf %lf", &Dot0.x,&Dot0.y,&Dot0.z);
		}else if (!strnicmp (line, "DotG_last", 9)) {
			sscanf(line + 9, "%lf %lf %lf", &DotG_last.x,&DotG_last.y,&DotG_last.z);
		}else if (!strnicmp (line, "ddotG_last", 10)) {
			sscanf(line + 10, "%lf %lf %lf", &ddotG_last.x,&ddotG_last.y,&ddotG_last.z);
		}else if (!strnicmp (line, "PCommandedAttitude", 18)) {
			sscanf(line + 18, "%lf %lf %lf", &PCommandedAttitude.x,&PCommandedAttitude.y,&PCommandedAttitude.z);
		}
		*/
		// NEW STUFF
		// Doing all this in one go makes the MS compiler barf.
		// INT
		if (strnicmp(line,"LVDC_IGMCycle",strlen("LVDC_IGMCycle"))==0){ sscanf(line+strlen("LVDC_IGMCycle"),"%i",&IGMCycle);
		} else if (strnicmp(line,"LVDC_LVDC_Timebase",strlen("LVDC_LVDC_Timebase"))==0){ sscanf(line+strlen("LVDC_LVDC_Timebase"),"%i",&LVDC_Timebase);
		} else if (strnicmp(line,"LVDC_T_EO1",strlen("LVDC_T_EO1"))==0){ sscanf(line+strlen("LVDC_T_EO1"),"%i",&T_EO1);
		} else if (strnicmp(line,"LVDC_T_EO2",strlen("LVDC_T_EO2"))==0){ sscanf(line+strlen("LVDC_T_EO2"),"%i",&T_EO2);
		} else if (strnicmp(line,"LVDC_UP",strlen("LVDC_UP"))==0){ sscanf(line+strlen("LVDC_UP"),"%i",&UP);
		}
		// BOOL
		if (strnicmp(line,"LVDC_alpha_D_op",strlen("LVDC_alpha_D_op"))==0){ sscanf(line+strlen("LVDC_alpha_D_op"),"%i",&tmp); if(tmp == 1){ alpha_D_op = true; }else{ alpha_D_op = false; }
		} else if (strnicmp(line,"LVDC_BOOST",strlen("LVDC_BOOST"))==0){ sscanf(line+strlen("LVDC_BOOST"),"%i",&tmp); if(tmp == 1){ BOOST = true; }else{ BOOST = false; }
		} else if (strnicmp(line,"LVDC_CountPIPA",strlen("LVDC_CountPIPA"))==0){ sscanf(line+strlen("LVDC_CountPIPA"),"%i",&tmp); if(tmp == 1){ CountPIPA = true; }else{ CountPIPA = false; }
		} else if (strnicmp(line,"LVDC_GATE",strlen("LVDC_GATE"))==0){ sscanf(line+strlen("LVDC_GATE"),"%i",&tmp); if(tmp == 1){ GATE = true; }else{ GATE = false; }
		} else if (strnicmp(line,"LVDC_GATE5",strlen("LVDC_GATE5"))==0){ sscanf(line+strlen("LVDC_GATE5"),"%i",&tmp); if(tmp == 1){ GATE5 = true; }else{ GATE5 = false; }
		} else if (strnicmp(line,"LVDC_GRR_init",strlen("LVDC_GRR_init"))==0){ sscanf(line+strlen("LVDC_GRR_init"),"%i",&tmp); if(tmp == 1){ GRR_init = true; }else{ GRR_init = false; }
		} else if (strnicmp(line,"LVDC_HSL",strlen("LVDC_HSL"))==0){ sscanf(line+strlen("LVDC_HSL"),"%i",&tmp); if(tmp == 1){ HSL = true; }else{ HSL = false; }
		} else if (strnicmp(line,"LVDC_INH",strlen("LVDC_INH"))==0){ sscanf(line+strlen("LVDC_INH"),"%i",&tmp); if(tmp == 1){ INH = true; }else{ INH = false; }
		} else if (strnicmp(line,"LVDC_INH1",strlen("LVDC_INH1"))==0){ sscanf(line+strlen("LVDC_INH1"),"%i",&tmp); if(tmp == 1){ INH1 = true; }else{ INH1 = false; }
		} else if (strnicmp(line,"LVDC_INH2",strlen("LVDC_INH2"))==0){ sscanf(line+strlen("LVDC_INH2"),"%i",&tmp); if(tmp == 1){ INH2 = true; }else{ INH2 = false; }
		} else if (strnicmp(line,"LVDC_i_op",strlen("LVDC_i_op"))==0){ sscanf(line+strlen("LVDC_i_op"),"%i",&tmp); if(tmp == 1){ i_op = true; }else{ i_op = false; }
		} else if (strnicmp(line,"LVDC_liftoff",strlen("LVDC_liftoff"))==0){ sscanf(line+strlen("LVDC_liftoff"),"%i",&tmp); if(tmp == 1){ liftoff = true; }else{ liftoff = false; }
		} else if (strnicmp(line,"LVDC_LVDC_EI_On",strlen("LVDC_LVDC_EI_On"))==0){ sscanf(line+strlen("LVDC_LVDC_EI_On"),"%i",&tmp); if(tmp == 1){ LVDC_EI_On = true; }else{ LVDC_EI_On = false; }
		} else if (strnicmp(line,"LVDC_LVDC_GRR",strlen("LVDC_LVDC_GRR"))==0){ sscanf(line+strlen("LVDC_LVDC_GRR"),"%i",&tmp); if(tmp == 1){ LVDC_GRR = true; }else{ LVDC_GRR = false; }
		} else if (strnicmp(line,"LVDC_LVDC_Stop",strlen("LVDC_LVDC_Stop"))==0){ sscanf(line+strlen("LVDC_LVDC_Stop"),"%i",&tmp); if(tmp == 1){ LVDC_Stop = true; }else{ LVDC_Stop = false; }
		} else if (strnicmp(line,"LVDC_MRS",strlen("LVDC_MRS"))==0){ sscanf(line+strlen("LVDC_MRS"),"%i",&tmp); if(tmp == 1){ MRS = true; }else{ MRS = false; }
		} else if (strnicmp(line,"LVDC_poweredflight",strlen("LVDC_poweredflight"))==0){ sscanf(line+strlen("LVDC_poweredflight"),"%i",&tmp); if(tmp == 1){ poweredflight = true; }else{ poweredflight = false; }
		} else if (strnicmp(line,"LVDC_S1B_Engine_Out",strlen("LVDC_S1B_Engine_Out"))==0){ sscanf(line+strlen("LVDC_S1B_Engine_Out"),"%i",&tmp); if(tmp == 1){ S1B_Engine_Out = true; }else{ S1B_Engine_Out = false; }
		} else if (strnicmp(line,"LVDC_S4B_IGN",strlen("LVDC_S4B_IGN"))==0){ sscanf(line+strlen("LVDC_S4B_IGN"),"%i",&tmp); if(tmp == 1){ S4B_IGN = true; }else{ S4B_IGN = false; }
		} else if (strnicmp(line,"LVDC_theta_N_op",strlen("LVDC_theta_N_op"))==0){ sscanf(line+strlen("LVDC_theta_N_op"),"%i",&tmp); if(tmp == 1){ theta_N_op = true; }else{ theta_N_op = false; }
		}
		// DOUBLE
		if (strnicmp(line,"LVDC_a",strlen("LVDC_a"))==0){ sscanf(line+strlen("LVDC_a"),"%lf",&a);
		} else if (strnicmp(line,"LVDC_a_0p",strlen("LVDC_a_0p"))==0){ sscanf(line+strlen("LVDC_a_0p"),"%lf",&a_0p);
		} else if (strnicmp(line,"LVDC_a_0r",strlen("LVDC_a_0r"))==0){ sscanf(line+strlen("LVDC_a_0r"),"%lf",&a_0r);
		} else if (strnicmp(line,"LVDC_a_0y",strlen("LVDC_a_0y"))==0){ sscanf(line+strlen("LVDC_a_0y"),"%lf",&a_0y);
		} else if (strnicmp(line,"LVDC_a_1",strlen("LVDC_a_1"))==0){ sscanf(line+strlen("LVDC_a_1"),"%lf",&a_1);
		} else if (strnicmp(line,"LVDC_a_2",strlen("LVDC_a_2"))==0){ sscanf(line+strlen("LVDC_a_2"),"%lf",&a_2);
		} else if (strnicmp(line,"LVDC_A1",strlen("LVDC_A1"))==0){ sscanf(line+strlen("LVDC_A1"),"%lf",&A1);
		} else if (strnicmp(line,"LVDC_A2",strlen("LVDC_A2"))==0){ sscanf(line+strlen("LVDC_A2"),"%lf",&A2);
		} else if (strnicmp(line,"LVDC_A3",strlen("LVDC_A3"))==0){ sscanf(line+strlen("LVDC_A3"),"%lf",&A3);
		} else if (strnicmp(line,"LVDC_A4",strlen("LVDC_A4"))==0){ sscanf(line+strlen("LVDC_A4"),"%lf",&A4);
		} else if (strnicmp(line,"LVDC_A5",strlen("LVDC_A5"))==0){ sscanf(line+strlen("LVDC_A5"),"%lf",&A5);
		} else if (strnicmp(line,"LVDC_a_1p",strlen("LVDC_a_1p"))==0){ sscanf(line+strlen("LVDC_a_1p"),"%lf",&a_1p);
		} else if (strnicmp(line,"LVDC_a_1r",strlen("LVDC_a_1r"))==0){ sscanf(line+strlen("LVDC_a_1r"),"%lf",&a_1r);
		} else if (strnicmp(line,"LVDC_a_1y",strlen("LVDC_a_1y"))==0){ sscanf(line+strlen("LVDC_a_1y"),"%lf",&a_1y);
		} else if (strnicmp(line,"LVDC_alpha_1",strlen("LVDC_alpha_1"))==0){ sscanf(line+strlen("LVDC_alpha_1"),"%lf",&alpha_1);
		} else if (strnicmp(line,"LVDC_alpha_2",strlen("LVDC_alpha_2"))==0){ sscanf(line+strlen("LVDC_alpha_2"),"%lf",&alpha_2);
		} else if (strnicmp(line,"LVDC_alpha_D",strlen("LVDC_alpha_D"))==0){ sscanf(line+strlen("LVDC_alpha_D"),"%lf",&alpha_D);
		} else if (strnicmp(line,"LVDC_Azimuth",strlen("LVDC_Azimuth"))==0){ sscanf(line+strlen("LVDC_Azimuth"),"%lf",&Azimuth);
		} else if (strnicmp(line,"LVDC_Azo",strlen("LVDC_Azo"))==0){ sscanf(line+strlen("LVDC_Azo"),"%lf",&Azo);
		} else if (strnicmp(line,"LVDC_Azs",strlen("LVDC_Azs"))==0){ sscanf(line+strlen("LVDC_Azs"),"%lf",&Azs);
		} else if (strnicmp(line,"LVDC_B_11",strlen("LVDC_B_11"))==0){ sscanf(line+strlen("LVDC_B_11"),"%lf",&B_11);
		} else if (strnicmp(line,"LVDC_B_21",strlen("LVDC_B_21"))==0){ sscanf(line+strlen("LVDC_B_21"),"%lf",&B_21);
		} else if (strnicmp(line,"LVDC_B_12",strlen("LVDC_B_12"))==0){ sscanf(line+strlen("LVDC_B_12"),"%lf",&B_12);
		} else if (strnicmp(line,"LVDC_B_22",strlen("LVDC_B_22"))==0){ sscanf(line+strlen("LVDC_B_22"),"%lf",&B_22);
		} else if (strnicmp(line,"LVDC_beta_p1c",strlen("LVDC_beta_p1c"))==0){ sscanf(line+strlen("LVDC_beta_p1c"),"%lf",&beta_p1c);
		} else if (strnicmp(line,"LVDC_beta_p2c",strlen("LVDC_beta_p2c"))==0){ sscanf(line+strlen("LVDC_beta_p2c"),"%lf",&beta_p2c);
		} else if (strnicmp(line,"LVDC_beta_p3c",strlen("LVDC_beta_p3c"))==0){ sscanf(line+strlen("LVDC_beta_p3c"),"%lf",&beta_p3c);
		} else if (strnicmp(line,"LVDC_beta_p4c",strlen("LVDC_beta_p4c"))==0){ sscanf(line+strlen("LVDC_beta_p4c"),"%lf",&beta_p4c);
		} else if (strnicmp(line,"LVDC_beta_pc",strlen("LVDC_beta_pc"))==0){ sscanf(line+strlen("LVDC_beta_pc"),"%lf",&beta_pc);
		} else if (strnicmp(line,"LVDC_beta_rc",strlen("LVDC_beta_rc"))==0){ sscanf(line+strlen("LVDC_beta_rc"),"%lf",&beta_rc);
		} else if (strnicmp(line,"LVDC_beta_y1c",strlen("LVDC_beta_y1c"))==0){ sscanf(line+strlen("LVDC_beta_y1c"),"%lf",&beta_y1c);
		} else if (strnicmp(line,"LVDC_beta_y2c",strlen("LVDC_beta_y2c"))==0){ sscanf(line+strlen("LVDC_beta_y2c"),"%lf",&beta_y2c);
		} else if (strnicmp(line,"LVDC_beta_y3c",strlen("LVDC_beta_y3c"))==0){ sscanf(line+strlen("LVDC_beta_y3c"),"%lf",&beta_y3c);
		} else if (strnicmp(line,"LVDC_beta_y4c",strlen("LVDC_beta_y4c"))==0){ sscanf(line+strlen("LVDC_beta_y4c"),"%lf",&beta_y4c);
		} else if (strnicmp(line,"LVDC_beta_yc",strlen("LVDC_beta_yc"))==0){ sscanf(line+strlen("LVDC_beta_yc"),"%lf",&beta_yc);
		} else if (strnicmp(line,"LVDC_C_2",strlen("LVDC_C_2"))==0){ sscanf(line+strlen("LVDC_C_2"),"%lf",&C_2);
		} else if (strnicmp(line,"LVDC_C_4",strlen("LVDC_C_4"))==0){ sscanf(line+strlen("LVDC_C_4"),"%lf",&C_4);
		} else if (strnicmp(line,"LVDC_C_3",strlen("LVDC_C_3"))==0){ sscanf(line+strlen("LVDC_C_3"),"%lf",&C_3);
		} else if (strnicmp(line,"LVDC_CG",strlen("LVDC_CG"))==0){ sscanf(line+strlen("LVDC_CG"),"%lf",&CG);
		} else if (strnicmp(line,"LVDC_cos_chi_Yit",strlen("LVDC_cos_chi_Yit"))==0){ sscanf(line+strlen("LVDC_cos_chi_Yit"),"%lf",&cos_chi_Yit);
		} else if (strnicmp(line,"LVDC_cos_chi_Zit",strlen("LVDC_cos_chi_Zit"))==0){ sscanf(line+strlen("LVDC_cos_chi_Zit"),"%lf",&cos_chi_Zit);
		} else if (strnicmp(line,"LVDC_cos_phi_L",strlen("LVDC_cos_phi_L"))==0){ sscanf(line+strlen("LVDC_cos_phi_L"),"%lf",&cos_phi_L);
		} else if (strnicmp(line,"LVDC_d2",strlen("LVDC_d2"))==0){ sscanf(line+strlen("LVDC_d2"),"%lf",&d2);
		} else if (strnicmp(line,"LVDC_ddot_xi_G",strlen("LVDC_ddot_xi_G"))==0){ sscanf(line+strlen("LVDC_ddot_xi_G"),"%lf",&ddot_xi_G);
		} else if (strnicmp(line,"LVDC_ddot_eta_G",strlen("LVDC_ddot_eta_G"))==0){ sscanf(line+strlen("LVDC_ddot_eta_G"),"%lf",&ddot_eta_G);
		} else if (strnicmp(line,"LVDC_ddot_zeta_G",strlen("LVDC_ddot_zeta_G"))==0){ sscanf(line+strlen("LVDC_ddot_zeta_G"),"%lf",&ddot_zeta_G);
		} else if (strnicmp(line,"LVDC_ddot_zeta_GT",strlen("LVDC_ddot_zeta_GT"))==0){ sscanf(line+strlen("LVDC_ddot_zeta_GT"),"%lf",&ddot_zeta_GT);
		} else if (strnicmp(line,"LVDC_ddot_xi_GT",strlen("LVDC_ddot_xi_GT"))==0){ sscanf(line+strlen("LVDC_ddot_xi_GT"),"%lf",&ddot_xi_GT);
		} else if (strnicmp(line,"LVDC_DescNodeAngle",strlen("LVDC_DescNodeAngle"))==0){ sscanf(line+strlen("LVDC_DescNodeAngle"),"%lf",&DescNodeAngle);
		} else if (strnicmp(line,"LVDC_deta",strlen("LVDC_deta"))==0){ sscanf(line+strlen("LVDC_deta"),"%lf",&deta);
		} else if (strnicmp(line,"LVDC_dxi",strlen("LVDC_dxi"))==0){ sscanf(line+strlen("LVDC_dxi"),"%lf",&dxi);
		}
		if (strnicmp(line,"LVDC_dot_dxi",strlen("LVDC_dot_dxi"))==0){ sscanf(line+strlen("LVDC_dot_dxi"),"%lf",&dot_dxi);
		} else if (strnicmp(line,"LVDC_dot_deta",strlen("LVDC_dot_deta"))==0){ sscanf(line+strlen("LVDC_dot_deta"),"%lf",&dot_deta);
		} else if (strnicmp(line,"LVDC_dot_dzeta",strlen("LVDC_dot_dzeta"))==0){ sscanf(line+strlen("LVDC_dot_dzeta"),"%lf",&dot_dzeta);
		} else if (strnicmp(line,"LVDC_dot_dxit",strlen("LVDC_dot_dxit"))==0){ sscanf(line+strlen("LVDC_dot_dxit"),"%lf",&dot_dxit);
		} else if (strnicmp(line,"LVDC_dot_detat",strlen("LVDC_dot_detat"))==0){ sscanf(line+strlen("LVDC_dot_detat"),"%lf",&dot_detat);
		} else if (strnicmp(line,"LVDC_dot_dzetat",strlen("LVDC_dot_dzetat"))==0){ sscanf(line+strlen("LVDC_dot_dzetat"),"%lf",&dot_dzetat);
		} else if (strnicmp(line,"LVDC_dotM_1",strlen("LVDC_dotM_1"))==0){ sscanf(line+strlen("LVDC_dotM_1"),"%lf",&dotM_1);
		} else if (strnicmp(line,"LVDC_dotM_2",strlen("LVDC_dotM_2"))==0){ sscanf(line+strlen("LVDC_dotM_2"),"%lf",&dotM_2);
		} else if (strnicmp(line,"LVDC_dot_phi_1",strlen("LVDC_dot_phi_1"))==0){ sscanf(line+strlen("LVDC_dot_phi_1"),"%lf",&dot_phi_1);
		} else if (strnicmp(line,"LVDC_dot_phi_T",strlen("LVDC_dot_phi_T"))==0){ sscanf(line+strlen("LVDC_dot_phi_T"),"%lf",&dot_phi_T);
		} else if (strnicmp(line,"LVDC_dot_zeta_T",strlen("LVDC_dot_zeta_T"))==0){ sscanf(line+strlen("LVDC_dot_zeta_T"),"%lf",&dot_zeta_T);
		} else if (strnicmp(line,"LVDC_dot_xi_T",strlen("LVDC_dot_xi_T"))==0){ sscanf(line+strlen("LVDC_dot_xi_T"),"%lf",&dot_xi_T);
		} else if (strnicmp(line,"LVDC_dot_eta_T",strlen("LVDC_dot_eta_T"))==0){ sscanf(line+strlen("LVDC_dot_eta_T"),"%lf",&dot_eta_T);
		} else if (strnicmp(line,"LVDC_dt",strlen("LVDC_dt"))==0){ sscanf(line+strlen("LVDC_dt"),"%lf",&dt);
		} else if (strnicmp(line,"LVDC_dT_2",strlen("LVDC_dT_2"))==0){ sscanf(line+strlen("LVDC_dT_2"),"%lf",&dT_2);
		} else if (strnicmp(line,"LVDC_dt_c",strlen("LVDC_dt_c"))==0){ sscanf(line+strlen("LVDC_dt_c"),"%lf",&dt_c);
		} else if (strnicmp(line,"LVDC_dT_F",strlen("LVDC_dT_F"))==0){ sscanf(line+strlen("LVDC_dT_F"),"%lf",&dT_F);
		} else if (strnicmp(line,"LVDC_dt_LET",strlen("LVDC_dt_LET"))==0){ sscanf(line+strlen("LVDC_dt_LET"),"%lf",&dt_LET);
		} else if (strnicmp(line,"LVDC_dtt_1",strlen("LVDC_dtt_1"))==0){ sscanf(line+strlen("LVDC_dtt_1"),"%lf",&dtt_1);
		} else if (strnicmp(line,"LVDC_dtt_2",strlen("LVDC_dtt_2"))==0){ sscanf(line+strlen("LVDC_dtt_2"),"%lf",&dtt_2);
		} else if (strnicmp(line,"LVDC_dV",strlen("LVDC_dV"))==0){ sscanf(line+strlen("LVDC_dV"),"%lf",&dV);
		} else if (strnicmp(line,"LVDC_dV_B",strlen("LVDC_dV_B"))==0){ sscanf(line+strlen("LVDC_dV_B"),"%lf",&dV_B);
		} else if (strnicmp(line,"LVDC_e",strlen("LVDC_e"))==0){ sscanf(line+strlen("LVDC_e"),"%lf",&e);
		} else if (strnicmp(line,"LVDC_eps_2",strlen("LVDC_eps_2"))==0){ sscanf(line+strlen("LVDC_eps_2"),"%lf",&eps_2);
		} else if (strnicmp(line,"LVDC_eps_3",strlen("LVDC_eps_3"))==0){ sscanf(line+strlen("LVDC_eps_3"),"%lf",&eps_3);
		} else if (strnicmp(line,"LVDC_eps_4",strlen("LVDC_eps_4"))==0){ sscanf(line+strlen("LVDC_eps_4"),"%lf",&eps_4);
		} else if (strnicmp(line,"LVDC_eps_p",strlen("LVDC_eps_p"))==0){ sscanf(line+strlen("LVDC_eps_p"),"%lf",&eps_p);
		} else if (strnicmp(line,"LVDC_eps_ymr",strlen("LVDC_eps_ymr"))==0){ sscanf(line+strlen("LVDC_eps_ymr"),"%lf",&eps_ymr);
		} else if (strnicmp(line,"LVDC_eps_ypr",strlen("LVDC_eps_ypr"))==0){ sscanf(line+strlen("LVDC_eps_ypr"),"%lf",&eps_ypr);
		} else if (strnicmp(line,"LVDC_f",strlen("LVDC_f"))==0){ sscanf(line+strlen("LVDC_f"),"%lf",&f);
		} else if (strnicmp(line,"LVDC_F",strlen("LVDC_F"))==0){ sscanf(line+strlen("LVDC_F"),"%lf",&F);
		} else if (strnicmp(line,"LVDC_Fm",strlen("LVDC_Fm"))==0){ sscanf(line+strlen("LVDC_Fm"),"%lf",&Fm);
		} else if (strnicmp(line,"LVDC_Fx[5][5]",strlen("LVDC_Fx[5][5]"))==0){ sscanf(line+strlen("LVDC_Fx[5][5]"),"%lf",&Fx[5][5]);
		} else if (strnicmp(line,"LVDC_fx[7]",strlen("LVDC_fx[7]"))==0){ sscanf(line+strlen("LVDC_fx[7]"),"%lf",&fx[7]);
		} else if (strnicmp(line,"LVDC_G_T",strlen("LVDC_G_T"))==0){ sscanf(line+strlen("LVDC_G_T"),"%lf",&G_T);
		} else if (strnicmp(line,"LVDC_gx[7]",strlen("LVDC_gx[7]"))==0){ sscanf(line+strlen("LVDC_gx[7]"),"%lf",&gx[7]);
		} else if (strnicmp(line,"LVDC_IGMInterval",strlen("LVDC_IGMInterval"))==0){ sscanf(line+strlen("LVDC_IGMInterval"),"%lf",&IGMInterval);
		} else if (strnicmp(line,"LVDC_Inclination",strlen("LVDC_Inclination"))==0){ sscanf(line+strlen("LVDC_Inclination"),"%lf",&Inclination);
		} else if (strnicmp(line,"LVDC_J",strlen("LVDC_J"))==0){ sscanf(line+strlen("LVDC_J"),"%lf",&J);
		} else if (strnicmp(line,"LVDC_J_1",strlen("LVDC_J_1"))==0){ sscanf(line+strlen("LVDC_J_1"),"%lf",&J_1);
		} else if (strnicmp(line,"LVDC_J_2",strlen("LVDC_J_2"))==0){ sscanf(line+strlen("LVDC_J_2"),"%lf",&J_2);
		} else if (strnicmp(line,"LVDC_Jt_2",strlen("LVDC_Jt_2"))==0){ sscanf(line+strlen("LVDC_Jt_2"),"%lf",&Jt_2);
		} else if (strnicmp(line,"LVDC_J_Y",strlen("LVDC_J_Y"))==0){ sscanf(line+strlen("LVDC_J_Y"),"%lf",&J_Y);
		} else if (strnicmp(line,"LVDC_J_P",strlen("LVDC_J_P"))==0){ sscanf(line+strlen("LVDC_J_P"),"%lf",&J_P);
		} else if (strnicmp(line,"LVDC_K_1",strlen("LVDC_K_1"))==0){ sscanf(line+strlen("LVDC_K_1"),"%lf",&K_1);
		} else if (strnicmp(line,"LVDC_K_2",strlen("LVDC_K_2"))==0){ sscanf(line+strlen("LVDC_K_2"),"%lf",&K_2);
		} else if (strnicmp(line,"LVDC_K_3",strlen("LVDC_K_3"))==0){ sscanf(line+strlen("LVDC_K_3"),"%lf",&K_3);
		} else if (strnicmp(line,"LVDC_K_4",strlen("LVDC_K_4"))==0){ sscanf(line+strlen("LVDC_K_4"),"%lf",&K_4);
		} else if (strnicmp(line,"LVDC_K_5",strlen("LVDC_K_5"))==0){ sscanf(line+strlen("LVDC_K_5"),"%lf",&K_5);
		} else if (strnicmp(line,"LVDC_K_P1",strlen("LVDC_K_P1"))==0){ sscanf(line+strlen("LVDC_K_P1"),"%lf",&K_P1);
		} else if (strnicmp(line,"LVDC_K_P2",strlen("LVDC_K_P2"))==0){ sscanf(line+strlen("LVDC_K_P2"),"%lf",&K_P2);
		} else if (strnicmp(line,"LVDC_K_p",strlen("LVDC_K_p"))==0){ sscanf(line+strlen("LVDC_K_p"),"%lf",&K_p);
		} else if (strnicmp(line,"LVDC_K_y",strlen("LVDC_K_y"))==0){ sscanf(line+strlen("LVDC_K_y"),"%lf",&K_y);
		} else if (strnicmp(line,"LVDC_K_r",strlen("LVDC_K_r"))==0){ sscanf(line+strlen("LVDC_K_r"),"%lf",&K_r);
		}
		if (strnicmp(line,"LVDC_K_Y1",strlen("LVDC_K_Y1"))==0){ sscanf(line+strlen("LVDC_K_Y1"),"%lf",&K_Y1);
		} else if (strnicmp(line,"LVDC_K_Y2",strlen("LVDC_K_Y2"))==0){ sscanf(line+strlen("LVDC_K_Y2"),"%lf",&K_Y2);
		} else if (strnicmp(line,"LVDC_K_Y",strlen("LVDC_K_Y"))==0){ sscanf(line+strlen("LVDC_K_Y"),"%lf",&K_Y);
		} else if (strnicmp(line,"LVDC_K_P",strlen("LVDC_K_P"))==0){ sscanf(line+strlen("LVDC_K_P"),"%lf",&K_P);
		} else if (strnicmp(line,"LVDC_D_P",strlen("LVDC_D_P"))==0){ sscanf(line+strlen("LVDC_D_P"),"%lf",&D_P);
		} else if (strnicmp(line,"LVDC_D_Y",strlen("LVDC_D_Y"))==0){ sscanf(line+strlen("LVDC_D_Y"),"%lf",&D_Y);
		} else if (strnicmp(line,"LVDC_L_1",strlen("LVDC_L_1"))==0){ sscanf(line+strlen("LVDC_L_1"),"%lf",&L_1);
		} else if (strnicmp(line,"LVDC_L_2",strlen("LVDC_L_2"))==0){ sscanf(line+strlen("LVDC_L_2"),"%lf",&L_2);
		} else if (strnicmp(line,"LVDC_dL_2",strlen("LVDC_dL_2"))==0){ sscanf(line+strlen("LVDC_dL_2"),"%lf",&dL_2);
		} else if (strnicmp(line,"LVDC_Lt_2",strlen("LVDC_Lt_2"))==0){ sscanf(line+strlen("LVDC_Lt_2"),"%lf",&Lt_2);
		} else if (strnicmp(line,"LVDC_L_P",strlen("LVDC_L_P"))==0){ sscanf(line+strlen("LVDC_L_P"),"%lf",&L_P);
		} else if (strnicmp(line,"LVDC_L_Y",strlen("LVDC_L_Y"))==0){ sscanf(line+strlen("LVDC_L_Y"),"%lf",&L_Y);
		} else if (strnicmp(line,"LVDC_Lt_Y",strlen("LVDC_Lt_Y"))==0){ sscanf(line+strlen("LVDC_Lt_Y"),"%lf",&Lt_Y);
		} else if (strnicmp(line,"LVDC_LVDC_TB_ETime",strlen("LVDC_LVDC_TB_ETime"))==0){ sscanf(line+strlen("LVDC_LVDC_TB_ETime"),"%lf",&LVDC_TB_ETime);
		} else if (strnicmp(line,"LVDC_mu",strlen("LVDC_mu"))==0){ sscanf(line+strlen("LVDC_mu"),"%lf",&mu);
		} else if (strnicmp(line,"LVDC_p",strlen("LVDC_p"))==0){ sscanf(line+strlen("LVDC_p"),"%lf",&p);
		} else if (strnicmp(line,"LVDC_P_1",strlen("LVDC_P_1"))==0){ sscanf(line+strlen("LVDC_P_1"),"%lf",&P_1);
		} else if (strnicmp(line,"LVDC_P_2",strlen("LVDC_P_2"))==0){ sscanf(line+strlen("LVDC_P_2"),"%lf",&P_2);
		} else if (strnicmp(line,"LVDC_phi_lat",strlen("LVDC_phi_lat"))==0){ sscanf(line+strlen("LVDC_phi_lat"),"%lf",&phi_lat);
		} else if (strnicmp(line,"LVDC_phi_lng",strlen("LVDC_phi_lng"))==0){ sscanf(line+strlen("LVDC_phi_lng"),"%lf",&phi_lng);
		} else if (strnicmp(line,"LVDC_phi_T",strlen("LVDC_phi_T"))==0){ sscanf(line+strlen("LVDC_phi_T"),"%lf",&phi_T);
		} else if (strnicmp(line,"LVDC_Q_1",strlen("LVDC_Q_1"))==0){ sscanf(line+strlen("LVDC_Q_1"),"%lf",&Q_1);
		} else if (strnicmp(line,"LVDC_Q_2",strlen("LVDC_Q_2"))==0){ sscanf(line+strlen("LVDC_Q_2"),"%lf",&Q_2);
		} else if (strnicmp(line,"LVDC_Q_Y",strlen("LVDC_Q_Y"))==0){ sscanf(line+strlen("LVDC_Q_Y"),"%lf",&Q_Y);
		} else if (strnicmp(line,"LVDC_Q_P",strlen("LVDC_Q_P"))==0){ sscanf(line+strlen("LVDC_Q_P"),"%lf",&Q_P);
		} else if (strnicmp(line,"LVDC_R",strlen("LVDC_R"))==0){ sscanf(line+strlen("LVDC_R"),"%lf",&R);
		} else if (strnicmp(line,"LVDC_ROV",strlen("LVDC_ROV"))==0){ sscanf(line+strlen("LVDC_ROV"),"%lf",&ROV);
		} else if (strnicmp(line,"LVDC_ROVs",strlen("LVDC_ROVs"))==0){ sscanf(line+strlen("LVDC_ROVs"),"%lf",&ROVs);
		} else if (strnicmp(line,"LVDC_R_T",strlen("LVDC_R_T"))==0){ sscanf(line+strlen("LVDC_R_T"),"%lf",&R_T);
		} else if (strnicmp(line,"LVDC_S1B_Sep_Time",strlen("LVDC_S1B_Sep_Time"))==0){ sscanf(line+strlen("LVDC_S1B_Sep_Time"),"%lf",&S1B_Sep_Time);
		} else if (strnicmp(line,"LVDC_S_1",strlen("LVDC_S_1"))==0){ sscanf(line+strlen("LVDC_S_1"),"%lf",&S_1);
		} else if (strnicmp(line,"LVDC_S_2",strlen("LVDC_S_2"))==0){ sscanf(line+strlen("LVDC_S_2"),"%lf",&S_2);
		} else if (strnicmp(line,"LVDC_S_P",strlen("LVDC_S_P"))==0){ sscanf(line+strlen("LVDC_S_P"),"%lf",&S_P);
		} else if (strnicmp(line,"LVDC_S_Y",strlen("LVDC_S_Y"))==0){ sscanf(line+strlen("LVDC_S_Y"),"%lf",&S_Y);
		} else if (strnicmp(line,"LVDC_sinceLastIGM",strlen("LVDC_sinceLastIGM"))==0){ sscanf(line+strlen("LVDC_sinceLastIGM"),"%lf",&sinceLastIGM);
		} else if (strnicmp(line,"LVDC_sin_chi_Yit",strlen("LVDC_sin_chi_Yit"))==0){ sscanf(line+strlen("LVDC_sin_chi_Yit"),"%lf",&sin_chi_Yit);
		} else if (strnicmp(line,"LVDC_sin_chi_Zit",strlen("LVDC_sin_chi_Zit"))==0){ sscanf(line+strlen("LVDC_sin_chi_Zit"),"%lf",&sin_chi_Zit);
		} else if (strnicmp(line,"LVDC_sin_phi_L",strlen("LVDC_sin_phi_L"))==0){ sscanf(line+strlen("LVDC_sin_phi_L"),"%lf",&sin_phi_L);
		} else if (strnicmp(line,"LVDC_sin_ups",strlen("LVDC_sin_ups"))==0){ sscanf(line+strlen("LVDC_sin_ups"),"%lf",&sin_ups);
		} else if (strnicmp(line,"LVDC_cos_ups",strlen("LVDC_cos_ups"))==0){ sscanf(line+strlen("LVDC_cos_ups"),"%lf",&cos_ups);
		} else if (strnicmp(line,"LVDC_SMCG",strlen("LVDC_SMCG"))==0){ sscanf(line+strlen("LVDC_SMCG"),"%lf",&SMCG);
		} else if (strnicmp(line,"LVDC_S",strlen("LVDC_S"))==0){ sscanf(line+strlen("LVDC_S"),"%lf",&S);
		} else if (strnicmp(line,"LVDC_P",strlen("LVDC_P"))==0){ sscanf(line+strlen("LVDC_P"),"%lf",&P);
		} else if (strnicmp(line,"LVDC_t",strlen("LVDC_t"))==0){ sscanf(line+strlen("LVDC_t"),"%lf",&t);
		} else if (strnicmp(line,"LVDC_t_1",strlen("LVDC_t_1"))==0){ sscanf(line+strlen("LVDC_t_1"),"%lf",&t_1);
		} else if (strnicmp(line,"LVDC_T_1",strlen("LVDC_T_1"))==0){ sscanf(line+strlen("LVDC_T_1"),"%lf",&T_1);
		} else if (strnicmp(line,"LVDC_t_2",strlen("LVDC_t_2"))==0){ sscanf(line+strlen("LVDC_t_2"),"%lf",&t_2);
		} else if (strnicmp(line,"LVDC_T_2",strlen("LVDC_T_2"))==0){ sscanf(line+strlen("LVDC_T_2"),"%lf",&T_2);
		} else if (strnicmp(line,"LVDC_t_3",strlen("LVDC_t_3"))==0){ sscanf(line+strlen("LVDC_t_3"),"%lf",&t_3);
		} else if (strnicmp(line,"LVDC_t_3i",strlen("LVDC_t_3i"))==0){ sscanf(line+strlen("LVDC_t_3i"),"%lf",&t_3i);
		} else if (strnicmp(line,"LVDC_t_4",strlen("LVDC_t_4"))==0){ sscanf(line+strlen("LVDC_t_4"),"%lf",&t_4);
		} else if (strnicmp(line,"LVDC_t_5",strlen("LVDC_t_5"))==0){ sscanf(line+strlen("LVDC_t_5"),"%lf",&t_5);
		} else if (strnicmp(line,"LVDC_t_6",strlen("LVDC_t_6"))==0){ sscanf(line+strlen("LVDC_t_6"),"%lf",&t_6);
		} else if (strnicmp(line,"LVDC_TA1",strlen("LVDC_TA1"))==0){ sscanf(line+strlen("LVDC_TA1"),"%lf",&TA1);
		}
		if (strnicmp(line,"LVDC_TA2",strlen("LVDC_TA2"))==0){ sscanf(line+strlen("LVDC_TA2"),"%lf",&TA2);
		} else if (strnicmp(line,"LVDC_TA3",strlen("LVDC_TA3"))==0){ sscanf(line+strlen("LVDC_TA3"),"%lf",&TA3);
		} else if (strnicmp(line,"LVDC_TABLE15_C_3",strlen("LVDC_TABLE15_C_3"))==0){ sscanf(line+strlen("LVDC_TABLE15_C_3"),"%lf",&TABLE15_C_3);
		} else if (strnicmp(line,"LVDC_TABLE15_e",strlen("LVDC_TABLE15_e"))==0){ sscanf(line+strlen("LVDC_TABLE15_e"),"%lf",&TABLE15_e);
		} else if (strnicmp(line,"LVDC_TABLE15_f",strlen("LVDC_TABLE15_f"))==0){ sscanf(line+strlen("LVDC_TABLE15_f"),"%lf",&TABLE15_f);
		} else if (strnicmp(line,"LVDC_T_ar",strlen("LVDC_T_ar"))==0){ sscanf(line+strlen("LVDC_T_ar"),"%lf",&T_ar);
		} else if (strnicmp(line,"LVDC_TAS",strlen("LVDC_TAS"))==0){ sscanf(line+strlen("LVDC_TAS"),"%lf",&TAS);
		} else if (strnicmp(line,"LVDC_tau1",strlen("LVDC_tau1"))==0){ sscanf(line+strlen("LVDC_tau1"),"%lf",&tau1);
		} else if (strnicmp(line,"LVDC_tau2",strlen("LVDC_tau2"))==0){ sscanf(line+strlen("LVDC_tau2"),"%lf",&tau2);
		} else if (strnicmp(line,"LVDC_t_B1",strlen("LVDC_t_B1"))==0){ sscanf(line+strlen("LVDC_t_B1"),"%lf",&t_B1);
		} else if (strnicmp(line,"LVDC_t_B3",strlen("LVDC_t_B3"))==0){ sscanf(line+strlen("LVDC_t_B3"),"%lf",&t_B3);
		} else if (strnicmp(line,"LVDC_tchi_y_last",strlen("LVDC_tchi_y_last"))==0){ sscanf(line+strlen("LVDC_tchi_y_last"),"%lf",&tchi_y_last);
		} else if (strnicmp(line,"LVDC_tchi_p_last",strlen("LVDC_tchi_p_last"))==0){ sscanf(line+strlen("LVDC_tchi_p_last"),"%lf",&tchi_p_last);
		} else if (strnicmp(line,"LVDC_tchi_y",strlen("LVDC_tchi_y"))==0){ sscanf(line+strlen("LVDC_tchi_y"),"%lf",&tchi_y);
		} else if (strnicmp(line,"LVDC_tchi_p",strlen("LVDC_tchi_p"))==0){ sscanf(line+strlen("LVDC_tchi_p"),"%lf",&tchi_p);
		} else if (strnicmp(line,"LVDC_t_clock",strlen("LVDC_t_clock"))==0){ sscanf(line+strlen("LVDC_t_clock"),"%lf",&t_clock);
		} else if (strnicmp(line,"LVDC_T_CO",strlen("LVDC_T_CO"))==0){ sscanf(line+strlen("LVDC_T_CO"),"%lf",&T_CO);
		} else if (strnicmp(line,"LVDC_t_fail",strlen("LVDC_t_fail"))==0){ sscanf(line+strlen("LVDC_t_fail"),"%lf",&t_fail);
		} else if (strnicmp(line,"LVDC_T_GO",strlen("LVDC_T_GO"))==0){ sscanf(line+strlen("LVDC_T_GO"),"%lf",&T_GO);
		} else if (strnicmp(line,"LVDC_T_LET",strlen("LVDC_T_LET"))==0){ sscanf(line+strlen("LVDC_T_LET"),"%lf",&T_LET);
		} else if (strnicmp(line,"LVDC_T_S1",strlen("LVDC_T_S1"))==0){ sscanf(line+strlen("LVDC_T_S1"),"%lf",&T_S1);
		} else if (strnicmp(line,"LVDC_T_S2",strlen("LVDC_T_S2"))==0){ sscanf(line+strlen("LVDC_T_S2"),"%lf",&T_S2);
		} else if (strnicmp(line,"LVDC_T_S3",strlen("LVDC_T_S3"))==0){ sscanf(line+strlen("LVDC_T_S3"),"%lf",&T_S3);
		} else if (strnicmp(line,"LVDC_TSMC1",strlen("LVDC_TSMC1"))==0){ sscanf(line+strlen("LVDC_TSMC1"),"%lf",&TSMC1);
		} else if (strnicmp(line,"LVDC_TSMC2",strlen("LVDC_TSMC2"))==0){ sscanf(line+strlen("LVDC_TSMC2"),"%lf",&TSMC2);
		} else if (strnicmp(line,"LVDC_T_T",strlen("LVDC_T_T"))==0){ sscanf(line+strlen("LVDC_T_T"),"%lf",&T_T);
		} else if (strnicmp(line,"LVDC_Tt_2",strlen("LVDC_Tt_2"))==0){ sscanf(line+strlen("LVDC_Tt_2"),"%lf",&Tt_2);
		} else if (strnicmp(line,"LVDC_Tt_T",strlen("LVDC_Tt_T"))==0){ sscanf(line+strlen("LVDC_Tt_T"),"%lf",&Tt_T);
		} else if (strnicmp(line,"LVDC_U_1",strlen("LVDC_U_1"))==0){ sscanf(line+strlen("LVDC_U_1"),"%lf",&U_1);
		} else if (strnicmp(line,"LVDC_U_2",strlen("LVDC_U_2"))==0){ sscanf(line+strlen("LVDC_U_2"),"%lf",&U_2);
		} else if (strnicmp(line,"LVDC_ups_T",strlen("LVDC_ups_T"))==0){ sscanf(line+strlen("LVDC_ups_T"),"%lf",&ups_T);
		} else if (strnicmp(line,"LVDC_V",strlen("LVDC_V"))==0){ sscanf(line+strlen("LVDC_V"),"%lf",&V);
		} else if (strnicmp(line,"LVDC_V_ex1",strlen("LVDC_V_ex1"))==0){ sscanf(line+strlen("LVDC_V_ex1"),"%lf",&V_ex1);
		} else if (strnicmp(line,"LVDC_V_ex2",strlen("LVDC_V_ex2"))==0){ sscanf(line+strlen("LVDC_V_ex2"),"%lf",&V_ex2);
		} else if (strnicmp(line,"LVDC_V_i",strlen("LVDC_V_i"))==0){ sscanf(line+strlen("LVDC_V_i"),"%lf",&V_i);
		} else if (strnicmp(line,"LVDC_V_0",strlen("LVDC_V_0"))==0){ sscanf(line+strlen("LVDC_V_0"),"%lf",&V_0);
		} else if (strnicmp(line,"LVDC_V_1",strlen("LVDC_V_1"))==0){ sscanf(line+strlen("LVDC_V_1"),"%lf",&V_1);
		} else if (strnicmp(line,"LVDC_V_2",strlen("LVDC_V_2"))==0){ sscanf(line+strlen("LVDC_V_2"),"%lf",&V_2);
		} else if (strnicmp(line,"LVDC_V_T",strlen("LVDC_V_T"))==0){ sscanf(line+strlen("LVDC_V_T"),"%lf",&V_T);
		} else if (strnicmp(line,"LVDC_V_TC",strlen("LVDC_V_TC"))==0){ sscanf(line+strlen("LVDC_V_TC"),"%lf",&V_TC);
		} else if (strnicmp(line,"LVDC_xi_T",strlen("LVDC_xi_T"))==0){ sscanf(line+strlen("LVDC_xi_T"),"%lf",&xi_T);
		} else if (strnicmp(line,"LVDC_eta_T",strlen("LVDC_eta_T"))==0){ sscanf(line+strlen("LVDC_eta_T"),"%lf",&eta_T);
		} else if (strnicmp(line,"LVDC_zeta_T",strlen("LVDC_zeta_T"))==0){ sscanf(line+strlen("LVDC_zeta_T"),"%lf",&zeta_T);
		} else if (strnicmp(line,"LVDC_X_S1",strlen("LVDC_X_S1"))==0){ sscanf(line+strlen("LVDC_X_S1"),"%lf",&X_S1);
		} else if (strnicmp(line,"LVDC_X_S2",strlen("LVDC_X_S2"))==0){ sscanf(line+strlen("LVDC_X_S2"),"%lf",&X_S2);
		} else if (strnicmp(line,"LVDC_X_S3",strlen("LVDC_X_S3"))==0){ sscanf(line+strlen("LVDC_X_S3"),"%lf",&X_S3);
		} else if (strnicmp(line,"LVDC_Xtt_y",strlen("LVDC_Xtt_y"))==0){ sscanf(line+strlen("LVDC_Xtt_y"),"%lf",&Xtt_y);
		} else if (strnicmp(line,"LVDC_Xtt_p",strlen("LVDC_Xtt_p"))==0){ sscanf(line+strlen("LVDC_Xtt_p"),"%lf",&Xtt_p);
		} else if (strnicmp(line,"LVDC_X_Zi",strlen("LVDC_X_Zi"))==0){ sscanf(line+strlen("LVDC_X_Zi"),"%lf",&X_Zi);
		} else if (strnicmp(line,"LVDC_X_Yi",strlen("LVDC_X_Yi"))==0){ sscanf(line+strlen("LVDC_X_Yi"),"%lf",&X_Yi);
		} else if (strnicmp(line,"LVDC_Y_u",strlen("LVDC_Y_u"))==0){ sscanf(line+strlen("LVDC_Y_u"),"%lf",&Y_u);
		}
		// VECTOR3
		if (strnicmp(line,"LVDC_ACommandedAttitude",strlen("LVDC_ACommandedAttitude"))==0){ sscanf(line+strlen("LVDC_ACommandedAttitude"),"%lf %lf %lf",&ACommandedAttitude.x,&ACommandedAttitude.y,&ACommandedAttitude.z);
		} else if (strnicmp(line,"LVDC_AttitudeError",strlen("LVDC_AttitudeError"))==0){ sscanf(line+strlen("LVDC_AttitudeError"),"%lf %lf %lf",&AttitudeError.x,&AttitudeError.y,&AttitudeError.z);
		} else if (strnicmp(line,"LVDC_AttRate",strlen("LVDC_AttRate"))==0){ sscanf(line+strlen("LVDC_AttRate"),"%lf %lf %lf",&AttRate.x,&AttRate.y,&AttRate.z);
		} else if (strnicmp(line,"LVDC_CommandedAttitude",strlen("LVDC_CommandedAttitude"))==0){ sscanf(line+strlen("LVDC_CommandedAttitude"),"%lf %lf %lf",&CommandedAttitude.x,&CommandedAttitude.y,&CommandedAttitude.z);
		} else if (strnicmp(line,"LVDC_CommandRateLimits",strlen("LVDC_CommandRateLimits"))==0){ sscanf(line+strlen("LVDC_CommandRateLimits"),"%lf %lf %lf",&CommandRateLimits.x,&CommandRateLimits.y,&CommandRateLimits.z);
		} else if (strnicmp(line,"LVDC_CurrentAttitude",strlen("LVDC_CurrentAttitude"))==0){ sscanf(line+strlen("LVDC_CurrentAttitude"),"%lf %lf %lf",&CurrentAttitude.x,&CurrentAttitude.y,&CurrentAttitude.z);
		} else if (strnicmp(line,"LVDC_ddotG_act",strlen("LVDC_ddotG_act"))==0){ sscanf(line+strlen("LVDC_ddotG_act"),"%lf %lf %lf",&ddotG_act.x,&ddotG_act.y,&ddotG_act.z);
		} else if (strnicmp(line,"LVDC_ddotG_last",strlen("LVDC_ddotG_last"))==0){ sscanf(line+strlen("LVDC_ddotG_last"),"%lf %lf %lf",&ddotG_last.x,&ddotG_last.y,&ddotG_last.z);
		} else if (strnicmp(line,"LVDC_ddotM_act",strlen("LVDC_ddotM_act"))==0){ sscanf(line+strlen("LVDC_ddotM_act"),"%lf %lf %lf",&ddotM_act.x,&ddotM_act.y,&ddotM_act.z);
		} else if (strnicmp(line,"LVDC_ddotM_last",strlen("LVDC_ddotM_last"))==0){ sscanf(line+strlen("LVDC_ddotM_last"),"%lf %lf %lf",&ddotM_last.x,&ddotM_last.y,&ddotM_last.z);
		} else if (strnicmp(line,"LVDC_DDotXEZ_G",strlen("LVDC_DDotXEZ_G"))==0){ sscanf(line+strlen("LVDC_DDotXEZ_G"),"%lf %lf %lf",&DDotXEZ_G.x,&DDotXEZ_G.y,&DDotXEZ_G.z);
		} else if (strnicmp(line,"LVDC_DeltaAtt",strlen("LVDC_DeltaAtt"))==0){ sscanf(line+strlen("LVDC_DeltaAtt"),"%lf %lf %lf",&DeltaAtt.x,&DeltaAtt.y,&DeltaAtt.z);
		} else if (strnicmp(line,"LVDC_Dot0",strlen("LVDC_Dot0"))==0){ sscanf(line+strlen("LVDC_Dot0"),"%lf %lf %lf",&Dot0.x,&Dot0.y,&Dot0.z);
		} else if (strnicmp(line,"LVDC_DotG_act",strlen("LVDC_DotG_act"))==0){ sscanf(line+strlen("LVDC_DotG_act"),"%lf %lf %lf",&DotG_act.x,&DotG_act.y,&DotG_act.z);
		} else if (strnicmp(line,"LVDC_DotG_last",strlen("LVDC_DotG_last"))==0){ sscanf(line+strlen("LVDC_DotG_last"),"%lf %lf %lf",&DotG_last.x,&DotG_last.y,&DotG_last.z);
		} else if (strnicmp(line,"LVDC_DotM_act",strlen("LVDC_DotM_act"))==0){ sscanf(line+strlen("LVDC_DotM_act"),"%lf %lf %lf",&DotM_act.x,&DotM_act.y,&DotM_act.z);
		} else if (strnicmp(line,"LVDC_DotM_last",strlen("LVDC_DotM_last"))==0){ sscanf(line+strlen("LVDC_DotM_last"),"%lf %lf %lf",&DotM_last.x,&DotM_last.y,&DotM_last.z);
		} else if (strnicmp(line,"LVDC_DotS",strlen("LVDC_DotS"))==0){ sscanf(line+strlen("LVDC_DotS"),"%lf %lf %lf",&DotS.x,&DotS.y,&DotS.z);
		} else if (strnicmp(line,"LVDC_DotXEZ",strlen("LVDC_DotXEZ"))==0){ sscanf(line+strlen("LVDC_DotXEZ"),"%lf %lf %lf",&DotXEZ.x,&DotXEZ.y,&DotXEZ.z);
		} else if (strnicmp(line,"LVDC_PCommandedAttitude",strlen("LVDC_PCommandedAttitude"))==0){ sscanf(line+strlen("LVDC_PCommandedAttitude"),"%lf %lf %lf",&PCommandedAttitude.x,&PCommandedAttitude.y,&PCommandedAttitude.z);
		} else if (strnicmp(line,"LVDC_Pos4",strlen("LVDC_Pos4"))==0){ sscanf(line+strlen("LVDC_Pos4"),"%lf %lf %lf",&Pos4.x,&Pos4.y,&Pos4.z);
		} else if (strnicmp(line,"LVDC_PosS",strlen("LVDC_PosS"))==0){ sscanf(line+strlen("LVDC_PosS"),"%lf %lf %lf",&PosS.x,&PosS.y,&PosS.z);
		} else if (strnicmp(line,"LVDC_PosXEZ",strlen("LVDC_PosXEZ"))==0){ sscanf(line+strlen("LVDC_PosXEZ"),"%lf %lf %lf",&PosXEZ.x,&PosXEZ.y,&PosXEZ.z);
		}
		// MATRIX3
		if (strnicmp(line,"LVDC_MX_A",strlen("LVDC_MX_A"))==0){ sscanf(line+strlen("LVDC_MX_A"),"%lf %lf %lf %lf %lf %lf %lf %lf %lf",&MX_A.m11,&MX_A.m12,&MX_A.m13,&MX_A.m21,&MX_A.m22,&MX_A.m23,&MX_A.m31,&MX_A.m32,&MX_A.m33);
		} else if (strnicmp(line,"LVDC_MX_B",strlen("LVDC_MX_B"))==0){ sscanf(line+strlen("LVDC_MX_B"),"%lf %lf %lf %lf %lf %lf %lf %lf %lf",&MX_B.m11,&MX_B.m12,&MX_B.m13,&MX_B.m21,&MX_B.m22,&MX_B.m23,&MX_B.m31,&MX_B.m32,&MX_B.m33);
		} else if (strnicmp(line,"LVDC_MX_G",strlen("LVDC_MX_G"))==0){ sscanf(line+strlen("LVDC_MX_G"),"%lf %lf %lf %lf %lf %lf %lf %lf %lf",&MX_G.m11,&MX_G.m12,&MX_G.m13,&MX_G.m21,&MX_G.m22,&MX_G.m23,&MX_G.m31,&MX_G.m32,&MX_G.m33);
		} else if (strnicmp(line,"LVDC_MX_K",strlen("LVDC_MX_K"))==0){ sscanf(line+strlen("LVDC_MX_K"),"%lf %lf %lf %lf %lf %lf %lf %lf %lf",&MX_K.m11,&MX_K.m12,&MX_K.m13,&MX_K.m21,&MX_K.m22,&MX_K.m23,&MX_K.m31,&MX_K.m32,&MX_K.m33);
		} else if (strnicmp(line,"LVDC_MX_phi_T",strlen("LVDC_MX_phi_T"))==0){ sscanf(line+strlen("LVDC_MX_phi_T"),"%lf %lf %lf %lf %lf %lf %lf %lf %lf",&MX_phi_T.m11,&MX_phi_T.m12,&MX_phi_T.m13,&MX_phi_T.m21,&MX_phi_T.m22,&MX_phi_T.m23,&MX_phi_T.m31,&MX_phi_T.m32,&MX_phi_T.m33);
		}
	}	
	if(oapiReadScenario_nextline (scn, line)){
		if (!strnicmp(line, LVIMU_START_STRING, sizeof(LVIMU_START_STRING))) {
			lvimu.LoadState(scn);
		}
	}
	return;
}