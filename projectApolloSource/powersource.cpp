/***************************************************************************
  This file is part of Project Apollo - NASSP
  Copyright 2004-2005 Mark Grant

  ORBITER vessel module: Power distribution code

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
  *	Revision 1.13  2006/07/21 23:04:34  movieman523
  *	Added Saturn 1b engine lights on panel and beginnings of electrical connector work (couldn't disentangle the changes). Be sure to get the config file for the SIVb as well.
  *	
  *	Revision 1.12  2006/06/21 12:54:12  tschachim
  *	Bugfix.
  *	
  *	Revision 1.11  2006/05/30 14:40:21  tschachim
  *	Fixed fuel cell - dc bus connectivity, added battery charger
  *	
  *	Revision 1.10  2006/01/14 20:58:16  movieman523
  *	Revised PowerSource code to ensure that classes which must be called each timestep are registered with the Panel SDK code.
  *	
  *	Revision 1.9  2006/01/10 19:34:44  movieman523
  *	Fixed AC bus switches and added ELS Logic/Auto support.
  *	
  *	Revision 1.8  2006/01/08 19:04:29  movieman523
  *	Wired up AC bus switches in a quick and hacky manner.
  *	
  *	Revision 1.7  2005/12/02 19:29:24  movieman523
  *	Started integrating PowerSource code into PanelSDK.
  *	
  *	Revision 1.6  2005/11/18 20:38:59  movieman523
  *	Revised condensor output from fuel cell to eliminate master alarms.
  *	
  *	Revision 1.5  2005/11/18 02:40:55  movieman523
  *	Major revamp of PanelSDK electrical code, and various modifications to run off fuel cells.
  *	
  *	Revision 1.4  2005/11/17 21:04:52  movieman523
  *	IMU and AGC now start powered-down. Revised battery code, and wired up all batteries in CSM.
  *	
  *	Revision 1.3  2005/11/17 19:19:12  movieman523
  *	Added three-phase AC bus and battery buses.
  *	
  *	Revision 1.2  2005/11/17 01:52:29  movieman523
  *	Simplified setup for circuit breakers, and added battery buses.
  *	
  *	Revision 1.1  2005/11/16 23:14:02  movieman523
  *	Initial support for wiring in the circuit breakers.
  *	
  **************************************************************************/


#include <stdio.h>
#include "orbitersdk.h"

#include "PanelSDK/PanelSDK.h"
#include "PanelSDK/Internals/Esystems.h"

#include "powersource.h"
#include "connector.h"

PowerSource::PowerSource()

{
	SRC = 0;
}

PowerSource::~PowerSource()

{
	// Nothing for now.
}

double PowerSource::Current()

{
	if (SRC)
		return SRC->Current();

	return 0.0;
}

//
// Default to passing calls on.
//

double PowerSource::Voltage()

{
	if (SRC)
		return SRC->Voltage();

	return 0.0;
}

double PowerBreaker::Voltage()

{
	if (!IsOpen() && SRC)
		return SRC->Voltage();

	return 0.0;
}

double PowerSDKObject::Voltage()

{
	if (SDKObj)
		return SDKObj->Voltage();

	return 0.0;
}

double PowerSDKObject::Current()

{
	if (SDKObj)
		return SDKObj->Current();

	return 0.0;
}

double PowerSDKObject::PowerLoad()

{
	if (SDKObj)
		return SDKObj->PowerLoad();

	return 0.0;
}

void PowerSDKObject::DrawPower(double watts)

{
	if (SDKObj)
		SDKObj->DrawPower(watts);
}

//
// Tie power together from two sources. 
//

PowerMerge::PowerMerge(char *i_name, PanelSDK &p) : sdk(p)

{
	if (i_name)
		strcpy(name, i_name);

	BusA = 0; 
	BusB = 0;

	sdk.AddElectrical(this, false);
}

double PowerMerge::Voltage()

{
	double VoltsA = 0;
	double VoltsB = 0;

	if (BusA) VoltsA = BusA->Voltage();
	if (BusB) VoltsB = BusB->Voltage();

	if (VoltsA != 0 && VoltsB != 0) return (VoltsA + VoltsB) / 2.0;
	if (VoltsA != 0) return VoltsA;
	if (VoltsB != 0) return VoltsB;

	return 0;
}

double PowerMerge::Current()

{
	double Volts = Voltage();

	if (Volts > 0.0) {
		return power_load / Volts;
	}
	return 0.0;
}

void PowerMerge::DrawPower(double watts)

{
	double Volts = 0.0;
	double VoltsA = 0.0;
	double VoltsB = 0.0;

	power_load += watts;

	if (BusA)
		VoltsA = BusA->Voltage();
	if (BusB)
		VoltsB = BusB->Voltage();

	Volts = VoltsA + VoltsB;

	if (Volts > 0.0) {
		if (BusA)
			BusA->DrawPower(watts * VoltsA / Volts);
		if (BusB)
			BusB->DrawPower(watts * VoltsB / Volts);
	}
}

//
// Tie power together from three sources. 
//

double ThreeWayPowerMerge::Voltage()

{
	double Volts1 = 0;
	double Volts2 = 0;
	double Volts3 = 0;

	if (Phase1)	Volts1 = Phase1->Voltage();
	if (Phase2)	Volts2 = Phase2->Voltage();
	if (Phase3)	Volts3 = Phase3->Voltage();

	if (Volts1 != 0 && Volts2 != 0 && Volts3 != 0) return (Volts1 + Volts2 + Volts3) / 3.0;

	if (Volts1 != 0 && Volts2 != 0) return (Volts1 + Volts2) / 2.0;
	if (Volts1 != 0 && Volts3 != 0) return (Volts1 + Volts3) / 2.0;
	if (Volts2 != 0 && Volts3 != 0) return (Volts2 + Volts3) / 2.0;

	if (Volts1 != 0) return Volts1;
	if (Volts2 != 0) return Volts2;
	if (Volts3 != 0) return Volts3;

	return 0;
}

double ThreeWayPowerMerge::Current()

{
	double Volts = Voltage();

	if (Volts > 0.0) {
		return power_load / Volts;
	}
	return 0.0;
}

ThreeWayPowerMerge::ThreeWayPowerMerge(char *i_name, PanelSDK &p) : sdk(p)

{
	if (i_name)
		strcpy (name, i_name);

	Phase1 = 0;
	Phase2 = 0;
	Phase3 = 0;

	//
	// Register with the Panel SDK so it will call our update function.
	//

	sdk.AddElectrical(this, false);
}

void ThreeWayPowerMerge::DrawPower(double watts)

{
	double Volts = 0.0;
	double VoltsA = 0.0;
	double VoltsB = 0.0;
	double VoltsC = 0.0;

	power_load += watts;

	if (Phase1)
		VoltsA = Phase1->Voltage();
	if (Phase2)
		VoltsB = Phase2->Voltage();
	if (Phase3)
		VoltsC = Phase3->Voltage();

	Volts = VoltsA + VoltsB + VoltsC;

	if (Volts > 0.0) {
		if (Phase1)
			Phase1->DrawPower(watts * VoltsA / Volts);
		if (Phase2)
			Phase2->DrawPower(watts * VoltsB / Volts);
		if (Phase3)
			Phase3->DrawPower(watts * VoltsC / Volts);
	}
}

void ThreeWayPowerMerge::WireToBus(int bus, e_object* e)

{
	if (bus == 1) Phase1 = e;
	if (bus == 2) Phase2 = e;
	if (bus == 3) Phase3 = e;
}

bool ThreeWayPowerMerge::IsBusConnected(int bus)

{
	if (bus == 1) return (Phase1 != NULL);
	if (bus == 2) return (Phase2 != NULL);
	if (bus == 3) return (Phase3 != NULL);
	return false;
}

NWayPowerMerge::NWayPowerMerge(char *i_name, PanelSDK &p, int n) : sdk(p)

{
	if (i_name)
		strcpy (name, i_name);

	nSources = n;
	sources = new e_object *[nSources];

	int i;

	for (i = 0; i < nSources; i++)
	{
		sources[i] = 0;
	}

	//
	// Register with the Panel SDK so it will call our update function.
	//

	sdk.AddElectrical(this, false);
}

NWayPowerMerge::~NWayPowerMerge()

{
	if (sources)
	{
		delete[] sources;
		sources = 0;
	}
}

double NWayPowerMerge::Voltage()

{
	double V = 0;

	int i;
	int activeSources = 0;

	for (i = 0; i < nSources; i++)
	{
		if (sources[i])
		{
			double VS = fabs(sources[i]->Voltage());
			if (VS != 0)
			{
				V += VS;
				activeSources++;
			}
		}
	}

	if (!activeSources)
		return 0.0;

	return V / (double) activeSources;
}

double NWayPowerMerge::Current()

{
	double Volts = Voltage();

	if (Volts > 0.0)
	{
		return power_load / Volts;
	}
	return 0.0;
}

void NWayPowerMerge::DrawPower(double watts)

{
	double Volts = 0.0;
	int i;

	//
	// Sum the voltage from all sources.
	//
	for (i = 0; i < nSources; i++)
	{
		if (sources[i])
		{
			double VS = fabs(sources[i]->Voltage());
			if (VS != 0)
			{
				Volts += VS;
			}
		}
	}

	power_load += watts;

	//
	// Divide the power drain up across the sources by voltage.
	//
	if (Volts > 0.0)
	{
		int i;

		for (i = 0; i < nSources; i++)
		{
			if (sources[i])
			{
				sources[i]->DrawPower(watts * fabs(sources[i]->Voltage()) / Volts);
			}
		}
	}
}

bool NWayPowerMerge::IsBusConnected(int bus)

{
	if (bus < 1 || bus > nSources)
		return false;

	return (sources[bus - 1] != 0);
}

void NWayPowerMerge::WireToBus(int bus, e_object* e)

{
	if (bus > 0 && bus <= nSources)
		sources[bus - 1] = e;
}

DCBusController::DCBusController(char *i_name, PanelSDK &p) : 
	sdk(p), fcPower(0, p), batPower(0, p), busPower(0, p, 4)

{
	if (i_name)
		strcpy(name, i_name);

	fuelcell1 = 0;
	fuelcell2 = 0;
	fuelcell3 = 0;
	battery1 = 0;
	battery2 = 0;
	gseBattery = 0;
	fcDisconnectAlarm[1] = false;
	fcDisconnectAlarm[2] = false;
	fcDisconnectAlarm[3] = false;
	tieState = 0;
	gseState = 0;

	sdk.AddElectrical(this, false);
}

void DCBusController::Init(e_object *fc1, e_object *fc2, e_object *fc3, e_object *bat1, e_object *bat2, e_object *gse, e_object *vp)

{
	fuelcell1 = fc1;
	fuelcell2 = fc2;
	fuelcell3 = fc3;

	battery1 = bat1;
	battery2 = bat2;
	gseBattery = gse;

	busPower.WireToBus(1, &fcPower);
	busPower.WireToBus(2, &batPower);
	busPower.WireToBus(3, NULL);		// Source 3 is for ground power.
	busPower.WireToBus(4, vp);			// Source 4 is for docked vessel power.
}

void DCBusController::ConnectFuelCell(int fc, bool connect)

{
	if (connect) {
		if (fc == 1) {
			fcPower.WireToBus(1, fuelcell1);
		} else if (fc == 2) {
			fcPower.WireToBus(2, fuelcell2);
		} else if (fc == 3) {
			fcPower.WireToBus(3, fuelcell3);
		}
	} else {
		if (fc == 1) {
			fcPower.WireToBus(1, NULL);
		} else if (fc == 2) {
			fcPower.WireToBus(2, NULL);
		} else if (fc == 3) {
			fcPower.WireToBus(3, NULL);
		}
	}
	fcDisconnectAlarm[fc] = false;
}

void DCBusController::refresh(double dt)

{
	// Disconnect because of to high current
	if (fuelcell1->Current() > 75.0 && fcPower.IsBusConnected(1)) {
		fcPower.WireToBus(1, NULL);
		fcDisconnectAlarm[1] = true;
	}
	if (fuelcell2->Current() > 75.0 && fcPower.IsBusConnected(2)) {
		fcPower.WireToBus(2, NULL);
		fcDisconnectAlarm[2] = true;
	}
	if (fuelcell3->Current() > 75.0 && fcPower.IsBusConnected(3)) {
		fcPower.WireToBus(3, NULL);
		fcDisconnectAlarm[3] = true;
	}

	// Disconnect because of reverse current
	if (busPower.Voltage() > 0) {
		if (fuelcell1->Voltage() <= 0 && fcPower.IsBusConnected(1)) {
			fcPower.WireToBus(1, NULL);
			fcDisconnectAlarm[1] = true;
		}
		if (fuelcell2->Voltage() <= 0 && fcPower.IsBusConnected(2)) {
			fcPower.WireToBus(2, NULL);
			fcDisconnectAlarm[2] = true;
		}
		if (fuelcell3->Voltage() <= 0 && fcPower.IsBusConnected(3)) {
			fcPower.WireToBus(3, NULL);
			fcDisconnectAlarm[3] = true;
		}
	}

	// Main bus tie
	if (tieState == 2) {
		batPower.WireToBuses(battery1, battery2);

	} else if (tieState == 1) {
		// Auto
		if (fcPower.Voltage() > 0 || (gseState && gseBattery->Voltage() > 0)) {
			batPower.WireToBuses(NULL, NULL);
		} else {
			batPower.WireToBuses(battery1, battery2);
		}

	} else {
		batPower.WireToBuses(NULL, NULL);
	}
}

bool DCBusController::IsFuelCellConnected(int fc)

{
	return fcPower.IsBusConnected(fc);
}

bool DCBusController::IsFuelCellDisconnectAlarm()

{
	return fcDisconnectAlarm[1] || fcDisconnectAlarm[2] || fcDisconnectAlarm[3];
}

void DCBusController::SetGSEState(int s) 

{ 
	gseState = s; 
	if (gseState)
		busPower.WireToBus(3, gseBattery);
	else
		busPower.WireToBus(3, NULL);
}

void DCBusController::Load(char *line)

{
	int fc1, fc2, fc3, gse;
	
	sscanf (line,"    <DCBUSCONTROLLER> %s %i %i %i %i %i", name, &fc1, &fc2, &fc3, &tieState, &gse);
	if (fc1) fcPower.WireToBus(1, fuelcell1);
	if (fc2) fcPower.WireToBus(2, fuelcell2);
	if (fc3) fcPower.WireToBus(3, fuelcell3);
	// tieState is evaluated in refresh()	
	SetGSEState(gse);
}

void DCBusController::Save(FILEHANDLE scn)

{
	char cbuf[1000];
	sprintf (cbuf, "%s %i %i %i %i %i", name, IsFuelCellConnected(1) ? 1 : 0, 
								  		      IsFuelCellConnected(2) ? 1 : 0, 
										      IsFuelCellConnected(3) ? 1 : 0,
										      tieState, gseState);

	oapiWriteScenario_string (scn, "    <DCBUSCONTROLLER> ", cbuf);
}


BatteryCharger::BatteryCharger(char *i_name, PanelSDK &p) : 
	sdk(p), dcPower(0, p)

{
	if (i_name)
		strcpy(name, i_name);

	SRC = NULL;

	battery1 = NULL;
	battery2 = NULL;
	battery3 = NULL;
	batSupply1 = NULL;
	batSupply2 = NULL;
	batSupply3 = NULL;
	currentBattery = NULL;
	acPower = NULL;

	sdk.AddElectrical(this, false);
}

void BatteryCharger::Init(e_object *bat1, e_object *bat2, e_object *bat3, 
					      e_object *batSup1, e_object *batSup2, e_object *batSup3,
			              e_object *dc1, e_object *dc2, e_object *ac)
{
	battery1 = bat1;
	battery2 = bat2;
	battery3 = bat3;

	batSupply1 = batSup1;
	batSupply2 = batSup2;
	batSupply3 = batSup3;

	dcPower.WireToBuses(dc1, dc2);
	acPower = ac;
}

void BatteryCharger::Charge(int bat)

{
	if (bat == 0) {
		batSupply1->WireTo(NULL);
		batSupply2->WireTo(NULL);
		batSupply3->WireTo(NULL);
		currentBattery = NULL;

	} else if (bat == 1) {
		batSupply1->WireTo(this);
		batSupply2->WireTo(NULL);
		batSupply3->WireTo(NULL);
		currentBattery = battery1;

	} else if (bat == 2) {
		batSupply1->WireTo(NULL);
		batSupply2->WireTo(this);
		batSupply3->WireTo(NULL);
		currentBattery = battery2;

	} else if (bat == 3) {
		batSupply1->WireTo(NULL);
		batSupply2->WireTo(NULL);
		batSupply3->WireTo(this);
		currentBattery = battery3;
	}
}

void BatteryCharger::UpdateFlow(double dt)

{
	e_object::UpdateFlow(dt);

	if (currentBattery && dcPower.Voltage() > SP_MIN_DCVOLTAGE && acPower->Voltage() > SP_MIN_ACVOLTAGE) {
		Volts = max(10.0, currentBattery->Voltage());
	} else {
		Volts = 0;
		Amperes = 0;
	}
}

void BatteryCharger::DrawPower(double watts)
{ 
	power_load += watts;
	dcPower.DrawPower(1.4 * watts);
	acPower->DrawPower(0.92 * watts);
}

void BatteryCharger::Load(char *line)

{
	int bat;
	
	sscanf (line,"    <BATTERYCHARGER> %s %i", name, &bat);
	Charge(bat);
}

void BatteryCharger::Save(FILEHANDLE scn)

{
	char cbuf[1000];

	int bat;

	bat = 0;
	if (currentBattery == battery1) bat = 1;
	if (currentBattery == battery2) bat = 2;
	if (currentBattery == battery3) bat = 3;

	sprintf (cbuf, "%s %i", name, bat);
	oapiWriteScenario_string (scn, "    <BATTERYCHARGER> ", cbuf);
}

PowerSourceConnectorObject::PowerSourceConnectorObject(char *i_name, PanelSDK &sdk)

{
	if (i_name)
		strcpy (name, i_name);

	connect = 0;
	sdk.AddElectrical(this, false);
}

double PowerSourceConnectorObject::Voltage()

{
	if (connect)
	{
		ConnectorMessage cm;

		cm.destination = connect->GetType();
		cm.messageType = POWERCON_GET_VOLTAGE;

		if (connect->SendMessage(cm))
		{
			return cm.val1.dValue;
		}
	}	

	return 0.0;
}

double PowerSourceConnectorObject::Current()

{
	if (connect)
	{
		ConnectorMessage cm;

		cm.destination = connect->GetType();
		cm.messageType = POWERCON_GET_CURRENT;

		if (connect->SendMessage(cm))
		{
			return cm.val1.dValue;
		}
	}	

	return 0.0;
}

void PowerSourceConnectorObject::DrawPower(double watts)

{
	if (connect)
	{
		ConnectorMessage cm;

		cm.destination = connect->GetType();
		cm.messageType = POWERCON_DRAW_POWER;

		cm.val1.dValue = watts;

		connect->SendMessage(cm);
	}
}

void PowerSourceConnectorObject::UpdateFlow(double dt)

{
	if (connect)
	{
		ConnectorMessage cm;

		cm.destination = connect->GetType();
		cm.messageType = POWERCON_UPDATE_FLOW;

		cm.val1.dValue = dt;

		connect->SendMessage(cm);
	}
}

PowerDrainConnectorObject::PowerDrainConnectorObject(char *i_name, PanelSDK &sdk)

{
	if (i_name)
		strcpy (name, i_name);

	PowerDraw = 0.0;
	PowerDrawn = 0.0;

	connect = 0;

	sdk.AddElectrical(this, false);
}

void PowerDrainConnectorObject::ProcessUpdateFlow(double dt)

{
	PowerDraw = PowerDrawn;
	PowerDrawn = 0.0;
}

void PowerDrainConnectorObject::ProcessDrawPower(double watts)

{
	PowerDrawn += watts;
}

void PowerDrainConnectorObject::refresh(double dt)

{
	DrawPower(PowerDraw);
}

void PowerDrainConnectorObject::Disconnected()

{
	PowerDrawn = PowerDraw = 0.0;
}