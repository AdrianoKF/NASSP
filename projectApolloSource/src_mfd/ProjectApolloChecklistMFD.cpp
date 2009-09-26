/***************************************************************************
  This file is part of Project Apollo - NASSP
  Copyright 2004-2008

  Project Apollo Checklist MFD

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
  *	Revision 1.4  2009/09/18 18:29:21  tschachim
  *	Bugfix
  *	
  *	Revision 1.3  2009/09/17 17:48:41  tschachim
  *	DSKY support and enhancements of ChecklistMFD / ChecklistController
  *	
  *	Revision 1.2  2009/08/17 13:27:49  tschachim
  *	Enhancement of ChecklistMFD
  *	
  *	Revision 1.1  2009/02/18 23:21:34  tschachim
  *	Moved files as proposed by Artlav.
  *	
  *	Revision 1.17  2008/12/05 13:05:47  tschachim
  *	New info screen by Coussini.
  *	
  *	Revision 1.16  2008/07/20 22:08:03  tschachim
  *	Slightly improved checklist display.
  *	
  *	Revision 1.15  2008/06/28 10:51:29  jasonims
  *	Updated ChecklistMFD interface to display event times.... only MISSION_TIME and LAST_ITEM_RELATIVE implemented.
  *	
  *	Revision 1.14  2008/06/17 16:39:07  tschachim
  *	Moved prime crew ingress to T-2h40min, bugfixes checklists.
  *	
  *	Revision 1.13  2008/05/27 14:45:39  tschachim
  *	Enabled manual item completion/failure
  *	
  *	Revision 1.12  2008/05/24 17:30:41  tschachim
  *	Bugfixes, new flash toggle.
  *	
  *	Revision 1.11  2008/04/11 11:49:26  tschachim
  *	Fixed BasicExcel for VC6, reduced VS2005 warnings, bugfixes.
  *	
  **************************************************************************/

// To force orbitersdk.h to use <fstream> in any compiler version
#pragma include_alias( <fstream.h>, <fstream> )
#include "orbitersdk.h"

#include "math.h"
#include "windows.h"
#include "nasspsound.h"
#include "OrbiterSoundSDK35.h"
#include "soundlib.h"
#include "tracer.h"
#include "nasspdefs.h"
#include "toggleswitch.h"
#include "apolloguidance.h"
#include "dsky.h"
#include "csmcomputer.h"
#include "lemcomputer.h"
#include "IMU.h"
#include "lvimu.h"
#include "saturn.h"
#include "LEM.h"
#include "Crawler.h"
#include "papi.h"
#include <stdio.h>
#include <string>

#include "MFDResource.h"
#include "ProjectApolloChecklistMFD.h"

#define PROG_CHKLST			0
#define PROG_CHKLSTNAV		1
#define PROG_CHKLSTREV		2
#define PROG_CHKLSTINFO		3

#define LINE0	.10
#define LINE1	.14
#define LINE2	.18
#define LINE3	.22
#define LINE4	.26
#define LINE5	.30
#define LINE6	.34
#define LINE7	.38
#define LINE8	.42
#define LINE9	.46
#define LINE10	.50
#define LINE11  .54
#define LINE12  .58
#define LINE13  .62
#define LINE14  .66
#define LINE15  .70
#define LINE16  .74
#define LINE17  .78
#define LINE18  .82
#define LINE19  .86
#define NLINES  20
#define HLINE   .04

static int hMFD;
static HINSTANCE g_hDLL;

void ProjectApolloChecklistMFDopcDLLInit (HINSTANCE hDLL)
{
	static char* name = "Project Apollo Checklist";
	MFDMODESPEC spec;
	spec.key = OAPI_KEY_C;
	spec.name = name;
	spec.msgproc = ProjectApolloChecklistMFD::MsgProc;
	hMFD = oapiRegisterMFDMode(spec);
	g_hDLL = hDLL;
}
void ProjectApolloChecklistMFDopcDLLExit (HINSTANCE hDLL)
{
	oapiUnregisterMFDMode(hMFD);
}
void ProjectApolloChecklistMFDopcTimestep (double simt, double simdt, double mjd)
{
}
ProjectApolloChecklistMFD::ProjectApolloChecklistMFD (DWORD w, DWORD h, VESSEL *vessel) : MFD (w,h,vessel)
{
	saturn = NULL;
	crawler = NULL;
	lem = NULL;
	conn.ConnectToVessel(vessel);
	width = w;
	height = h;
	screen = PROG_CHKLST;
	NumChkLsts = 0;
	MFDInit = false;
	CurrentStep = 0;
	TopStep = 0;
	HiLghtdLine = 0;
	//SelectedGroup = -1;

	//We need to find out what type of vessel it is, so we check for the class name.
	//Saturns have different functions than Crawlers.  But we have methods for both.
	if (!stricmp(vessel->GetClassName(), "ProjectApollo\\Saturn5") ||
		!stricmp(vessel->GetClassName(), "ProjectApollo/Saturn5") ||
		!stricmp(vessel->GetClassName(), "ProjectApollo\\Saturn1b") ||
		!stricmp(vessel->GetClassName(), "ProjectApollo/Saturn1b")) {
		saturn = (Saturn *)vessel;
	}
	else if (!stricmp(vessel->GetClassName(), "ProjectApollo\\Crawler") ||
		!stricmp(vessel->GetClassName(), "ProjectApollo/Crawler"))  {
			crawler = (Crawler *)vessel;
	}
	else if (!stricmp(vessel->GetClassName(), "ProjectApollo\\LEM") ||
		!stricmp(vessel->GetClassName(), "ProjectApollo/LEM")) {
			lem = (LEM *)vessel;
	}

	hBmpLogo = LoadBitmap(g_hDLL, MAKEINTRESOURCE (IDB_LOGO));
}

ProjectApolloChecklistMFD::~ProjectApolloChecklistMFD ()
{
	/*
	Do not do that, at least not here. The MFD gets destroyed every time you change the 
	panel and is not constructed if the panel has no MFDs, so with this code active you cannot 
	flash switches on the left or right panel for example.

	int i = 0;
	item.group = -1;
	item.index = i;
	while (conn.GetChecklistItem(&item))
	{
		conn.SetFlashing(item.item,false);
		i++;
		item.group = -1;
		item.index = i;
	}
	*/
}
char *ProjectApolloChecklistMFD::ButtonLabel (int bt)
{

	static char *labelCHKLST[12] = {"NAV","","INFO","","FLSH","AUTO","PgUP","UP","PRO","FAIL","DN","PgDN"};
	static char *labelCHKLSTNAV[12] = {"BCK","","INFO","","FLSH","AUTO","PgUP","UP","SEL","REV","DN","PgDN"};
	static char *labelCHKLSTREV[12] = {"NAV","","INFO","","","AUTO","PgUP","UP","","","DN","PgDN"};
	static char *labelCHKLSTINFO[12] = {"BCK","","","","","","","","FLSH","","",""};

	if (screen == PROG_CHKLST) {
		return (bt < 12 ? labelCHKLST[bt] : 0);
	}
	else if (screen == PROG_CHKLSTNAV) {
		return (bt < 12 ? labelCHKLSTNAV[bt] : 0);
	}
	else if (screen == PROG_CHKLSTREV) {
		return (bt < 12 ? labelCHKLSTREV[bt] : 0);
	}
	else if (screen == PROG_CHKLSTINFO) {
		return (bt < 12 ? labelCHKLSTINFO[bt] : 0);
	}

	return 0;
}
int ProjectApolloChecklistMFD::ButtonMenu (const MFDBUTTONMENU **menu) const
{
	static const MFDBUTTONMENU mnuCHKLST[12] = {
		{"Go to","Checklist Navigation",'N'},
		{0,0,0},
		{"More Information","About This Step",'N'},
		{0,0,0},
		{"Toggle Flashing",0,'L'},
		{"Toggle AutoComplete",0,'A'},
		{"Scroll Up","One Page",'<'},
		{"Scroll Up","One Line",'U'},
		{"Step Succeeds",0,'S'},
		{"Step Fails",0,'F'},
		{"Scroll Down","One Line",'D'},
		{"Scroll Down","One Page",'>'}
	};
	static const MFDBUTTONMENU mnuCHKLSTNAV[12] = {
		{"Go back to","the Current Checklist",'C'},
		{0,0,0},
		{"More Information","About This Checklist",'N'},
		{0,0,0},
		{"Toggle Flashing",0,'L'},
		{"Toggle AutoComplete",0,'A'},
		{"Scroll Up","One Page",'<'},
		{"Scroll Up","One Line",'U'},
		{"Select Checklist",0,'S'},
		{"Review Checklist",0,'R'},
		{"Scroll Down","One Line",'D'},
		{"Scroll Down","One Page",'>'}
	};
	static const MFDBUTTONMENU mnuCHKLSTREV[12] = {
		{"Return to","Checklist Navigation",'C'},
		{0,0,0},
		{"More Information","About This Step",'N'},
		{0,0,0},
		{0,0,0},
		{"Toggle Hints",0,'H'},
		{"Scroll Up","One Page",'<'},
		{"Scroll Up","One Line",'U'},
		{0,0,0},
		{0,0,0},
		{"Scroll Down","One Line",'D'},
		{"Scroll Down","One Page",'>'}
	};
	static const MFDBUTTONMENU mnuCHKLSTINFO[12] = {
		{"Back to","Checklist",'B'},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{"Toggle Flashing",0,'F'},
		{0,0,0},
		{0,0,0},
		{0,0,0}
	};
	if (screen == PROG_CHKLST)
	{
		if (menu) *menu = mnuCHKLST;
		return 12;
	}
	else if (screen == PROG_CHKLSTNAV)
	{
		if (menu) *menu = mnuCHKLSTNAV;
		return 12;
	}
	else if (screen == PROG_CHKLSTREV)
	{
		if (menu) *menu = mnuCHKLSTREV;
		return 12;
	}
	else if (screen == PROG_CHKLSTINFO)
	{
		if (menu) *menu = mnuCHKLSTINFO;
		return 12;
	}
	return 0;
}
bool ProjectApolloChecklistMFD::ConsumeButton (int bt, int event)
{
	if (!(event & PANEL_MOUSE_LBDOWN)) return false;
	
	static const DWORD btkeyCHKLST[12] = { OAPI_KEY_C,0,OAPI_KEY_N,0,OAPI_KEY_L,OAPI_KEY_A,OAPI_KEY_PRIOR,OAPI_KEY_U,OAPI_KEY_S,OAPI_KEY_F,OAPI_KEY_D,OAPI_KEY_NEXT};
	static const DWORD btkeyCHKLSTNAV[12] = { OAPI_KEY_C,0,OAPI_KEY_N,0,OAPI_KEY_L,OAPI_KEY_A,OAPI_KEY_PRIOR,OAPI_KEY_U,OAPI_KEY_S,OAPI_KEY_R,OAPI_KEY_D,OAPI_KEY_NEXT};
	static const DWORD btkeyCHKLSTREV[12] = { OAPI_KEY_C,0,OAPI_KEY_N,0,0,OAPI_KEY_A,OAPI_KEY_PRIOR,OAPI_KEY_U,0,0,OAPI_KEY_D,OAPI_KEY_NEXT};
	static const DWORD btkeyCHKLSTINFO[12] = { OAPI_KEY_B,0,0,0,0,0,0,0,OAPI_KEY_F,0,0,0};

	if (screen == PROG_CHKLST)
	{
		if (bt < 12) return ConsumeKeyBuffered (btkeyCHKLST[bt]);
	}
	else if (screen == PROG_CHKLSTNAV)
	{
		if (bt < 12) return ConsumeKeyBuffered (btkeyCHKLSTNAV[bt]);
	}
	else if (screen == PROG_CHKLSTREV)
	{
		if (bt < 12) return ConsumeKeyBuffered (btkeyCHKLSTREV[bt]);
	}
	else if (screen == PROG_CHKLSTINFO)
	{
		if (bt < 12) return ConsumeKeyBuffered (btkeyCHKLSTINFO[bt]);
	}
	
	return false;
}
bool ProjectApolloChecklistMFD::ConsumeKeyBuffered (DWORD key)
{
	ChecklistItem *item;

	if (screen == PROG_CHKLST)
	{
		if (key == OAPI_KEY_C)
		{
			screen = PROG_CHKLSTNAV;
			CurrentStep = 0;
			TopStep = 0;
			HiLghtdLine = 0;

			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_N)
		{
			screen = PROG_CHKLSTINFO;

			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_A)
		{
			ChkLstAutoOn = conn.ChecklistAutocomplete();
			if (conn.ChecklistAutocomplete(!(ChkLstAutoOn)))
				ChkLstAutoOn = !(ChkLstAutoOn);
			
			InvalidateDisplay();
			return true;
		}
		if (key == OAPI_KEY_PRIOR)
		{
			CurrentStep -= 9;
			if (CurrentStep < 0)
				CurrentStep = 0;
			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_U)
		{
			CurrentStep--;
			if (CurrentStep < 0)
				CurrentStep = 0;
			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_S)
		{
			item = conn.GetChecklistItem(-1, CurrentStep);
			if (item)
				conn.completeChecklistItem(item);
			return true;

			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_F)
		{
			item = conn.GetChecklistItem(-1, CurrentStep);
			if (item)
				conn.failChecklistItem(item);
			return true;

			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_D)
		{
			CurrentStep++;
			//TODO: Prevent overrunning max number of items
			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_NEXT)
		{
			CurrentStep += 9;
			//TODO: Prevent overrunning max number of items
			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_L)
		{
			bool fl = conn.GetChecklistFlashing();
			conn.SetChecklistFlashing(!fl);
			InvalidateDisplay();
			return true;
		}
	}
	else if (screen == PROG_CHKLSTNAV)
	{
		if (key == OAPI_KEY_C)
		{
			screen = PROG_CHKLST;
			CurrentStep = 0;
			TopStep = 0;
			HiLghtdLine = 0;
			
			//TODO: Reset MFD
			
			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		else if (key == OAPI_KEY_N)
		{
			return true;
		}
		if (key == OAPI_KEY_A)
		{
			ChkLstAutoOn = conn.ChecklistAutocomplete();
			if (conn.ChecklistAutocomplete(!(ChkLstAutoOn)))
				ChkLstAutoOn = !(ChkLstAutoOn);
			
			InvalidateDisplay();
			return true;
		}
		if (key == OAPI_KEY_PRIOR)
		{
			TopStep -= NLINES;
			CurrentStep -= NLINES;
			// Limit Movement
			if(TopStep < 0) {
				TopStep = 0;
				CurrentStep = 0;
				HiLghtdLine = 0;
			}
			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_U)
		{
			if(HiLghtdLine > 0){
				HiLghtdLine--;
			} else {
				TopStep--;
			}
			CurrentStep--;
			// Limit Movement
			if(TopStep < 0)
				TopStep = 0;
			if(CurrentStep < 0)
				CurrentStep = 0;
			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_S)
		{			
			// This spawns the group, if possible
			conn.GetChecklistItem(CurrentStep, 0);

			screen = PROG_CHKLST;
			CurrentStep = 0;

			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_R)
		{
		
			// TODO SelectedGroup would be the checklist to be reviewed
			//SelectedGroup = CurrentStep;

			screen = PROG_CHKLSTREV;
			CurrentStep = 0;

			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_D)
		{
			if (HiLghtdLine < NLINES - 1){
				HiLghtdLine++;
			} else {
				TopStep++;
			}
			CurrentStep++;
			if (CurrentStep > NumChkLsts - 1) {
				CurrentStep--;
				if (HiLghtdLine < NLINES){
					HiLghtdLine--;
				} else {
					TopStep--;
				}
			}
			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_NEXT)
		{
			TopStep = TopStep + NLINES;
			CurrentStep = CurrentStep + NLINES;
			if (CurrentStep >= NumChkLsts) {
				CurrentStep = NumChkLsts - 1;
				HiLghtdLine = CurrentStep - TopStep;
			}
			if (TopStep >= NumChkLsts) {
				TopStep = NumChkLsts - 1;
				HiLghtdLine = 0;
			}
			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_L)
		{
			bool fl = conn.GetChecklistFlashing();
			conn.SetChecklistFlashing(!fl);
			InvalidateDisplay();
			return true;
		}
	}
	else if (screen == PROG_CHKLSTREV)
	{
		if (key == OAPI_KEY_C)
		{
			screen = PROG_CHKLSTNAV;
			CurrentStep = 0;
			TopStep = 0;
			HiLghtdLine = 0;

			//SelectedGroup = -1;

			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_N)
		{
			screen = PROG_CHKLSTINFO;

			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_A)
		{
			ChkLstAutoOn = conn.ChecklistAutocomplete();
			if (conn.ChecklistAutocomplete(!(ChkLstAutoOn)))
				ChkLstAutoOn = !(ChkLstAutoOn);
			
			InvalidateDisplay();
			return true;
		}
		if (key == OAPI_KEY_PRIOR)
		{
			CurrentStep -= 6;
			if (CurrentStep < 0)
				CurrentStep = 0;
			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_U)
		{
			CurrentStep--;
			if (CurrentStep < 0)
				CurrentStep = 0;
			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_D)
		{
			CurrentStep++;
			//TODO: Prevent overrunning max number of items
			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_NEXT)
		{
			CurrentStep += 9;
			//TODO: Prevent overrunning max number of items
			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
	}
	else if (screen == PROG_CHKLSTINFO)
	{
		if (key == OAPI_KEY_B)
		{
			screen = PROG_CHKLST;
			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
		if (key == OAPI_KEY_F)
		{
			item = conn.GetChecklistItem(-1, CurrentStep);
			if (item)
				conn.SetFlashing(item->item, true);	// TODO DSKY
			InvalidateDisplay();
			InvalidateButtons();
			return true;
		}
	}

	return false;
}
void ProjectApolloChecklistMFD::Update (HDC hDC)
{
	static RECT ShadedBox;
	ChecklistItem *item;
	HBRUSH hBr;

	HDC hDCTemp = CreateCompatibleDC(hDC);
	HBITMAP hBmpTemp = (HBITMAP) SelectObject(hDCTemp, hBmpLogo);
	StretchBlt(hDC, 1, 1, width - 2, height - 2, hDCTemp, 0, 0, 256, 256, SRCCOPY);
	DeleteObject(hBmpTemp);
	DeleteDC(hDCTemp);

	// Retrieve Checklists
	vector<ChecklistGroup> *temp = conn.GetChecklistList();
	if (temp) {
		groups = *temp;
		NumChkLsts = groups.size();
	}

	if (screen == PROG_CHKLST)
	{
		SelectDefaultFont(hDC, 0);
		char *cn = conn.checklistName();
		if (cn)
			Title(hDC, cn);
		
		// Display the MET after the header
		SetTextColor (hDC, RGB(0, 128, 228)); // blue 
		SetTextAlign (hDC, TA_CENTER);
		line = DisplayMissionElapsedTime();
		TextOut(hDC, (int) (width * .5), (int) (height * 0.05), line.c_str(), line.size());

		SetTextAlign (hDC, TA_LEFT);
		SelectDefaultPen(hDC, 1);
		MoveToEx (hDC, (int) (width * 0.05), (int) (height * 0.94), 0);
		LineTo (hDC, (int) (width * 0.95), (int) (height * 0.94));

		//display AutoToggle selection box.
		hBr = CreateSolidBrush( RGB(0, 150, 0));
		if (!conn.ChecklistAutocomplete()){
			SetRect(&ShadedBox,(int) (width * 0.34),(int) (height * 0.955),(int) (width * 0.47),(int) (height*.999));
		}else{
			SetRect(&ShadedBox,(int) (width * 0.24),(int) (height * 0.955),(int) (width * 0.34),(int) (height*.999));
		}
		FillRect(hDC, &ShadedBox, hBr);
		SetTextColor (hDC, RGB(0, 255, 0));
		SetTextAlign (hDC, TA_LEFT);
		TextOut(hDC, (int) (width * .05), (int) (height * .95), " AUTO:  ON  OFF", 15);

		//display flashing selection box. 
		if (!conn.GetChecklistFlashing()){
			SetRect(&ShadedBox,(int) (width * 0.82),(int) (height * 0.955),(int) (width * 0.95),(int) (height*.999));
		}else{
			SetRect(&ShadedBox,(int) (width * 0.72),(int) (height * 0.955),(int) (width * 0.82),(int) (height*.999));
		}
		FillRect(hDC, &ShadedBox, hBr);
		DeleteObject(hBr);
		SetTextColor (hDC, RGB(0, 255, 0));
		SetTextAlign (hDC, TA_LEFT);
		TextOut(hDC, (int) (width * .5), (int) (height * .95), " FLASH:  ON  OFF", 16);

		//Retrieve 15 visible checklist steps  (all are displayed on two lines)
		//Lines 1,2,3,4,5,6,7,8,9 (index 0/1,2/3,4/5,6/7,8/9,10/11,12/13,14/15,16/17)
		int StepCnt = 0;
		bool linebreakpanel = false;
		for (cnt = 0 ; cnt < 20 ; cnt++) {
			if (CurrentStep + StepCnt >= 0){
				item = conn.GetChecklistItem(-1, CurrentStep + StepCnt);
				if (item) {
					//Heading 1
					if (strlen(item->heading1) > 0) {
						if (StepCnt != 0)
							cnt++; //go to next line
						SetTextColor(hDC,RGB(225, 225, 255)); 
						SetTextAlign (hDC, TA_CENTER);
						TextOut(hDC, (int) (width * .5), (int) (height * (LINE0+cnt*HLINE)), item->heading1, strlen(item->heading1));
						cnt++;
						cnt++; //go to next line to write information
					}
					if (cnt >= 20) break;

					//Heading 2
					if (strlen(item->heading2) > 0) {
						if (strlen(item->heading1) == 0 && StepCnt != 0 && !linebreakpanel)
							cnt++; //go to next line
						if (cnt >= 20) break;
						SetTextColor(hDC,RGB(225, 225, 255)); 
						SetTextAlign (hDC, TA_LEFT);
						TextOut(hDC, (int) (width * .02), (int) (height * (LINE0+cnt*HLINE)), item->heading2, strlen(item->heading2));
						cnt++; //go to next line to write information
					}
					if (cnt >= 20) break;

					//Check to see if step is time dependent and print time if so
					if (item->relativeEvent != NO_TIME_DEF && item->relativeEvent != HIDDEN_DELAY) {
						SetTextColor(hDC,RGB(225, 225, 255)); 
						SetTextAlign (hDC, TA_LEFT);
						//display Current Checklist Mission Time
						line = DisplayChecklistMissionTime(item);
						TextOut(hDC, (int) (width * .02), (int) (height * (LINE0+cnt*HLINE)), line.c_str(), line.size());
						cnt++; //go to next line to write information
					}
					if (cnt >= 20) break;

					//Display Highlighted box
					if (StepCnt == 0) {
						hBr = CreateSolidBrush( RGB(0, 100, 0));
						SetRect(&ShadedBox,(int) (width * .01),(int) (height * (LINE0+cnt*HLINE+.008)),(int) (width * .99), (int) (height * (LINE1+cnt*HLINE+.01)));
						FillRect(hDC, &ShadedBox, hBr);
						DeleteObject(hBr);
					}

					//Print Step
					SetTextColor (hDC, RGB(0, 255, 0));
					SetTextAlign (hDC, TA_LEFT);
					TextOut(hDC, (int) (width * .05), (int) (height * (LINE0+cnt*HLINE)), item->text, strlen(item->text));

					//Print panel
					linebreakpanel = false;
					if (strlen(item->panel) > 0) {
						if (strlen(item->panel) + strlen(item->text) > 33) {
							cnt++; //go to next line
							if (cnt >= 20) break;
							linebreakpanel = true;
						}
						SetTextAlign (hDC, TA_RIGHT);
						TextOut(hDC, (int) (width * .95), (int) (height * (LINE0+cnt*HLINE)), item->panel, strlen(item->panel));
					}
				}
			}
			StepCnt++;
		}
		
	}
	else if (screen == PROG_CHKLSTNAV)
	{
		SelectDefaultFont(hDC, 0);				
		Title(hDC, "Checklist Navigation");
		
		// Display the MET after the header
		SetTextColor (hDC, RGB(0, 128, 228)); // blue 
		SetTextAlign (hDC, TA_CENTER);
		line = DisplayMissionElapsedTime();
		TextOut(hDC, (int) (width * .5), (int) (height * 0.05), line.c_str(), line.size());

		SelectDefaultPen(hDC, 1);
		MoveToEx (hDC, (int) (width * 0.05), (int) (height * 0.94), 0);
		LineTo (hDC, (int) (width * 0.95), (int) (height * 0.94));

		//display AutoToggle selection box.
		hBr = CreateSolidBrush( RGB(0, 150, 0));
		if (!conn.ChecklistAutocomplete()){
			SetRect(&ShadedBox,(int) (width * 0.35),(int) (height * 0.96),(int) (width * 0.47),height-1);
		}else{
			SetRect(&ShadedBox,(int) (width * 0.25),(int) (height * 0.96),(int) (width * 0.34),height-1);
		}
		FillRect(hDC, &ShadedBox, hBr);
		SetTextColor (hDC, RGB(0, 255, 0));
		SetTextAlign (hDC, TA_LEFT);
		TextOut(hDC, (int) (width * .05), (int) (height * .95), " AUTO:  ON  OFF", 15);

		//display flashing selection box.
		if (!conn.GetChecklistFlashing()){
			SetRect(&ShadedBox,(int) (width * 0.83),(int) (height * 0.96),(int) (width * 0.95),height-1);
		}else{
			SetRect(&ShadedBox,(int) (width * 0.73),(int) (height * 0.96),(int) (width * 0.82),height-1);
		}
		FillRect(hDC, &ShadedBox, hBr);
		DeleteObject(hBr);
		SetTextColor (hDC, RGB(0, 255, 0));
		SetTextAlign (hDC, TA_LEFT);
		TextOut(hDC, (int) (width * .5), (int) (height * .95), " FLASH:  ON  OFF", 16);
		
		//display Highlighted box
		hBr = CreateSolidBrush( RGB(0, 100, 0));
		SetRect(&ShadedBox,(int) (width * .01),(int) (height * (LINE0 + HiLghtdLine * HLINE + .01)),(int) (width * .99), (int) (height * (LINE1 + HiLghtdLine * HLINE + .01)));
		FillRect(hDC, &ShadedBox, hBr);
		DeleteObject(hBr);

		SelectDefaultFont(hDC, 0);

		//TODO: Handle Writing Text
		/*
		sprintf(buffer, "%d", groups.size());
		line = buffer;
		line = line.append(" checklists found.");
		TextOut(hDC, (int) (width * .10), (int) (height * .05), line.c_str(), line.size());
		*/

		//Set up Type for checklist display
		SetTextAlign (hDC, TA_LEFT);

		//sprintf(oapiDebugString(), "TopStep: %d  CurrentStep: %d  HighLightedStep: %d", TopStep,CurrentStep,HiLghtdLine);
		int cnt;

		//Following is the display loop:
		//		Compares Lines displayed (NumChkLsts - TopStep) to Lines able to display (NLINES)
		//		
		for (cnt = 0; cnt < (((NumChkLsts-TopStep) < NLINES) ? (NumChkLsts-TopStep) : NLINES); cnt++) {

			TextOut(hDC, (int) (width * .05), (int) (height * (LINE0 + (cnt * HLINE))),groups[cnt+TopStep].name,strlen(groups[cnt+TopStep].name));

		}

	}
	else if (screen == PROG_CHKLSTREV)
	{
		/*
		SelectDefaultFont(hDC, 0);
		Title(hDC, conn.checklistName());

		SetTextAlign (hDC, TA_LEFT);

		SelectDefaultPen(hDC, 1);
		MoveToEx (hDC, (int) (width * 0.05), (int) (height * 0.95), 0);
		LineTo (hDC, (int) (width * 0.95), (int) (height * 0.95));

		//display AutoToggle selection box.
		hBr = CreateSolidBrush( RGB(0, 150, 0));
		if (conn.ChecklistAutocomplete(ChkLstAutoOn)){
			SetRect(&ShadedBox,(int) (width * 0.35),(int) (height * 0.96),(int) (width * 0.47),height-1);
			ChkLstAutoOn = true;
		}else{
			SetRect(&ShadedBox,(int) (width * 0.25),(int) (height * 0.96),(int) (width * 0.34),height-1);
			ChkLstAutoOn = false;
		}
		FillRect(hDC, &ShadedBox, hBr);
		SetTextColor (hDC, RGB(0, 255, 0));
		SetTextAlign (hDC, TA_LEFT);
		TextOut(hDC, (int) (width * .05), (int) (height * .95), " AUTO:  ON  OFF", 15);

		//display Highlighted box
		hBr = CreateSolidBrush( RGB(0, 100, 0));
		SetRect(&ShadedBox,(int) (width * .05),(int) (height * (LINE0 + .01)),(int) (width * .95), (int) (height * (LINE2 + .01)));
		FillRect(hDC, &ShadedBox, hBr);

		//Retrieve 9 visible checklist steps  (all are displayed on two lines)

		//Lines 1,2,3,4,5,6,7,8,9 (index 0/1,2/3,4/5,6/7,8/9,10/11,12/13,14/15,16/17)
		for (cnt = 0 ; cnt < 9 ; cnt++) {
			item.group = -1;
			item.index = CurrentStep + cnt;
			if (item.index >= 0){
				if (conn.GetChecklistItem(&item)){
										
					SetTextAlign (hDC, TA_CENTER);
					sprintf(buffer, "%d", item.index);
					line = buffer;
					TextOut(hDC, (int) (width * .05), (int) (height * (LINE0+cnt*2*HLINE+HLINE/2)), line.c_str(), line.size());
					SetTextAlign (hDC, TA_LEFT);
					TextOut(hDC, (int) (width * .1), (int) (height * (LINE0+cnt*2*HLINE)), item.text, strlen(item.text));
					TextOut(hDC, (int) (width * .1), (int) (height * (LINE1+cnt*2*HLINE)), item.info, strlen(item.info));
				}
			}
		}
		*/
	}
	else if (screen == PROG_CHKLSTINFO)
	{
		item = conn.GetChecklistItem(-1, CurrentStep);
		if (item) {
			SelectDefaultFont(hDC, 0);
			Title(hDC, "Checklist Information");
			// Display the MET after the header
			SetTextColor (hDC, RGB(0, 128, 228)); // blue 
			SetTextAlign (hDC, TA_CENTER);
			line = DisplayMissionElapsedTime();
			TextOut(hDC, (int) (width * .5), (int) (height * 0.05), line.c_str(), line.size());
			SetTextColor(hDC,RGB(225, 225, 255)); 
			SetTextAlign (hDC, TA_LEFT);
			//display Current Checklist Mission Time
			if (item->relativeEvent != NO_TIME_DEF) {
				line = DisplayChecklistMissionTime(item);
				TextOut(hDC, (int) (width * .02), (int) (height * (LINE0+(0*HLINE))), line.c_str(), line.size());
			}
			//display Highlighted box
			hBr = CreateSolidBrush( RGB(0, 100, 0));
			SetRect(&ShadedBox,(int) (width * .01),(int) (height * (LINE0 + HLINE + .01)),(int) (width * .99), (int) (height * (LINE1 + HLINE + .01)));
			FillRect(hDC, &ShadedBox, hBr);
			DeleteObject(hBr);
			SelectDefaultPen(hDC, 1);
			MoveToEx (hDC, (int) (width * 0.05), (int) (height * 0.94), 0);
			LineTo (hDC, (int) (width * 0.95), (int) (height * 0.94));
			SelectDefaultFont(hDC, 0);
			SetTextColor (hDC, RGB(0, 255, 0));
			SetTextAlign (hDC, TA_LEFT);
			TextOut(hDC, (int) (width * .05), (int) (height * (LINE0 + (1 * HLINE))), item->text, strlen(item->text));
			TextOut(hDC, (int) (width * .05), (int) (height * (LINE0 + (2 * HLINE))), item->info, strlen(item->info));
		}
	}
}
std::string ProjectApolloChecklistMFD::DisplayChecklistMissionTime(ChecklistItem *item)
{
	char buffer[100];

	temptime.x = 0;
	temptime.y = 0;
	temptime.z = 0;

	switch (item->relativeEvent) {
		case MISSION_TIME:
			temptime.x = floor(fabs(item->time)/3600);
			temptime.y = floor((fabs(item->time)-(temptime.x*3600))/60);
			temptime.z = fabs(item->time)-(temptime.x*3600)-(temptime.y*60);
			if (item->time >= 0.0) {
				sprintf(buffer, "T+%d:%02d:%02d",(int) temptime.x,(int) temptime.y,(int) temptime.z);
			} else {
				sprintf(buffer, "T-%d:%02d:%02d",(int) temptime.x,(int) temptime.y,(int) temptime.z);
			}
			break;

		case LAST_ITEM_RELATIVE:
			temptime.y = floor((fabs(item->time)-(temptime.x*3600))/60);
			temptime.z = fabs(item->time)-(temptime.x*3600)-(temptime.y*60);
			if (temptime.y >= 1.0){
				sprintf(buffer, "After %d min %02d sec",(int) temptime.y,(int) temptime.z);
			} else {
				sprintf(buffer, "After %d sec",(int) temptime.z);
			}
			break;

		case SECOND_STAGE_STAGING:
			sprintf(buffer, "First Staging");
			break;

		case EARTH_ORBIT_INSERTION:
			sprintf(buffer, "Earth Orbit Insertion");
			break;

		default:
			sprintf(buffer, "(Unknown Event)");
			break;
	}
	std::string line = buffer;
	return line;

}
void ProjectApolloChecklistMFD::WriteStatus (FILEHANDLE scn) const
{
}
void ProjectApolloChecklistMFD::ReadStatus (FILEHANDLE scn)
{
}
void ProjectApolloChecklistMFD::StoreStatus (void) const
{
}
void ProjectApolloChecklistMFD::RecallStatus (void)
{
}
	
int ProjectApolloChecklistMFD::MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case OAPI_MSG_MFD_OPENED:
		// Our new MFD mode has been selected, so we create the MFD and
		// return a pointer to it.
		return (int)(new ProjectApolloChecklistMFD (LOWORD(wparam), HIWORD(wparam), (VESSEL*)lparam));
	}
	return 0;
}
// Display MET depending if saturn, crawler or Lem
std::string ProjectApolloChecklistMFD::DisplayMissionElapsedTime (void)
{
	char buffer[20];
	double mt = 0;

	// Take the mission time from class 
	if (saturn){ mt = saturn->GetMissionTime(); }
	if (crawler){ mt = crawler->GetMissionTime(); }
	if (lem){ mt = lem->GetMissionTime(); }

	int secs = abs((int) mt);
	int hours = (secs / 3600);
	secs -= (hours * 3600);
	int minutes = (secs / 60);
	secs -= 60 * minutes;
	if (mt < 0)
		sprintf(buffer, "-%d:%02d:%02d", hours, minutes, secs);
	else
		sprintf(buffer, "%d:%02d:%02d", hours, minutes, secs);
	
	std::string line = buffer;
	return line;
}
