
// FreezeKeyboardLayoutDlg.h : header file
//

#pragma once
#include "afxcmn.h"

#include "Registry.h"


#define WM_TRAY_ICON_NOTIFY_MESSAGE (WM_USER + 1)

// CFreezeKeyboardLayoutDlg dialog
class CFreezeKeyboardLayoutDlg : public CDialog
{
// Construction
public:
	void QueryKeyboardLayoutsKeyAndPopulateDropList(HKEY hKey);
	CFreezeKeyboardLayoutDlg(CWnd* pParent = NULL);	// standard constructor

	NOTIFYICONDATA m_nidIconData;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FreezeKeyboardLayout_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	private:
	afx_msg LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

	bool bWaitedForChangeKLIDWhileFocused = false;
	bool bMinimized = true;
	bool bHidden = false;
	CString m_strKeyboadLayout = L"";
	CFont m_font;
	CRegistry m_regkeyFreezeKeyboardLayout;

public:
	afx_msg void OnBnClickedOk();
	void FixKLID();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedExit();
	afx_msg void OnBnClickedAboutButton();
	CComboBoxEx m_dropListKeyboardLayouts;
	afx_msg void OnCbnSelchangeDroplistKeyboardLayouts();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
};
