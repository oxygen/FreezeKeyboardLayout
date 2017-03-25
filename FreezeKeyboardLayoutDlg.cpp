
// FreezeKeyboardLayoutDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FreezeKeyboardLayout.h"
#include "FreezeKeyboardLayoutDlg.h"
#include "afxdialogex.h"


#include <algorithm>        // std::sort
#include <utility>          // std::begin, std::end


#include <windows.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int FREEZE_KEYBOARD_LAYOUT_EVENT_ID = 12346;
const int ON_STARTUP_DELAY_TIMER = 12347;

const int TRAY_ID = 1;


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()



// From https://msdn.microsoft.com/en-us/library/windows/desktop/ms724256(v=vs.85).aspx
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
void CFreezeKeyboardLayoutDlg::QueryKeyboardLayoutsKeyAndPopulateDropList(HKEY hKey)
{
	TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	DWORD    cbName;                   // size of name string 
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys = 0;               // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 

	DWORD i, retCode;

	TCHAR  achValue[MAX_VALUE_NAME];
	DWORD cchValue = MAX_VALUE_NAME;
	PLONG pcchValue = (PLONG)MAX_VALUE_NAME;
	
	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hKey,                    // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime         // last write time 
	);

	// Enumerate the subkeys, until RegEnumKeyEx fails.

	if (cSubKeys)
	{
		CString arrCodes[4096];
		CString arrNames[4096];
		int nInserted = 0;

		for (i = 0; i<cSubKeys; i++)
		{
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(
				hKey, 
				i,
				achKey,
				&cbName,
				NULL,
				NULL,
				NULL,
				&ftLastWriteTime
			);
			
			if (retCode == ERROR_SUCCESS)
			{
				// Try open registry key
				HKEY hKey = NULL;

				CString strSubkey = L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts\\" + (CString)achKey;
				if (RegOpenKey(HKEY_LOCAL_MACHINE, strSubkey, &hKey) != ERROR_SUCCESS)
				{
					AfxMessageBox(L"Failed opening Keyboard Layouts registry key.");
					abort();
				}

				// Buffer to store string read from registry
				TCHAR szValue[1024];
				DWORD cbValueLength = sizeof(szValue);

				// Query string value
				if (
					RegQueryValueEx(
						hKey,
						_T("Layout Text"),
						NULL,
						NULL,
						reinterpret_cast<LPBYTE>(&szValue),
						&cbValueLength
					) != ERROR_SUCCESS
				)
				{
					// Error
					// throw an exception or something...
					//AtlThrowLastWin32();
				}
				
				// Create a CString from the value buffer
				CString LayoutName = (CString)(LPWSTR)szValue;

				CString strPadding = L"";
				for (int y = 0; y < 100 - LayoutName.GetLength(); y++)
				{
					strPadding += L" ";
				}
				arrNames[nInserted] = LayoutName + strPadding + (CString)achKey;

				nInserted++;
			}
		}


		m_strKeyboadLayout = m_regkeyFreezeKeyboardLayout.ReadString((CString)"KLID", (CString)"");

		if (!m_strKeyboadLayout.GetLength())
		{
			TCHAR strCurrentKeyboardLayoutID[256];
			GetKeyboardLayoutName(strCurrentKeyboardLayoutID);

			m_strKeyboadLayout = (CString)strCurrentKeyboardLayoutID;
		}


		std::sort(arrNames, arrNames+nInserted);
		int nSelectedIndex = -1;
		for (int x = nInserted-1; x >= 0; x--)
		{
			COMBOBOXEXITEM item;
			ZeroMemory(&item, sizeof(item));
			item.mask = CBEIF_TEXT;
			item.iItem = 0;
			item.pszText = (LPTSTR)(LPCTSTR)(arrNames[x]);

			m_dropListKeyboardLayouts.InsertItem(&item);

			if ((CString)m_strKeyboadLayout == arrNames[x].Mid(arrNames[x].GetLength()-8))
			{
				nSelectedIndex = x;
			}
		}

		if (nSelectedIndex > -1)
		{
			m_dropListKeyboardLayouts.SetCurSel(nSelectedIndex);
			
			if (m_strKeyboadLayout != "00000000")
			{
				m_regkeyFreezeKeyboardLayout.WriteString((CString)"KLID", m_strKeyboadLayout);
				LoadKeyboardLayout(m_strKeyboadLayout, KLF_ACTIVATE | KLF_NOTELLSHELL | KLF_REORDER | KLF_REPLACELANG);
			}
		}


		/*CDC *pDC = GetDC();
		LOGFONT lf;
		memset(&lf, 0, sizeof(lf));
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
		lf.lfOutPrecision = OUT_RASTER_PRECIS;
		lf.lfHeight = MulDiv(10, ::GetDeviceCaps(pDC->m_hDC, LOGPIXELSY), 72);
		wcscpy(lf.lfFaceName, L"Courier New");
		m_font.CreateFontIndirect(&lf);
		m_dropListKeyboardLayouts.SetFont(&m_font);
		ReleaseDC(pDC);*/
	}
}



// CFreezeKeyboardLayoutDlg dialog
CFreezeKeyboardLayoutDlg::CFreezeKeyboardLayoutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_FreezeKeyboardLayout_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFreezeKeyboardLayoutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DROPLIST_KEYBOARD_LAYOUTS, m_dropListKeyboardLayouts);
}

BEGIN_MESSAGE_MAP(CFreezeKeyboardLayoutDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CFreezeKeyboardLayoutDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCANCEL, &CFreezeKeyboardLayoutDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_EXIT, &CFreezeKeyboardLayoutDlg::OnBnClickedExit)
	ON_BN_CLICKED(IDC_BUTTON2, &CFreezeKeyboardLayoutDlg::OnBnClickedAboutButton)
	ON_MESSAGE(WM_TRAY_ICON_NOTIFY_MESSAGE, OnTrayNotify)
	ON_CBN_SELCHANGE(IDC_DROPLIST_KEYBOARD_LAYOUTS, &CFreezeKeyboardLayoutDlg::OnCbnSelchangeDroplistKeyboardLayouts)
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()


// CFreezeKeyboardLayoutDlg message handlers

BOOL CFreezeKeyboardLayoutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();


	m_regkeyFreezeKeyboardLayout.SetRootKey(HKEY_CURRENT_USER);
	m_regkeyFreezeKeyboardLayout.SetKey((CString)"SOFTWARE\\FreezeKeyboardLayout", true);


	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here


	m_nidIconData.hIcon = m_hIcon;
	m_nidIconData.uFlags = NIF_MESSAGE;
	m_nidIconData.uFlags |= NIF_ICON;
	m_nidIconData.cbSize = sizeof(NOTIFYICONDATA);

	m_nidIconData.hWnd = m_hWnd;
	m_nidIconData.uID = TRAY_ID;

	m_nidIconData.uCallbackMessage = WM_TRAY_ICON_NOTIFY_MESSAGE;

	LPCWSTR lpszToolTip = L"Freezing your keyboard layout!";
	wcscpy_s(m_nidIconData.szTip, lpszToolTip);
	m_nidIconData.uFlags |= NIF_TIP;

	Shell_NotifyIcon(NIM_ADD, &m_nidIconData); 

	HKEY hKeyboardLayoutsKey;

	if (
		RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			TEXT("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts"),
			0,
			KEY_READ,
			&hKeyboardLayoutsKey
		) == ERROR_SUCCESS
	)
	{
		this->QueryKeyboardLayoutsKeyAndPopulateDropList(hKeyboardLayoutsKey);
	}
	else
	{
		AfxMessageBox(L"Unable to open HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts");
		::exit(1);
	}

	RegCloseKey(hKeyboardLayoutsKey);


	
	// Prevent multiple instances of the same app from running.
	HANDLE hMutex = CreateMutexA(NULL, FALSE, "FreezeKeyboardLayoutApp");
	DWORD dwMutexWaitResult = WaitForSingleObject(hMutex, 0);
	if (dwMutexWaitResult != WAIT_OBJECT_0)
	{
		CloseHandle(hMutex);
		this->OnBnClickedExit();
	}


	FixKLID();

	SetTimer(FREEZE_KEYBOARD_LAYOUT_EVENT_ID, 2500 /*milliseconds*/, NULL);
	SetTimer(ON_STARTUP_DELAY_TIMER, 1000 /*milliseconds*/, NULL);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFreezeKeyboardLayoutDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if(nID == SC_MINIMIZE)
	{
		this->bMinimized = true;

		CDialog::OnSysCommand(nID, lParam);
	}
	else if (nID == SC_RESTORE)
	{
		this->bMinimized = false;
		CDialog::OnSysCommand(nID, lParam);
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFreezeKeyboardLayoutDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFreezeKeyboardLayoutDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CFreezeKeyboardLayoutDlg::OnBnClickedOk()
{
	FixKLID();

	// TODO: Add your control notification handler code here
	// CDialog::OnOK();
	ShowWindow(SW_MINIMIZE);
	this->bMinimized = true;

	SetTimer(ON_STARTUP_DELAY_TIMER, 300 /*milliseconds*/, NULL);
}


void CFreezeKeyboardLayoutDlg::FixKLID()
{
	if (m_strKeyboadLayout == L"")
	{
		return;
	}


	// Fatal flaw of this application:
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646305(v=vs.85).aspx
	// (LoadKeyboardLayout) This function has no effect if the current process does not own the window with keyboard focus.
	// @TODO workaround: use another process's foreground window to call GetKeyboardLayoutName() from user32.dll.

	
	TCHAR strCurrentKeyboardLayoutID[256];
	GetKeyboardLayoutName(strCurrentKeyboardLayoutID);


	LoadKeyboardLayout(m_strKeyboadLayout, KLF_ACTIVATE | KLF_NOTELLSHELL | KLF_REORDER | KLF_REPLACELANG);
	HKL hkls[4096];
	int nCount = GetKeyboardLayoutList(4096, hkls);
	while (--nCount)
	{
		UnloadKeyboardLayout(hkls[nCount]);
	}
	LoadKeyboardLayout(m_strKeyboadLayout, KLF_ACTIVATE | KLF_NOTELLSHELL | KLF_REORDER | KLF_REPLACELANG);
}


void CFreezeKeyboardLayoutDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (
		nIDEvent == FREEZE_KEYBOARD_LAYOUT_EVENT_ID
		|| nIDEvent == ON_STARTUP_DELAY_TIMER
	)
	{
		FixKLID();
	}


	if (nIDEvent == ON_STARTUP_DELAY_TIMER)
	{
		if (this->bMinimized && !this->bHidden)
		{
			this->bHidden = true;
			ShowWindow(SW_HIDE);
		}

		if (!this->bMinimized && this->bHidden)
		{
			this->bHidden = false;
			ShowWindow(SW_RESTORE);
		}

		KillTimer(ON_STARTUP_DELAY_TIMER);
	}


	CDialog::OnTimer(nIDEvent);
}


void CFreezeKeyboardLayoutDlg::OnBnClickedCancel()
{
	ShowWindow(SW_MINIMIZE);
	this->bMinimized = true;
	SetTimer(ON_STARTUP_DELAY_TIMER, 300 /*milliseconds*/, NULL);
}


void CFreezeKeyboardLayoutDlg::OnBnClickedExit()
{
	if(m_nidIconData.hWnd && m_nidIconData.uID>0)
	{
		Shell_NotifyIcon(NIM_DELETE, &m_nidIconData);
	}


	CDialog::OnCancel();
}


void CFreezeKeyboardLayoutDlg::OnBnClickedAboutButton()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}


LRESULT CFreezeKeyboardLayoutDlg::OnTrayNotify(WPARAM wParam, LPARAM lParam)
{
	UINT uID;
	UINT uMsg;

	uID = (UINT)wParam;
	uMsg = (UINT)lParam;

	if (uID != TRAY_ID)
		return 0;

	switch (uMsg)
	{
		if (this->bMinimized && !this->bHidden)
		{
			this->bHidden = true;
			ShowWindow(SW_HIDE);
		}

		if (!this->bMinimized && this->bHidden)
		{
			this->bHidden = false;
			ShowWindow(SW_RESTORE);
		}


		case WM_MOUSEMOVE:
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
		case WM_RBUTTONDBLCLK:
			if(this->bMinimized)
			{
				this->bHidden = false;
				this->bMinimized = false;
				ShowWindow(SW_SHOW); // Triggers WM_ACTIVATE
				ShowWindow(SW_RESTORE);
			}
			break;
	}

	return 1;
}




void CFreezeKeyboardLayoutDlg::OnCbnSelchangeDroplistKeyboardLayouts()
{
	CString strText;
	m_dropListKeyboardLayouts.GetLBText(m_dropListKeyboardLayouts.GetCurSel(), strText);

	m_strKeyboadLayout = strText.Mid(strText.GetLength() - 8);

	LoadKeyboardLayout(m_strKeyboadLayout, KLF_ACTIVATE | KLF_NOTELLSHELL | KLF_REORDER | KLF_REPLACELANG);

	m_regkeyFreezeKeyboardLayout.WriteString((CString)"KLID", m_strKeyboadLayout);
}



void CFreezeKeyboardLayoutDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);

	FixKLID();
}
