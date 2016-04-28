#pragma once

extern CBooguNoteConfig g_config;
// AddFileLink dialog

class CAddFileLink : public CDialogImpl<CAddFileLink>,
	public CWinDataExchange<CAddFileLink>
{
public:
	enum { IDD = IDD_DIALOG_ADD_FILE_LINK };

	CEditImpl   m_wndSearchDir;
	CString		m_strSearchDir;


	// Construction
	CAddFileLink()
	{
	};

	~CAddFileLink()
	{
	};

	// Maps
	BEGIN_MSG_MAP(CAddFileLink)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_HANDLER(IDC_BUTTON_BROWSE_FILE, BN_CLICKED, OnBnClickedButtonBrowse)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CAddFileLink)
		DDX_CONTROL(IDC_EDIT_SEARCH, m_wndSearchDir)
		DDX_TEXT(IDC_EDIT_SEARCH, m_strSearchDir)
	END_DDX_MAP()

	LRESULT    OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DoDataExchange(true);
		PathAddBackslash(m_strSearchDir.GetBuffer(MAX_PATH));
		m_strSearchDir.ReleaseBuffer();
		DWORD hFillAtt = GetFileAttributes(m_strSearchDir);
		if (FILE_ATTRIBUTE_DIRECTORY == hFillAtt || PathIsRoot(m_strSearchDir))
		{
			EndDialog(wID);
			return IDOK;
		}
		else
		{
			MessageBox( _T("搜索路径不正确"), _T("警告"), MB_OK|MB_ICONEXCLAMATION);
		}
	}
	LRESULT    OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return IDCANCEL;
	}

	LRESULT	   OnBnClickedButtonBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CFolderDialog fd(NULL, NULL, BIF_USENEWUI);
		if (IDOK == fd.DoModal())
		{
			LPITEMIDLIST pidlTarget	= fd.GetSelectedItem();
			TCHAR pszPath[65536];
			SHGetPathFromIDList(pidlTarget, pszPath);   // Make sure it is a path
			PathAddBackslash(pszPath);
			m_strSearchDir = pszPath;
			m_wndSearchDir.SetWindowText(m_strSearchDir);
		}

		return 0;
	}

	BOOL OnInitDialog ( HWND hwndFocus, LPARAM lParam )
	{
		DoDataExchange(false);
		return TRUE;
	}
};
