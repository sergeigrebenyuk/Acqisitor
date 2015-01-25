// MCAgentDlg.h : header file
//

#if !defined(AFX_MCAGENTDLG_H__F7C37B04_6BB5_4265_8E0A_F11C8FC1EF9C__INCLUDED_)
#define AFX_MCAGENTDLG_H__F7C37B04_6BB5_4265_8E0A_F11C8FC1EF9C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMCAgentDlg dialog

class CMCAgentDlg : public CDialog
{
// Construction
public:
	CMCAgentDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMCAgentDlg)
	enum { IDD = IDD_MCAGENT_DIALOG };
	CStatic	m_devnum;
	CStatic	m_stat;
	CStatic	m_curperiod;
	CEdit	m_devices;
	CStatic	m_PN;
	UINT	m_reqperiod;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMCAgentDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	unsigned long PN;
	UINT CurReqPeriod;
	UINT TimerID;
	HANDLE hThread;
	// Generated message map functions
	//{{AFX_MSG(CMCAgentDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnCancel();
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MCAGENTDLG_H__F7C37B04_6BB5_4265_8E0A_F11C8FC1EF9C__INCLUDED_)
