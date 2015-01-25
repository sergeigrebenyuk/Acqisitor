// MCAgent.h : main header file for the MCAGENT application
//

#if !defined(AFX_MCAGENT_H__BC677AEA_5CE0_4F16_9FA1_5BBF4EBF9639__INCLUDED_)
#define AFX_MCAGENT_H__BC677AEA_5CE0_4F16_9FA1_5BBF4EBF9639__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

//#define MCAGENT_API  __declspec(dllexport) 

#include "resource.h"		// main symbols
#include "process.h"
#include "MultiClampBroadcastMsg.hpp"

/////////////////////////////////////////////////////////////////////////////
// CMCAgentApp:
// See MCAgent.cpp for the implementation of this class
//

class CMCAgentApp : public CWinApp
{
public:
	CMCAgentApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMCAgentApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMCAgentApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MCAGENT_H__BC677AEA_5CE0_4F16_9FA1_5BBF4EBF9639__INCLUDED_)
