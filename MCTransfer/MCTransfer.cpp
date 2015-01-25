#include "stdafx.h"
#include "MCTransfer.h"
#include "..\\MCAgent\\MCDataStruct.h"
#include "MultiClampBroadcastMsg.hpp"
#include "AxMultiClampMsg.h" 
#include <stdio.h>
#include <stdlib.h> 
#include  <process.h>
#include  <Winbase.h>
//#include <afx.h>
static       UINT s_uMCTGOpenMessage				= 0;	
static       UINT s_uMCTGCloseMessage			= 0;
static       UINT s_uMCTGRequestMessage       = 0;
static       UINT s_uMCTGReconnectMessage     = 0;
static       UINT s_uMCTGBroadcastMessage     = 0;
static       UINT s_uMCTGIdMessage            = 0;

static const UINT s_cuConnectionTimerEventID  = 13377; // arbitrary
static const UINT s_cuConnectionTimerInterval = 1000;  // millisec

static const UINT s_cuRequestTimerEventID     = 24488; // arbitrary
static const UINT s_cuRequestTimerInterval    = 1000;  // millisec
static const UINT s_cuScanMultiClampTimeOutMS = 10;
static const UINT s_cuNumMultiClampScans      = 100;

// magic number indicating USB port scan detected no devices
static const DWORD s_cdwNoDevice              = 13561;
static const char  s_cszNoDevice[]            = "No Device";

CRITICAL_SECTION CSection;

STAGE_UNIT *STAGE;
MC_COMMAND *CmdStruct;
HANDLE hSharedFile=0;
HANDLE hMappingObject=0;


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


HMCCMSG MCHandle =0;
char ExitPump=0;
                
UINT uModel =0 ;
UINT uCOMPortID =0;
UINT uDeviceID = 0;
UINT uChannelID = 0;
char sSerialNum[16];
int nError = 0;
HWND HWnd = 0;
HINSTANCE hInstance = 0;

/********************************************************************************************/


LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	if (uMsg==WM_TIMER)
		{
		
			//::MessageBeep(MB_ICONEXCLAMATION);     
			::MessageBox(NULL,"Tick","From DLL",0);           
		}
	if (uMsg==WM_COPYDATA)
	{
	::MessageBox(NULL,"COPYDATA","From DLL",0);           

		COPYDATASTRUCT* pcpds = (COPYDATASTRUCT*) lParam;
		if( ( pcpds->cbData == sizeof( MC_TELEGRAPH_DATA ) ) && ( pcpds->dwData == (DWORD) s_uMCTGRequestMessage  ) )
		{
			// this WM_COPYDATA message contains MC_TELEGRAPH_DATA
			MC_TELEGRAPH_DATA* pmctdReceived    = (MC_TELEGRAPH_DATA*) pcpds->lpData;

			// here is a special case for the demo driver
			if( strncmp( pmctdReceived->szSerialNumber,"Demo Driver",  sizeof(pmctdReceived->szSerialNumber) ) == 0 )
			{
				strncpy( pmctdReceived->szSerialNumber, "00000000", sizeof(pmctdReceived->szSerialNumber) );
			}

			UINT RecievedSN = atoi(pmctdReceived->szSerialNumber);

			for (int i=0; i < CmdStruct->ActiveStages; i++)
			{
				// is it the correct serial number

				if( STAGE[i].SN == RecievedSN)
				if( STAGE[i].StageID == pmctdReceived->uChannelID)
				{
					// copy all telegraph packet data into data member struct
					memcpy(&STAGE[i].MCPacket,pmctdReceived,sizeof(MC_TELEGRAPH_DATA));

					STAGE[i].Status = STAT_ONLINE;
				}
			}               
		}
		
	}
	DefWindowProc(hWnd,uMsg,wParam,lParam);
	return 0;
}

BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
			
	char str[16];
	sprintf(&str[0],"Hwnd -%7i, msg.hwnd - %7i, msg - %7i",HWnd, hwndDlg, uMsg);
	SetWindowText(HWnd,&str[0]);
	return TRUE;	
}


/*
void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime )
{
		
    if (!CmdStruct->MCOnline)
    {
                    //Try to lunch MultiClamp
    
    }
    for (int i=0; i<CmdStruct->ActiveStages; i++)
    {

        UINT uSerialNum = UINT(106341);//STAGE[i].SN;
        UINT uChannelID = UINT(1);//STAGE[i].StageID;

        if (STAGE[i].Status == STAT_PENDING)
			STAGE[i].Status = STAT_OFFLINE;
        else
			STAGE[i].Status = STAT_PENDING;
		
        LPARAM lparamSignalIDs = MCTG_Pack700BSignalIDs( uSerialNum, uChannelID  );
        if( !::PostMessage( HWND_BROADCAST, s_uMCTGRequestMessage, (WPARAM)(HWND) HWnd, lparamSignalIDs ) )
       {
        
       }
		
    }
}
  */

UINT hThread =0;
UINT hTimer = 0;
bool MCAgentFound=0;
bool MCFound=0;
BOOL CALLBACK FindMCAgent(HWND hwnd, LPARAM lParam)
{
		
	char name[256];
	::GetWindowText(hwnd,name,256);
	
	if (!strcmp(name,"MultiClamp/LabView Linker - Sergei Grebenyuk, 2005"))
		{
			MCAgentFound = true;
			return FALSE;
		}
	return TRUE;

}
BOOL CALLBACK FindMultiClamp(HWND hwnd, LPARAM lParam)
{
		
	char name[256];
	::GetWindowText(hwnd,name,256);
	
	if (strspn(name,"MultiClamp 700B")>=0)
		{
			MCFound = true;
			return FALSE;
		}
	return TRUE;

}


int Init (HINSTANCE hInstance)
{
    s_uMCTGOpenMessage = RegisterWindowMessage( MCTG_OPEN_MESSAGE_STR );
    s_uMCTGCloseMessage = RegisterWindowMessage( MCTG_CLOSE_MESSAGE_STR );
    s_uMCTGRequestMessage = RegisterWindowMessage( MCTG_REQUEST_MESSAGE_STR );
    s_uMCTGReconnectMessage = RegisterWindowMessage( MCTG_RECONNECT_MESSAGE_STR );
    s_uMCTGBroadcastMessage = RegisterWindowMessage( MCTG_BROADCAST_MESSAGE_STR );
    s_uMCTGIdMessage = RegisterWindowMessage( MCTG_ID_MESSAGE_STR );
        
        int Error=0;
        MCHandle = MCCMSG_CreateObject(&Error);

	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo); 
	int MC_pages = sizeof(MC_COMMAND) / SysInfo.dwAllocationGranularity + 1;
	int	MC_Size = MC_pages * SysInfo.dwAllocationGranularity;

	int STAGE_pages = sizeof(STAGE_UNIT)*MAX_STAGE / SysInfo.dwAllocationGranularity + 1;
	int	STAGE_Size = STAGE_pages * SysInfo.dwAllocationGranularity;

	hSharedFile =  CreateFile( "MCSharedData.map", GENERIC_WRITE|GENERIC_READ, FILE_SHARE_WRITE|FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM|FILE_FLAG_WRITE_THROUGH|FILE_FLAG_RANDOM_ACCESS, 0 );
	if (!hSharedFile) 
		{::MessageBox(NULL,"CreateFile failed!","From DLL",0);  return 1;}

	hMappingObject = CreateFileMapping(hSharedFile, 0, PAGE_READWRITE|SEC_COMMIT, 0, MC_Size + STAGE_Size, "MC_Shared_Area" );
	if (!hMappingObject) 
		{::MessageBox(NULL,"CreateFileMapping failed!","From DLL",0);  return 1;}

	STAGE = (STAGE_UNIT*)MapViewOfFile( hMappingObject, FILE_MAP_ALL_ACCESS, 0, MC_Size, STAGE_Size );
	if (!STAGE) 
		{::MessageBox(NULL,"MapViewOfFile1 failed!","From DLL",0);  
			char str[16];
			sprintf(&str[0],"err - %i",GetLastError());
			::MessageBox(NULL,str,"From DLL",0);	
			return 1;}

	CmdStruct = (MC_COMMAND*)MapViewOfFile( hMappingObject, FILE_MAP_ALL_ACCESS, 0, 0, MC_Size );
	if (!CmdStruct) 
		{::MessageBox(NULL,"MapViewOfFile2 failed!","From DLL",0);  return 1;}


	
		InitializeCriticalSection(&CSection);
		MCCMSG_SetTimeOut(MCHandle, 1000, &nError);

        CmdStruct->ShutDown = 0;
		CmdStruct->ActiveStages=0;
		CmdStruct->mc_cmd=0;
		CmdStruct->MCOnline=0;
		CmdStruct->MCAgentOnline=0;
		
		ExitPump =0;


		EnumDesktopWindows(GetThreadDesktop(GetCurrentThreadId()),FindMCAgent,NULL);
		if (!MCAgentFound)
			{
				/*::MessageBox(NULL,"MCAgent is not loaded! Press OK to load.","From DLL",0);  
				PROCESS_INFORMATION pi;
				if (!CreateProcess("MCAgent.exe", "", 0, 0, 0, 0, NULL, NULL, NULL, &pi))
					::MessageBox(NULL,"Cannot load MCAgent. Try to load it manually and press OK","From DLL",0);  
					*/

				::MessageBox(NULL,"MCAgent is not loaded! Launch it and press OK.","From DLL",0);  
				bool go=true;
				while (go)
					{
						MCAgentFound=0;
						EnumDesktopWindows(GetThreadDesktop(GetCurrentThreadId()),FindMCAgent,NULL);
						if (!MCAgentFound)
							{
								if (::MessageBox(NULL,"Still can't find MCAgent. Try again?","From DLL",MB_YESNO|MB_DEFBUTTON1)==IDNO)
									{go = false;}
							}
						else
							{go = false;}
					}
				
			}			
		if (!MCAgentFound) return 1;
		
		EnumDesktopWindows(GetThreadDesktop(GetCurrentThreadId()),FindMultiClamp,NULL);
		if (!MCFound)
			{
				/*::MessageBox(NULL,"MCAgent is not loaded! Press OK to load.","From DLL",0);  
				PROCESS_INFORMATION pi;
				if (!CreateProcess("MCAgent.exe", "", 0, 0, 0, 0, NULL, NULL, NULL, &pi))
					::MessageBox(NULL,"Cannot load MCAgent. Try to load it manually and press OK","From DLL",0);  
					*/

				::MessageBox(NULL,"MultiClamp is not loaded! Launch it and press OK.","From DLL",0);  
				bool go=true;
				while (go)
					{
						MCFound=0;
						EnumDesktopWindows(GetThreadDesktop(GetCurrentThreadId()),FindMultiClamp,NULL);
						if (!MCFound)
							{
								if (::MessageBox(NULL,"Still can't find MultiClamp. Try again?","From DLL",MB_YESNO|MB_DEFBUTTON1)==IDNO)
									{go = false;}
							}
						else
							{go = false;}
					}
				
			}			
		if (!MCFound) return 1;


        return 0;

}

int DeInit ()
{
        
        CmdStruct->ShutDown = 1;   
        int Error=0;
		
        MCCMSG_DestroyObject(MCHandle);
		DeleteCriticalSection(&CSection);

		UnmapViewOfFile(STAGE); 
		UnmapViewOfFile(CmdStruct); 
		CloseHandle(hMappingObject);	
		CloseHandle(hSharedFile);

        return 0;
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                                         )
{
    switch (ul_reason_for_call)
        {
                case DLL_PROCESS_ATTACH: Init((HINSTANCE)hModule); break;
                case DLL_THREAD_ATTACH:  break;
                case DLL_THREAD_DETACH:  break;
                case DLL_PROCESS_DETACH: DeInit(); break;
                        break;
    }
	hInstance = (HINSTANCE) hModule;
    return TRUE;
}

/********************************************************************************************/


MCTRANSFER_API int Test(UINT t)
{
        int s = 78;
		CmdStruct->ActiveStages = 0;   
        return -1;
}
/*
MCTRANSFER_API void StartPump()
{
	//hThread = _beginthread(Thread,0,0);
	HANDLE hiThread = CreateThread(0, 0, Thread, 0, 0, NULL); 
	if (!hiThread)	::MessageBox(NULL,"Thread couldn't be created!","From DLL",0);                

    hTimer = SetTimer(NULL, 0, 1000, TimerProc ); 
	//hTimer = SetTimer(HWnd, 1, 1000, NULL ); 
}
  
MCTRANSFER_API void StopPump()
{
	
	KillTimer(NULL, 0 ); 
	ExitPump = 1;
	while (ExitPump) Sleep(10);
	
}
  */
MCTRANSFER_API void Reset()
{
	CmdStruct->ActiveStages=0;
	CmdStruct->MCOnline=0;
	CmdStruct->ShutDown=0;
	CmdStruct->mc_cmd=0;
    memset(&STAGE[0], 0, sizeof(STAGE_UNIT)*MAX_STAGE);
    for (int i=0; i< MAX_STAGE; i++)
    {
            STAGE[i].MCPacket.uVersion              = MCTG_API_VERSION;
            STAGE[i].MCPacket.uStructSize           = sizeof(MC_TELEGRAPH_DATA);
            STAGE[i].MCPacket.uComPortID            = 1;
            STAGE[i].MCPacket.uAxoBusID             = 0;
            STAGE[i].MCPacket.uChannelID            = 1;
            STAGE[i].MCPacket.uOperatingMode        = MCTG_MODE_VCLAMP;
            STAGE[i].MCPacket.uScaledOutSignal      = AXMCD_OUT_PRI_VC_GLDR_V_CMD_EXT;
            STAGE[i].MCPacket.dAlpha                = 0.0;
            STAGE[i].MCPacket.dScaleFactor          = 0.0;
            STAGE[i].MCPacket.uScaleFactorUnits     = MCTG_UNITS_VOLTS_PER_VOLT;
            STAGE[i].MCPacket.dRawScaleFactor       = 0.0;
            STAGE[i].MCPacket.uRawScaleFactorUnits  = MCTG_UNITS_VOLTS_PER_VOLT;
            STAGE[i].MCPacket.dLPFCutoff            = 0.0;
            STAGE[i].MCPacket.dMembraneCap          = 0.0;
            STAGE[i].MCPacket.dExtCmdSens           = 0.0;
            STAGE[i].MCPacket.dSecondaryAlpha       = 0.0;
            STAGE[i].MCPacket.dSecondaryLPFCutoff   = 0.0;
    }       

    ExitPump =0;
        
}
MCTRANSFER_API void TossMCPacket(long buf)
{

		EnterCriticalSection( &CSection );

		COPYDATASTRUCT* pcpds = (COPYDATASTRUCT*) buf;
		
		if( ( pcpds->cbData == sizeof( MC_TELEGRAPH_DATA ) ) && ( pcpds->dwData == (DWORD) s_uMCTGRequestMessage  ) )
		{
			// this WM_COPYDATA message contains MC_TELEGRAPH_DATA
			MC_TELEGRAPH_DATA* pmctdReceived    = (MC_TELEGRAPH_DATA*) pcpds->lpData;

			// here is a special case for the demo driver
			if( strncmp( pmctdReceived->szSerialNumber,"Demo Driver",  sizeof(pmctdReceived->szSerialNumber) ) == 0 )
			{
				strncpy( pmctdReceived->szSerialNumber, "00000000", sizeof(pmctdReceived->szSerialNumber) );
			}

			UINT RecievedSN = atoi(pmctdReceived->szSerialNumber);

			for (int i=0; i < CmdStruct->ActiveStages; i++)
			{
				// is it the correct serial number

				if( STAGE[i].SN == RecievedSN)
				if( STAGE[i].StageID == pmctdReceived->uChannelID)
				{
					// copy all telegraph packet data into data member struct
					memcpy(&STAGE[i].MCPacket,pmctdReceived,sizeof(MC_TELEGRAPH_DATA));

					STAGE[i].Status = STAT_ONLINE;
				}
			}               
		}
	
		LeaveCriticalSection( &CSection );

}
/*
MCTRANSFER_API int ScanMultiClamps2()
{

        

		// Find the first MultiClamp Commander and return device info
		if (!MCCMSG_FindFirstMultiClamp(MCHandle, &uModel, &sSerialNum[0], 16, &uCOMPortID, &uDeviceID, &uChannelID, &nError))
			{
				//No devices found...
				return 0;
			}

		bool found=false;
		UINT SN = atoi(&sSerialNum[0]);
		uChannelID = 1;
		if (MCCMSG_SelectMultiClamp(MCHandle, uModel, &sSerialNum[0], uCOMPortID, uDeviceID, uChannelID, &nError))
		{
			
			for(int i=0; i < CmdStruct->ActiveStages; i++)
				if ((STAGE[i].SN == SN)||(STAGE[i].StageID == uChannelID))	{found = true; break;}
			if (!found)
			{
				STAGE[CmdStruct->ActiveStages].SN = SN;
				STAGE[CmdStruct->ActiveStages].StageID = uChannelID;
				STAGE[i].MCPacket.uChannelID = uChannelID;
				sprintf(&STAGE[CmdStruct->ActiveStages].MCPacket.szSerialNumber[0],"%08i",SN);
				CmdStruct->ActiveStages++;
			}
		}
		found=false; uChannelID = 2;
		if (MCCMSG_SelectMultiClamp(MCHandle, uModel, &sSerialNum[0], uCOMPortID, uDeviceID, uChannelID, &nError))
		{
			
			for(int i=0; i < CmdStruct->ActiveStages; i++)
				if ((STAGE[i].SN == SN)||(STAGE[i].StageID == uChannelID))	{found = true; break;}
			if (!found)
			{
				STAGE[CmdStruct->ActiveStages].SN = SN;
				STAGE[CmdStruct->ActiveStages].StageID = uChannelID;
				STAGE[i].MCPacket.uChannelID = uChannelID;
				sprintf(&STAGE[CmdStruct->ActiveStages].MCPacket.szSerialNumber[0],"%08i",SN);
				CmdStruct->ActiveStages++;
			}
		}

		

		for (int i=0; i< MAX_STAGE; i++)
		{
			// Find next MultiClamp Commander and return device info, returns FALSE when all MultiClamp Commanders have been found
			if (!MCCMSG_FindNextMultiClamp(MCHandle, &uModel, &sSerialNum[0], 16, &uCOMPortID, &uDeviceID, &uChannelID, &nError))
				{
					//No more devices found...
					return 0;
				}
			found=false;
			UINT SN = atoi(&sSerialNum[0]);
			uChannelID = 1;
			if (MCCMSG_SelectMultiClamp(MCHandle, uModel, &sSerialNum[0], uCOMPortID, uDeviceID, uChannelID, &nError))
			{
				
				for(int i=0; i < CmdStruct->ActiveStages; i++)
					if ((STAGE[i].SN == SN)||(STAGE[i].StageID == uChannelID))	{found = true; break;}
				if (!found)
				{
					STAGE[CmdStruct->ActiveStages].SN = SN;
					STAGE[CmdStruct->ActiveStages].StageID = uChannelID;
					STAGE[i].MCPacket.uChannelID = uChannelID;
					sprintf(&STAGE[CmdStruct->ActiveStages].MCPacket.szSerialNumber[0],"%08i",SN);
					CmdStruct->ActiveStages++;
				}
			}
			found=false; 
			uChannelID = 2;
			if (MCCMSG_SelectMultiClamp(MCHandle, uModel, &sSerialNum[0], uCOMPortID, uDeviceID, uChannelID, &nError))
			{
				
				for(int i=0; i < CmdStruct->ActiveStages; i++)
					if ((STAGE[i].SN == SN)||(STAGE[i].StageID == uChannelID))	{found = true; break;}
				if (!found)
				{
					if (CmdStruct->ActiveStages < MAX_STAGE - 1)      
					{
						STAGE[CmdStruct->ActiveStages].SN = SN;
						STAGE[CmdStruct->ActiveStages].StageID = uChannelID;
						STAGE[i].MCPacket.uChannelID = uChannelID;
						sprintf(&STAGE[CmdStruct->ActiveStages].MCPacket.szSerialNumber[0],"%08i",SN);
						CmdStruct->ActiveStages++;
					}
				}
			}
			

		}
        
    return 0;            
}
*/
MCTRANSFER_API int ScanMultiClamps()
{

        
		// Find the first MultiClamp Commander and return device info
		if (!MCCMSG_FindFirstMultiClamp(MCHandle, &uModel, &sSerialNum[0], 16, &uCOMPortID, &uDeviceID, &uChannelID, &nError))
			{
				//No devices found...
				return 0;
			}
		
		bool found=false;
		UINT SN = atoi(&sSerialNum[0]);
		for(int j=0; j < CmdStruct->ActiveStages;j++)
			if ((STAGE[j].SN == SN)&&(STAGE[j].StageID == uChannelID))	{found = true; break;}
			
		if (!found)
		{
			STAGE[CmdStruct->ActiveStages].SN = SN;
			STAGE[CmdStruct->ActiveStages].StageID = uChannelID;
			STAGE[CmdStruct->ActiveStages].MCPacket.uChannelID = uChannelID;
			sprintf(&STAGE[CmdStruct->ActiveStages].MCPacket.szSerialNumber[0],"%08i",SN);
			CmdStruct->ActiveStages++;
		}

		//for (int i=0; i< MAX_STAGE; i++)
		while(1)
		{
			// Find next MultiClamp Commander and return device info, returns FALSE when all MultiClamp Commanders have been found
			if (!MCCMSG_FindNextMultiClamp(MCHandle, &uModel, &sSerialNum[0], 16, &uCOMPortID, &uDeviceID, &uChannelID, &nError))
			{
				//No more devices found...
				break;
			}
			
			found=false;
			UINT SN = atoi(&sSerialNum[0]);
				
			for(int j=0; j < CmdStruct->ActiveStages;j++)
				if ((STAGE[j].SN == SN)&&(STAGE[j].StageID == uChannelID))	{found = true; break;}
			
			if (!found)
			{
				if (CmdStruct->ActiveStages < MAX_STAGE - 1)      
				{
					STAGE[CmdStruct->ActiveStages].SN = SN;
					STAGE[CmdStruct->ActiveStages].StageID = uChannelID;
					STAGE[CmdStruct->ActiveStages].MCPacket.uChannelID = uChannelID;
					sprintf(&STAGE[CmdStruct->ActiveStages].MCPacket.szSerialNumber[0],"%08i",SN);
					CmdStruct->ActiveStages++;
				}
				else break;
			}

		}
        
    return 0;            
}

MCTRANSFER_API int ConnectBySerialNumber(UINT SN, UINT Stage)
{

    for(int i=0; i < CmdStruct->ActiveStages; i++)
    {
            if ((STAGE[i].SN == SN)&&(STAGE[i].StageID == Stage))
				{
					STAGE[i].Connected = 1;
                    return i;
				}
    }
        
    return -1;            
}
MCTRANSFER_API int ConnectByOrder(int Idx)
{
if (Idx < CmdStruct->ActiveStages)	
	{
		STAGE[Idx].Connected = 1;
		return Idx;
	}
	else return -1;
}

MCTRANSFER_API int SelectMC(UINT StageIdx)
{
		return MCCMSG_SelectMultiClamp(MCHandle, uModel, &STAGE[StageIdx].MCPacket.szSerialNumber[0], uCOMPortID, uDeviceID, STAGE[StageIdx].StageID, &nError);
}

MCTRANSFER_API int GetMode(UINT StageIdx)
{
        return STAGE[StageIdx].MCPacket.uOperatingMode;
}

MCTRANSFER_API double GetPrimaryScaleIn(UINT StageIdx)
{
        return STAGE[StageIdx].MCPacket.dScaleFactor * STAGE[StageIdx].MCPacket.dAlpha;
}

MCTRANSFER_API double GetSecondaryScaleIn(UINT StageIdx)
{
        return STAGE[StageIdx].MCPacket.dRawScaleFactor * STAGE[StageIdx].MCPacket.dSecondaryAlpha;
}

MCTRANSFER_API double GetScaleOut(UINT StageIdx)
{
        return STAGE[StageIdx].MCPacket.dExtCmdSens;
}

MCTRANSFER_API char GetStatus(UINT StageIdx)
{
        return STAGE[StageIdx].Status;
}


MCTRANSFER_API char IsAgentOnline()
{
	int a =  CmdStruct->MCAgentOnline;

	if ( --CmdStruct->MCAgentOnline < 0) CmdStruct->MCAgentOnline=0;    
	return a;
		
}

MCTRANSFER_API char SetReqPeriod(UINT Period)
{
        return CmdStruct->ReqPeriod = Period;
}


MCTRANSFER_API int SetMode(/*UINT StageIdx,*/ UINT Mode)
{
        int nError=0;
                
        //if( !MCCMSG_SelectMultiClamp(MCHandle, MCCMSG_HW_TYPE_MC700B, &STAGE[StageIdx].MCPacket.szSerialNumber[0], 0, 0, STAGE[StageIdx].StageID, &nError) ) return 0;
        return MCCMSG_SetMode(MCHandle, Mode, &nError);
        
}

MCTRANSFER_API int AutoPipetteOffset(/*UINT StageIdx*/)
{
        int nError=0;
        //if( !MCCMSG_SelectMultiClamp(MCHandle, MCCMSG_HW_TYPE_MC700B, &STAGE[StageIdx].MCPacket.szSerialNumber[0], 0, 0, STAGE[StageIdx].StageID, &nError) ) return 0;
        return MCCMSG_AutoPipetteOffset(MCHandle, &nError);
}

MCTRANSFER_API int SetPipetteOffset(/*UINT StageIdx*/ double Offs)
{
        int nError=0;
        //if( !MCCMSG_SelectMultiClamp(MCHandle, MCCMSG_HW_TYPE_MC700B, &STAGE[StageIdx].MCPacket.szSerialNumber[0], 0, 0, STAGE[StageIdx].StageID, &nError) ) return 0;
        return MCCMSG_SetPipetteOffset(MCHandle, Offs, &nError);
}


MCTRANSFER_API int AutoFastCompensation(/*UINT StageIdx*/)
{
        int nError=0;
        //if( !MCCMSG_SelectMultiClamp(MCHandle, MCCMSG_HW_TYPE_MC700B, &STAGE[StageIdx].MCPacket.szSerialNumber[0], 0, 0, STAGE[StageIdx].StageID, &nError) ) return 0;
        return MCCMSG_AutoFastComp(MCHandle, &nError);
}

MCTRANSFER_API int AutoSlowCompensation(/*UINT StageIdx*/)
{
        int nError=0;
        //if( !MCCMSG_SelectMultiClamp(MCHandle, MCCMSG_HW_TYPE_MC700B, &STAGE[StageIdx].MCPacket.szSerialNumber[0], 0, 0, STAGE[StageIdx].StageID, &nError) ) return 0;
        return MCCMSG_AutoSlowComp(MCHandle, &nError);
}

MCTRANSFER_API int AutoWholeCellCompensation(/*UINT StageIdx*/)
{
        int nError=0;
        //if( !MCCMSG_SelectMultiClamp(MCHandle, MCCMSG_HW_TYPE_MC700B, &STAGE[StageIdx].MCPacket.szSerialNumber[0], 0, 0, STAGE[StageIdx].StageID, &nError) ) return 0;
        return MCCMSG_AutoWholeCellComp(MCHandle, &nError);
}
MCTRANSFER_API int AutoLeakSubst(/*UINT StageIdx*/)
{
        int nError=0;
        //if( !MCCMSG_SelectMultiClamp(MCHandle, MCCMSG_HW_TYPE_MC700B, &STAGE[StageIdx].MCPacket.szSerialNumber[0], 0, 0, STAGE[StageIdx].StageID, &nError) ) return 0;
        return MCCMSG_AutoLeakSub(MCHandle, &nError);
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


// This is an example of an exported variable
MCTRANSFER_API int nMCTransfer=0;

// This is an example of an exported function.
MCTRANSFER_API int fnMCTransfer(void)
{
        return 42;
}

// This is the constructor of a class that has been exported.
// see MCTransfer.h for the class definition
CMCTransfer::CMCTransfer()
{ 
        return; 
}

