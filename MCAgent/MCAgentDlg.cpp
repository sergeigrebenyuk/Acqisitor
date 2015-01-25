// MCAgentDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AxMultiClampMsg.h"
#include "MCDataStruct.h"
#include "MCAgent.h"
#include "MCAgentdLG.h"
#include "Winbase.h"
#include <atlbase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



static       UINT s_uMCTGOpenMessage          = 0;
static       UINT s_uMCTGCloseMessage         = 0;
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


STAGE_UNIT *STAGE;
MC_COMMAND *CmdStruct;

HANDLE hSharedFile=0;
HANDLE hMappingObject=0;
/////////////////////////////////////////////////////////////////////////////
// CMCAgentDlg dialog
CMCAgentDlg::CMCAgentDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMCAgentDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMCAgentDlg)
	m_reqperiod = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMCAgentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMCAgentDlg)
	DDX_Control(pDX, IDC_DEVNUM, m_devnum);
	DDX_Control(pDX, IDC_STAT, m_stat);
	DDX_Control(pDX, IDC_CURPERIOD, m_curperiod);
	DDX_Control(pDX, IDC_PN, m_PN);
	DDX_Text(pDX, IDC_REQPERIOD, m_reqperiod);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMCAgentDlg, CDialog)
	//{{AFX_MSG_MAP(CMCAgentDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_COPYDATA()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMCAgentDlg message handlers

unsigned long __stdcall Thread(void *p)
{

	//MC_COMMAND * cmd_struct = (MC_COMMAND *)p;
	//STAGE_UNIT *STAGE = cmd_struct->stage_arr;
	CMCAgentDlg* dlg = (CMCAgentDlg*)p;
	while( !CmdStruct->ShutDown)
	{ 
		
		CString status="";
		for (int i=0; i<CmdStruct->ActiveStages; i++)
		{
		
			UINT uSerialNum = STAGE[i].SN;
			UINT uChannelID = STAGE[i].StageID;
			
			if (STAGE[i].Status == STAT_PENDING)
				STAGE[i].Status = STAT_OFFLINE;
			else
				if (STAGE[i].Status != STAT_OFFLINE)
				STAGE[i].Status = STAT_PENDING;

			if (STAGE[i].Connected)
			{
				LPARAM lparamSignalIDs = MCTG_Pack700BSignalIDs( uSerialNum, uChannelID  );
				if( !::PostMessage( HWND_BROADCAST, s_uMCTGRequestMessage, (WPARAM) dlg->GetSafeHwnd(), lparamSignalIDs ) )
				{
				  CmdStruct->MCAgentOnline = 0;
				}
			}
			
		}
		CString str; str.Format("Req. period: %i ms",CmdStruct->ReqPeriod);
		dlg->m_curperiod.SetWindowText(LPCSTR(str));
	
		CmdStruct->MCAgentOnline = 2;

		UINT sl = CmdStruct->ReqPeriod >= 100 ? CmdStruct->ReqPeriod : 500;
		Sleep(sl);
	}

	CmdStruct->ShutDown = 0;
	return 0;
}


BOOL CMCAgentDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	PN=0;
	m_PN.SetWindowText("No packets received.");
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	//////////////////////////////////

	s_uMCTGOpenMessage = RegisterWindowMessage( MCTG_OPEN_MESSAGE_STR );
	s_uMCTGCloseMessage = RegisterWindowMessage( MCTG_CLOSE_MESSAGE_STR );
	s_uMCTGRequestMessage = RegisterWindowMessage( MCTG_REQUEST_MESSAGE_STR );
	s_uMCTGReconnectMessage = RegisterWindowMessage( MCTG_RECONNECT_MESSAGE_STR );
	s_uMCTGBroadcastMessage = RegisterWindowMessage( MCTG_BROADCAST_MESSAGE_STR );
	s_uMCTGIdMessage = RegisterWindowMessage( MCTG_ID_MESSAGE_STR );

	
	int Error=0;

	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo); 
	int MC_pages = sizeof(MC_COMMAND) / SysInfo.dwAllocationGranularity + 1;
	int	MC_Size = MC_pages * SysInfo.dwAllocationGranularity;

	int STAGE_pages = sizeof(STAGE_UNIT)*MAX_STAGE / SysInfo.dwAllocationGranularity + 1;
	int	STAGE_Size = STAGE_pages * SysInfo.dwAllocationGranularity;

	hSharedFile =  CreateFile( "MCSharedData.map", GENERIC_WRITE|GENERIC_READ, FILE_SHARE_WRITE|FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM|FILE_FLAG_WRITE_THROUGH|FILE_FLAG_RANDOM_ACCESS, 0 );
	if (!hSharedFile) 
		{::MessageBox(NULL,"CreateFile failed!","From DLL",0);  return 1;}

	hMappingObject = CreateFileMapping(hSharedFile, 0, PAGE_READWRITE|SEC_COMMIT, 0, MC_Size + STAGE_Size, "MC_Shared_Area" );
	if (!hMappingObject) 
		{::MessageBox(NULL,"CreateFileMapping failed!","From DLL",0);  return 1;}

	STAGE = (STAGE_UNIT*)MapViewOfFile( hMappingObject, FILE_MAP_ALL_ACCESS, 0, MC_Size, STAGE_Size );
	if (!STAGE) 
		{::MessageBox(NULL,"MapViewOfFile1 failed!","From DLL",0);  
			
			return 1;}

	CmdStruct = (MC_COMMAND*)MapViewOfFile( hMappingObject, FILE_MAP_ALL_ACCESS, 0, 0, MC_Size );
	if (!CmdStruct) 
		{::MessageBox(NULL,"MapViewOfFile2 failed!","From DLL",0);  return 1;}

	
	CmdStruct->stage_arr = STAGE;
	CurReqPeriod = 500;
	CmdStruct->ShutDown = 0;
	TimerID = SetTimer(0,CurReqPeriod,0);

	hThread = CreateThread(0, 0, Thread, this, 0, 0); 
	if (!hThread) {AfxMessageBox("Can't create message thread!"); CmdStruct->MCAgentOnline=0;}

	

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMCAgentDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	if (IsIconic())
	{
		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		RECT r;
		GetClientRect(&r);

/*
		if (MCOnline)
			{
				dc.FillSolidRect(&r,RGB(127,200,127));
			}
		else
			{
				dc.FillSolidRect(&r,RGB(200,127,127));
			}
*/
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMCAgentDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMCAgentDlg::OnTimer(UINT nIDEvent) 
{
	
	Invalidate(FALSE);
	if (nIDEvent == 0)		
	{
		
		if (!CmdStruct->MCOnline)
		{
			//Try to lunch MultiClamp
			
		}
		CString status="";
		for (int i=0; i<CmdStruct->ActiveStages; i++)
		{
			if ((STAGE[i].Status != STAT_OFFLINE) && (STAGE[i].Connected))
			{
				CString t="";
				UINT uSerialNum = STAGE[i].SN;
				UINT uChannelID = STAGE[i].StageID;
				t.Format("Dev SN: %08i  Stage# %03i\n",uSerialNum,uChannelID);
				status += t;
			}
		}

		m_stat.SetWindowText(LPCTSTR(status));
		CString t;t.Format("Connected devices (of %i present):",CmdStruct->ActiveStages);
		m_devnum.SetWindowText(LPCTSTR(t)); 

	}
	
	CmdStruct->MCAgentOnline = 2;
	
	
	CDialog::OnTimer(nIDEvent);
	
}

void CMCAgentDlg::OnClose() 
{
	
	
	
	//if (!CmdStruct->ShutDown) 
	if (AfxMessageBox("Close agent?",MB_YESNO) == IDYES) 
	{
		CDialog::OnClose();	
		
		CmdStruct->ShutDown = 1;

		//while (CmdStruct->ShutDown) 
		Sleep(3000);
		UnmapViewOfFile(STAGE);
		UnmapViewOfFile(CmdStruct);

		CloseHandle(hSharedFile); 
		CloseHandle(hMappingObject);	
		CloseHandle(hThread);	
		CDialog::OnCancel();
	}
	

}

void CMCAgentDlg::OnCancel() 
{
	
	OnClose();	
	//CDialog::OnCancel();
}


BOOL CMCAgentDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) 
{
	
	COPYDATASTRUCT* pcpds = pCopyDataStruct;
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
		 PN++;
		 char str[256];
		 sprintf(str, "Pkts rcvd: %u",PN);
		 m_PN.SetWindowText(str);

		 for (int i=0; i < CmdStruct->ActiveStages; i++)
			 {
				 // is it the correct serial number
				
				 if( STAGE[i].SN == RecievedSN)
				 if( STAGE[i].StageID == pmctdReceived->uChannelID)
				 {
					// copy all telegraph packet data into data member struct
					memcpy(&STAGE[i].MCPacket,pmctdReceived,sizeof(MC_TELEGRAPH_DATA));
					//TRACE0("WM_COPYDATA\n");
					STAGE[i].Status = STAT_ONLINE;

				 }

			 }		 
		  
	}
	
	return CDialog::OnCopyData(pWnd, pCopyDataStruct);
}




void CMCAgentDlg::OnOK() 
{

UpdateData(TRUE);
	CmdStruct->ReqPeriod = m_reqperiod;
}
