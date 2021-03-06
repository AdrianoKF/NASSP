/***************************************************************************
This file is part of Project Apollo - NASSP
Copyright 2017

Saturn Launch Vehicle Data Adapter

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

#include "Orbitersdk.h"

#include "soundlib.h"

#include "apolloguidance.h"
#include "csmcomputer.h"
#include "LVIMU.h"
#include "LVDC.h"
#include "iu.h"
#include "saturn.h"
#include "papi.h"

#include "LVDA.h"

LVDA::LVDA()
{
	iu = NULL;
}

void LVDA::Init(IU *i)
{
	iu = i;
}

void LVDA::SwitchSelector(int stage, int channel)
{
	if (stage < 0 || stage > 3) return;

	iu->ControlDistributor(stage, channel);
}

void LVDA::SetFCCAttitudeError(VECTOR3 atterr)
{
	iu->GetFCC()->SetAttitudeError(atterr);
}

VECTOR3 LVDA::GetLVIMUAttitude()
{
	return iu->lvimu.GetTotalAttitude();
}

void LVDA::ZeroLVIMUPIPACounters()
{
	iu->lvimu.ZeroPIPACounters();
}

void LVDA::ZeroLVIMUCDUs()
{
	iu->lvimu.ZeroIMUCDUFlag = true;
}

void LVDA::ReleaseLVIMUCDUs()
{
	iu->lvimu.ZeroIMUCDUFlag = false;
}

void LVDA::ReleaseLVIMU()
{
	iu->lvimu.SetCaged(false);
}

void LVDA::DriveLVIMUGimbals(double x, double y, double z)
{
	iu->lvimu.DriveGimbals(x, y, z);
}

VECTOR3 LVDA::GetLVIMUPIPARegisters()
{
	return _V(iu->lvimu.CDURegisters[LVRegPIPAX], iu->lvimu.CDURegisters[LVRegPIPAY], iu->lvimu.CDURegisters[LVRegPIPAZ]);
}

bool LVDA::GetSIInboardEngineOut()
{
	return iu->GetSIInboardEngineOut();
}

bool LVDA::GetSIOutboardEngineOut()
{
	return iu->GetSIOutboardEngineOut();
}

bool LVDA::GetSIIEngineOut()
{
	return iu->GetSIIEngineOut();
}

bool LVDA::GetCMCSIVBIgnitionSequenceStart()
{
	return iu->GetCommandConnector()->GetCMCSIVBIgnitionSequenceStart();
}

bool LVDA::GetCMCSIVBCutoff()
{
	return iu->GetCommandConnector()->GetCMCSIVBCutoff();
}

bool LVDA::GetCMCSIVBTakeover()
{
	return iu->GetCommandConnector()->GetCMCSIVBTakeover();
}
bool LVDA::GetLVIMUFailure()
{
	return iu->lvimu.IsFailed();
}

bool LVDA::SIVBInjectionDelay()
{
	return iu->GetCommandConnector()->GetTLIInhibitSignal();
}

bool LVDA::SCInitiationOfSIISIVBSeparation()
{
	return iu->GetCommandConnector()->GetSIISIVbDirectStagingSignal();
}

bool LVDA::GetSIIPropellantDepletionEngineCutoff()
{
	return iu->GetSIIPropellantDepletionEngineCutoff();
}

bool LVDA::SpacecraftSeparationIndication()
{
	return iu->GetLVCommandConnector()->CSMSeparationSensed();
}

bool LVDA::GetSIVBEngineOut()
{
	return iu->GetSIVBEngineOut();
}

bool LVDA::GetSIPropellantDepletionEngineCutoff()
{
	return iu->GetSIPropellantDepletionEngineCutoff();
}

bool LVDA::SIBLowLevelSensorsDry()
{
	return iu->SIBLowLevelSensorsDry();
}

bool LVDA::GetLiftoff()
{
	return iu->IsUmbilicalConnected() == false;
}

bool LVDA::GetSICInboardEngineCutoff()
{
	return iu->GetSIInboardEngineOut();
}

void LVDA::TLIBegun()
{
	iu->GetCommandConnector()->TLIBegun();
}

void LVDA::TLIEnded()
{
	iu->GetCommandConnector()->TLIEnded();
}

bool LVDA::GeneralizedSwitchSelector(int stage, int channel)
{
	if (iu->GetLVDC())
		return iu->GetLVDC()->GeneralizedSwitchSelector(stage, channel);

	return false;
}

bool LVDA::TimebaseUpdate(double dt)
{
	if (iu->GetLVDC())
		return iu->GetLVDC()->TimebaseUpdate(dt);

	return false;
}

bool LVDA::LMAbort()
{
	if (iu->GetLVDC())
		return iu->GetLVDC()->LMAbort();

	return false;
}

bool LVDA::RestartManeuverEnable()
{
	if (iu->GetLVDC())
		return iu->GetLVDC()->RestartManeuverEnable();

	return false;
}

bool LVDA::InhibitAttitudeManeuver()
{
	if (iu->GetLVDC())
		return iu->GetLVDC()->InhibitAttitudeManeuver();

	return false;
}

bool LVDA::Timebase8Enable()
{
	if (iu->GetLVDC())
		return iu->GetLVDC()->TimeBase8Enable();

	return false;
}

bool LVDA::EvasiveManeuverEnable()
{
	if (iu->GetLVDC())
		return iu->GetLVDC()->EvasiveManeuverEnable();

	return false;
}

bool LVDA::ExecuteCommManeuver()
{
	if (iu->GetLVDC())
		return iu->GetLVDC()->ExecuteCommManeuver();

	return false;
}

bool LVDA::SIVBIULunarImpact(double tig, double dt, double pitch, double yaw)
{
	if (iu->GetLVDC())
		return iu->GetLVDC()->SIVBIULunarImpact(tig, dt, pitch, yaw);

	return false;
}

void LVDA::SwitchSelectorOld(int chan)
{
	iu->GetLVCommandConnector()->SwitchSelector(chan);
}

double LVDA::GetMissionTime()
{
	return iu->GetLVCommandConnector()->GetMissionTime();
}

void LVDA::AddForce(VECTOR3 F, VECTOR3 r)
{
	iu->GetLVCommandConnector()->AddForce(F, r);
}

double LVDA::GetFirstStageThrust()
{
	return iu->GetLVCommandConnector()->GetFirstStageThrust();
}

double LVDA::GetAltitude()
{
	return iu->GetLVCommandConnector()->GetAltitude();
}

int LVDA::GetStage()
{
	return iu->GetLVCommandConnector()->GetStage();
}

void LVDA::SetStage(int stage)
{
	iu->GetLVCommandConnector()->SetStage(stage);
}

int LVDA::GetApolloNo()
{
	return iu->GetLVCommandConnector()->GetApolloNo();
}

void LVDA::GetRelativePos(VECTOR3 &v)
{
	iu->GetLVCommandConnector()->GetRelativePos(oapiGetObjectByName("Earth"), v);
}

void LVDA::GetRelativeVel(VECTOR3 &v)
{
	iu->GetLVCommandConnector()->GetRelativeVel(oapiGetObjectByName("Earth"), v);
}

bool LVDA::GetSCControlPoweredFlight()
{
	return iu->GetSCControlPoweredFlight();
}