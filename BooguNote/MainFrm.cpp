// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"	
#include "shlwapi.h"
#include "resource.h"
#include <assert.h>
#include <algorithm>
#include <time.h>

#include "aboutdlg.h"
#include "BooguNoteView.h"
#include "MainFrm.h"
#include "BooguNoteConfig.h"
#include "filedialogfilter.h"
#include "SearchDlg.h"
#include "Common.h"
#include "PreferenceDlg.h"
#include "BooguNoteHandle.h"
#include "BooguNoteIcon.h"
#include "PasteDlg.h"
#include "NotifyWnd.h"
#include "DesktopDlg.h"
#include "FileTreeView.h"
#include "ShareWnd.h"

extern CBooguNoteConfig g_config;
extern TCHAR strExePath[MAX_PATH+10];

//int g_nMainFrameState = 0;

UINT CF_HTML = 0;
UINT _CF_RTF = 0;

CSearchDlg* g_pSearchDlg = NULL;
CString g_SearchDir = _T("");
CString g_SearchWords = _T("");

CMainFrame::CMainFrame()
{
	m_pCurrView = NULL;
	m_nCaptionHeight    = GetSystemMetrics(SM_CXSIZEFRAME)*2 + -PointsToLogical(9)*1;
	m_nSizeFrameBorderX = GetSystemMetrics(SM_CXSIZEFRAME); //hunter : 框架可以缩放的厚度  参见 GetSystemMetrics  https://msdn.microsoft.com/en-us/library/windows/desktop/ms724385%28v=vs.85%29.aspx
	m_nSizeFrameBorderY = GetSystemMetrics(SM_CYSIZEFRAME); //hunter : 框架可以缩放的厚度  参见 GetSystemMetrics  https://msdn.microsoft.com/en-us/library/windows/desktop/ms724385%28v=vs.85%29.aspx
	m_nTopOfView = m_nCaptionHeight;
	m_nHeightOfTabPanel = 7;
	m_bShrinkTabList = true;
	m_menuCaption.m_hMenu = NULL;
	m_haccelerator = NULL;
	m_nTimerID = 100;
	m_pwndNotify = NULL;
	m_nCaptionHistoryCursor = 0;
}

void CMainFrame::LoadAccel()
{
	if(NULL!=m_haccelerator)
	{
		DestroyAcceleratorTable(m_haccelerator);
	}
	m_haccelerator = CreateAcceleratorTable(g_config.AccelMainFrm, g_config.nAccelMainFrmCount);
}

LRESULT CMainFrame::OnPrevFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int index=0;
	int nCurrView = GetActiveView();
	if (nCurrView>=0 && m_ActiveFileChain.size()>0)
	{	    
	    if(g_config.nCAPTION_CHANGE_RULE == 1)
	    {
		    //hunter : 打开记录只有一个或者已经到第一个, 则无上一个打开的文件直接返回
		    if( 0 == m_nCaptionHistoryCursor)
		    {
		        return 0;
		    }
		    else
		    {
		        //查找打开视图链表中的上一个视图编号			
			    m_nCaptionHistoryCursor--;
			    for (; index<m_viewList.size(); index++)
			    {
			    	if (m_ActiveFileChain[m_nCaptionHistoryCursor] == m_viewList[index])
			    	{
			    		break;
			    	}
			    }
		    }	        
        }
        else
        {			
			index = (nCurrView-1+ m_viewList.size())%int(m_viewList.size());			
        }
		if ((index>=0)&&(index<m_viewList.size()))
		{
			int nActiveWindowIndex = GetActiveView();
			UINT attr = 0;
			if (nActiveWindowIndex != -1)
			{

				m_viewList[nActiveWindowIndex]->ShowWindow(SW_HIDE);
				m_viewList[nActiveWindowIndex]->m_bActive = false;
			}
			m_viewList[index]->m_bActive = true;
			m_viewList[index]->ShowWindow(SW_SHOW);
			m_viewList[index]->SetActiveWindow();
			m_viewList[index]->SetFocus();

			m_pCurrView = m_viewList[index];
			SendMessage(m_hWnd, WM_NCPAINT, 0, 0);
		}         
	}	
	WriteBooguNoteIni();
	return 0;
}

LRESULT CMainFrame::OnNextFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nCurrView = GetActiveView();
	if (nCurrView>=0 && m_ActiveFileChain.size()>0)
	{
	    int index=0;
	    if(g_config.nCAPTION_CHANGE_RULE == 1)
	    {

		    //hunter : 如果是最后一个, 则无下一个打开的文件直接返回
		    if( m_ActiveFileChain.size() == (m_nCaptionHistoryCursor+1))
		    {
		        return 0;
		    }
		    else
		    {
		        //查找打开视图链表中的上一个视图编号			
			    m_nCaptionHistoryCursor++;
			    for (; index<m_viewList.size(); index++)
			    {
			    	if (m_ActiveFileChain[m_nCaptionHistoryCursor] == m_viewList[index])
			    	{
			    		break;
			    	}
			    }
		    }	                   
        }
        else
        {
            index = (nCurrView+1)%int(m_viewList.size());
        }
        if ((index>=0)&&(index<m_viewList.size()))
        {
            int nActiveWindowIndex = GetActiveView();
            UINT attr = 0;
            if (nActiveWindowIndex != -1)
            {

                m_viewList[nActiveWindowIndex]->ShowWindow(SW_HIDE);
                m_viewList[nActiveWindowIndex]->m_bActive = false;
            }
            m_viewList[index]->m_bActive = true;
            m_viewList[index]->ShowWindow(SW_SHOW);
            m_viewList[index]->SetActiveWindow();
            m_viewList[index]->SetFocus();

            m_pCurrView = m_viewList[index];
        }
        SendMessage(m_hWnd, WM_NCPAINT, 0, 0);        
	}
	WriteBooguNoteIni();
	return 0;
}

void CMainFrame::SaveAll()
{
	for (int i=0; i<m_viewList.size(); ++i)
	{
		if (m_viewList[i]->m_bDirty)
		{
			m_viewList[i]->Save();
		}
	}
}

void CMainFrame::Save()
{
	int nCurrActiveView = GetActiveView();
	if (nCurrActiveView<0)
    {
        return;
    }
    m_viewList[nCurrActiveView]->Save(); 
}

void CMainFrame::SaveAs()
{
	int nCurrActiveView = GetActiveView();
	if (nCurrActiveView<0)
    {
        return;
    }
    m_viewList[nCurrActiveView]->SaveAs(); 
}

LRESULT CMainFrame::OnSaveAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SaveAll();
	return 0;
}

LRESULT CMainFrame::OnSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	Save();
	return 0;
}

LRESULT CMainFrame::OnCaptureSetting(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nCurrActiveView = GetActiveView();
	if (nCurrActiveView<0)
    {
        return 0;
    }
    m_viewList[nCurrActiveView]->CaptureSetting(); 

	
	return 0;
}


LRESULT CMainFrame::OnSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SaveAs();
	return 0;
}

LRESULT CMainFrame::OnPopFileList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PopFileList();
	return 0;
}


LRESULT CMainFrame::OnCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseAll(false);
	return 0;
}

LRESULT CMainFrame::OnOpenStorageRoot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(g_config.szRootStorageDir))
	{
		ShellExecute(NULL, _T("open"), _T("explorer.exe"), g_config.szRootStorageDir, NULL, SW_SHOWNORMAL);
	}
	return 0;
}

bool CMainFrame::CloseAll(bool record)
{
	bool bIsCanceled = false;
	vector<CBooguNoteView*> tempList = m_viewList;

	WritePrivateProfileSection(_T("RestoreFiles"), _T(""), g_config.strConfigFileName);

	int iActiveView = -1;
	for (int i=0, j=0, k=0; i<tempList.size(); ++i)
	{
		if (tempList[i]->m_bDirty)
		{
			CString msg;
			CString fileName;
			if (tempList[i]->GetFileName() == _T(""))
				fileName = _T("(未命名)");
			else
				fileName = tempList[i]->GetFileName();
			msg.Format(_T("\"%s\" 即将关闭，你想保存它么？"), fileName);
			int re = MessageBox(msg, _T("警告"), MB_YESNOCANCEL|MB_ICONWARNING);
			if (IDCANCEL == re)
			{
				bIsCanceled = true;
				break;
			}
			else
			{
				if (IDYES == re)
				{
					m_viewList[i]->Save();
				}
				else if (IDNO == re)
				{
				}
			}
		}
		if (record)
		{
			if (_T('\0') != g_config.szRootStorageDir[0])
			{
				CString tempStorageRootStr(g_config.szRootStorageDir);
				tempStorageRootStr.MakeLower();
				if (_T('\\')!=tempStorageRootStr[tempStorageRootStr.GetLength()-1])
				{
					tempStorageRootStr += _T("\\");
				}
				CString tempfileDirectory = m_viewList[i]->m_fileDirectory;
				tempfileDirectory.MakeLower();
				if (0 == tempfileDirectory.Find(tempStorageRootStr))
				{
					
					m_viewList[i]->m_fileDirectory.Delete(0, tempStorageRootStr.GetLength()-1);//-1 for '\'
				}
			}
			if(!m_viewList[i]->m_fileDirectory.IsEmpty())
			{
		        if(i== GetActiveView())
		        {
		            iActiveView = k;
		        }			    
			    CString index;
				index.Format(_T("%d"),k);
				k++;
				WritePrivateProfileString(_T("RestoreFiles"),index.GetBuffer(256),m_viewList[i]->m_fileDirectory,g_config.strConfigFileName);
				index.ReleaseBuffer();
				++j;
			}			
			
		}

	}
	if (bIsCanceled)
	{
		return false;
	}
	else
	{
		TCHAR buf[1024];
		int nIndex = GetActiveView();
		if(-1 != nIndex && !m_viewList[nIndex]->m_fileDirectory.IsEmpty())
		{
		    _itot(iActiveView, buf, 10);
		}
		else
		{
            _itot(0, buf, 10);
		}
		
		WritePrivateProfileString(_T("RestoreFiles"),_T("ActiveView"),buf,g_config.strConfigFileName);
		m_viewList.clear();
		for (int i=0; i<tempList.size(); ++i)
		{
			tempList[i]->DestroyWindow();
			delete tempList[i];
		}
		SendMessage(WM_NCPAINT);
		return true;
	}
}

LRESULT CMainFrame::OnSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (NULL == g_pSearchDlg)
	{
		//m_bEnableNcActive = TRUE;
		g_pSearchDlg = new CSearchDlg();
		CString strDir(g_config.szRootStorageDir); 
		g_pSearchDlg->m_searchDir=strDir;
		g_pSearchDlg->m_searchWords = g_SearchWords;
		g_pSearchDlg->Create(this->m_hWnd, IDD_DIALOG_SEARCH);
		g_pSearchDlg->ShowWindow(SW_SHOW);
	}
	return 0;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{

	if(m_haccelerator != NULL)
    {
        if(::TranslateAccelerator(m_hWnd, m_haccelerator, pMsg))
            return TRUE;
    }
	if (m_FileTV.IsWindow())
	{
		HWND hFocusWindow = GetFocus();
		if (m_FileTV.m_Org.m_hWnd == hFocusWindow)
		{
			return m_FileTV.PreTranslateMessage(pMsg);
		}
	}
	int index = GetActiveView();
	if ((g_pSearchDlg != NULL) && (g_pSearchDlg->IsDialogMessageW(pMsg)))
	{
		switch (pMsg->message)
		{		
		case WM_USER_SEARCHDLG_CLOSED:
			if (g_pSearchDlg!=NULL)
			{
				g_SearchDir = g_pSearchDlg->m_searchDir;
				g_SearchWords = g_pSearchDlg->m_searchWords;
				g_pSearchDlg->DestroyWindow();
				delete g_pSearchDlg;
				g_pSearchDlg = NULL;
			}
			break;
		case WM_LBUTTONDBLCLK:
			{
				if (((CListBox)(g_pSearchDlg->GetDlgItem(IDC_LIST_REPORT))).GetCount()>0)
				{
					int index = ((CListBox)(g_pSearchDlg->GetDlgItem(IDC_LIST_REPORT))).GetCurSel();
					if (-1 != index)
					{
						ItemData* pItemData = (ItemData*)((CListBox)(g_pSearchDlg->GetDlgItem(IDC_LIST_REPORT))).GetItemDataPtr(index);
						if (pItemData != NULL)
						{
							int nViewIndex = OpenFileByName(pItemData->filename.GetBuffer(pItemData->filename.GetLength()));
							if (-1 != nViewIndex)
							{
								WriteBooguNoteIni();
								CBooguNoteView* pCurrView = m_viewList[nViewIndex];
								if (pItemData->lineNumber!=-1)
								{
									//expand hiden text block.
									if (!pCurrView->m_textBlockList[pItemData->lineNumber]->m_bShown)
									{
										int i = pItemData->lineNumber;
										for (; i>=0; --i)
										{
											pCurrView->ExpandBlocks(i);
											if (pCurrView->m_textBlockList[i]->m_bShown)
											{
												break;
											}
										}
									}
									pCurrView->m_nLastClicked = pItemData->lineNumber;
									pCurrView->m_pTextBlockFocused = pCurrView->m_textBlockList[pItemData->lineNumber];
									pCurrView->m_pTextBlockFocused->TxWindowProc(m_hWnd, WM_SETFOCUS, 0, 0);

									//move text block to the view center.
									CRect rcWindow;
									pCurrView->GetWindowRect(rcWindow);
									int nOffsetY = pCurrView->m_pTextBlockFocused->GetClientRect()->top - rcWindow.Height()/2;
									if (nOffsetY<0)
										nOffsetY = 0;
									else if (nOffsetY>(pCurrView->m_nVRange-rcWindow.Height()))
										nOffsetY = pCurrView->m_nVRange-rcWindow.Height();
									pCurrView->m_ptOffset.y = nOffsetY;
									pCurrView->CalcScrollBar();
									pCurrView->OptimizeSendWM_PAINT();
								}
							}
						}
					}
				}
			}
			break;
		default:
			break;
		}
		return TRUE;
	}

	if (-1 != index && GetFocus() == m_viewList[index]->m_hWnd)
		return m_viewList[index]->PreTranslateMessage(pMsg);
	else
		return FALSE;
}

int CMainFrame::GetActiveView()
{
	for (int i=0; i < m_viewList.size(); ++i)
	{
		if (m_viewList[i]->m_bActive)
			return i;
	}
	return -1;
}


CRect CMainFrame::GetTabRectByOrderNumber(int nOrder)
{
	CRect rc;
	CRect rcWindow;
	GetWindowRect(rcWindow);
	rc.top = (g_config.nTabBorder*2-PointsToLogical(g_config.tabFontSize))*nOrder + m_nCaptionHeight+1;
	rc.bottom = (g_config.nTabBorder*2-PointsToLogical(g_config.tabFontSize))*(nOrder+1) + m_nCaptionHeight+1;
	rc.left = m_nSizeFrameBorderY;
	rc.right = rcWindow.Width()-m_nSizeFrameBorderY;
	return rc;
}

BOOL CMainFrame::OnIdle()
{
	return FALSE;
}

void CMainFrame::UpdateLayout(BOOL bResizeBars)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	CRect rcTemp = rcClient;
	ClientToScreen(&rcTemp);
	CRect rcWindow;
	GetWindowRect(&rcWindow);
	int nCaptionHeight = rcTemp.top - rcWindow.top;
	int nTop = m_nTopOfView - nCaptionHeight;
	
	for (int i=0; i<m_viewList.size(); ++i)
	{
		m_viewList[i]->MoveWindow(rcClient.left, nTop,	rcClient.Width(), rcClient.Height() - nTop, FALSE);
	}
}

LRESULT CMainFrame::OnMainFrmRefresh(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	int nActive = GetActiveView();
	vector<CString> fileNames;
	for (int i=0; i<m_viewList.size(); ++i)
	{
		fileNames.push_back(m_viewList[i]->m_fileDirectory);
	}
	CloseAll(false);
	for (int i=0; i<fileNames.size(); ++i)
	{
		OpenFileByName(fileNames[i].GetBuffer(MAX_PATH));
		fileNames[i].ReleaseBuffer();
	}
	ActivePage(nActive);
	WriteBooguNoteIni();
	return 0;
}
int CMainFrame::ShowCaptionMenu()
{
    if (m_menuCaption.m_hMenu == NULL)
    {
    	if (!m_menuCaption.LoadMenu(IDR_CAPTION_MENU))
    	{
    		ATLTRACE(_T("Menu resource was not loaded successfully!\n"));
    		return 0;
    	}
    
    	// TrackPopupMenu cannot display the menu bar so get 
    	// a handle to the first shortcut menu. 
    	CMenuHandle menuPopup = m_menuCaption.GetSubMenu(0);
    	UpdateMenuKeys(menuPopup.m_hMenu);
    	if (menuPopup.m_hMenu == NULL)
    	{
    		ATLTRACE(_T("Submenu was not retrieved successfully!\n"));
    		return 0;
    	}
    
    	// Display the shortcut menu
    	
    	CRect rcWindow;
    	GetWindowRect(rcWindow);
        POINT pt;  
        GetCursorPos(&pt); 
    
    	if (FRAME_STATE_NORMAL == m_nFrameState)
    	{
    		menuPopup.CheckMenuItem(ID_CAPTION_NORMAL_WINDOW, MF_BYCOMMAND|MF_CHECKED);
    	}
    	else
    	{
    		menuPopup.CheckMenuItem(ID_CAPTION_NORMAL_WINDOW, MF_BYCOMMAND|MF_UNCHECKED);
    	}
    	if (FRAME_STATE_DOCK == m_nFrameState)
    	{
    		menuPopup.CheckMenuItem(ID_CAPTION_DOCK, MF_BYCOMMAND|MF_CHECKED);
    	}
    	else
    	{
    		menuPopup.CheckMenuItem(ID_CAPTION_DOCK, MF_BYCOMMAND|MF_UNCHECKED);
    	}
    	if (FRAME_STATE_DOCK_AUTOHIDE == m_nFrameState)
    	{
    		menuPopup.CheckMenuItem(ID_CAPTION_AUTOHIDE, MF_BYCOMMAND|MF_CHECKED);
    	}
    	else
    	{
    		menuPopup.CheckMenuItem(ID_CAPTION_AUTOHIDE, MF_BYCOMMAND|MF_UNCHECKED);
    	}
    
    	if (!menuPopup.TrackPopupMenu(TPM_LEFTALIGN|TPM_TOPALIGN|TPM_LEFTBUTTON,
    		(int)pt.x, (int)pt.y, m_hWnd))
    	{
    		ATLTRACE(_T("Context menu was not displayed successfully!\n"));
    		return 0;
    	}
    
    	// Destroy the menu and free any memory that the menu occupies
    	menuPopup.DestroyMenu();
    	m_menuCaption.DestroyMenu();
    	m_menuCaption.m_hMenu = NULL;
    }    
}
void CMainFrame::WriteRegistry()
{
	HKEY hBooTest;
	LONG lr = RegOpenKey(HKEY_CLASSES_ROOT, _T(".boo"), &hBooTest);
	if (ERROR_ACCESS_DENIED != lr)
	{
		HKEY hDotBoo;
		RegCreateKey(HKEY_CLASSES_ROOT, _T(".boo"), &hDotBoo);
		TCHAR szAppName[] = _T("BooguStudio.BooguNote.1");
		RegSetValue(hDotBoo, NULL, REG_SZ, szAppName, (lstrlen(szAppName) + 1)*sizeof(TCHAR));

		HKEY hPerceivedType;
		RegCreateKey(hDotBoo, _T("PerceivedType"), &hPerceivedType);
		TCHAR szPerceivedType[] = _T("Text");
		RegSetValue(hPerceivedType, NULL, REG_SZ, szPerceivedType, (lstrlen(szPerceivedType) + 1)*sizeof(TCHAR));
		RegCloseKey(hPerceivedType);

		HKEY hContentType;
		RegCreateKey(hDotBoo, _T("Content Type"), &hContentType);
		TCHAR szContentType[] = _T("text/BooguNote");
		RegSetValue(hContentType, NULL, REG_SZ, szContentType, (lstrlen(szContentType) + 1)*sizeof(TCHAR));
		RegCloseKey(hContentType);
		RegFlushKey(HKEY_CLASSES_ROOT);
		RegCloseKey(hDotBoo);

		HKEY hAppName;
		RegCreateKey(HKEY_CLASSES_ROOT, szAppName, &hAppName);
		TCHAR szAppNameDefault[] = _T("BooguNote File");
		RegSetValue(hAppName, NULL, REG_SZ, szAppNameDefault, (lstrlen(szAppNameDefault) + 1)*sizeof(TCHAR));

		HKEY hShell;
		RegCreateKey(hAppName, _T("shell"), &hShell);
		HKEY hOpen;
		RegCreateKey(hShell, _T("open"), &hOpen);
		HKEY hCommand;
		RegCreateKey(hOpen, _T("command"), &hCommand);

		TCHAR strBuf[MAX_PATH+100];
		_stprintf(strBuf, _T("\"%s\" \"%%1\""), strExePath);
		RegSetValue(hCommand, NULL, REG_SZ, strBuf, (lstrlen(strBuf) + 1)*sizeof(TCHAR));
		RegCloseKey(hCommand);
		RegCloseKey(hOpen);
		RegCloseKey(hShell);

		HKEY hDefaultIcon;
		RegCreateKey(hAppName, _T("DefaultIcon"), &hDefaultIcon);
		TCHAR strTempExePath[MAX_PATH+10];
		lstrcpy(strTempExePath, strExePath);
		lstrcat(strTempExePath, _T(",0"));
		RegSetValue(hDefaultIcon, NULL, REG_SZ, strTempExePath, (lstrlen(strTempExePath) + 1)*sizeof(TCHAR));
		RegCloseKey(hDefaultIcon);

		DWORD dwType;
		TCHAR szCurVer[] = _T("BooguStudio.BooguNote.1");
		TCHAR szOldCurVer[1024];
		DWORD nSize = 1024;
		SHGetValue(hAppName, _T("CurVer"), NULL, &dwType,szOldCurVer, &nSize);
		if (_tcscmp(szOldCurVer, szCurVer)!=0)
		{
			HKEY hCurVer;
			RegCreateKey(hAppName, _T("CurVer"), &hCurVer);
			
			RegSetValue(hCurVer, NULL, REG_SZ, szCurVer, (lstrlen(szCurVer) + 1)*sizeof(TCHAR));
			RegCloseKey(hCurVer);
			SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
		}

		RegFlushKey(hAppName);
		RegCloseKey(hAppName);
	}

}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = TRUE;
	// remove old menu
	SetMenu(NULL);
	//hunter : https://msdn.microsoft.com/zh-cn/library/0xhz2t1c.aspx
	//hunter : 此处会导致任务栏上也没有系统关闭菜单 https://msdn.microsoft.com/zh-cn/library/ms632600.aspx
	ModifyStyle(WS_SYSMENU, 0); //hunter : 屏蔽系统默认右键菜单  2015-3-1 14:40:33
	                             

	SetTrayIcon(true);
	CF_HTML = RegisterClipboardFormat(_T("HTML Format"));

	LoadAccel();
	DragAcceptFiles();
	
	InitAppBar(APPBAR_DOCKING_ALL, false, true);
	

	if (g_szCommandLineFile.IsEmpty()==FALSE)
	{
		m_nFrameState = FRAME_STATE_NORMAL;
		if (_T(':') == g_szCommandLineFile[1] && INVALID_FILE_ATTRIBUTES != GetFileAttributes(g_szCommandLineFile))
		{
			CBooguNoteView* pView = new CBooguNoteView();
			pView->m_fileDirectory = g_szCommandLineFile;
			m_hWndClient = pView->Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE /*| WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL*//* , WS_EX_CLIENTEDGE*/);
			pView->m_bActive = true;
			m_viewList.push_back(pView);
			m_nTopOfView = GetTabRectByOrderNumber(0).bottom;
			UpdateLayout();
			SendMessage(WM_NCPAINT,0,0);
		}
	}
	else
	{
		m_nFrameState = g_config.FrameState;
		if (FRAME_STATE_DOCK == m_nFrameState)
		{
			Dock();
		}
		else if ( FRAME_STATE_DOCK_AUTOHIDE == m_nFrameState)
		{
			if (g_config.nPosX>=0 && g_config.nPosY>=0 && g_config.nWidth >= 30 && g_config.nHeight >=30)
			{
				CRect rect;
				rect.left = g_config.nPosX;
				rect.top = g_config.nPosY;
				rect.right = rect.left+g_config.nWidth;
				rect.bottom = rect.top+g_config.nHeight;
				MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());
			}
			
			SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |  SWP_FRAMECHANGED);
			ShowWindow(SW_HIDE);
			ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW|WS_EX_TOPMOST);
			ShowWindow(SW_SHOW);
			SetTrayIcon(true);
			SetKeepSize(true);
			SetAutoHide(true);
			UINT uFlag = -1;
			if (DOCK_RIGHT == g_config.DockSide)
			{
				uFlag = APPBAR_DOCKING_RIGHT;
			}
			else if (DOCK_TOP == g_config.DockSide)
			{
				uFlag = APPBAR_DOCKING_TOP;
			}
			else if (DOCK_LEFT == g_config.DockSide)
			{
				uFlag = APPBAR_DOCKING_LEFT;
			}
			else if (DOCK_BOTTOM == g_config.DockSide)
			{
				uFlag = APPBAR_DOCKING_BOTTOM;
			}
			else
			{
				uFlag = APPBAR_DOCKING_TOP;
			}
			DockAppBar(uFlag);
			m_nFrameState = FRAME_STATE_DOCK_AUTOHIDE;
			
		}
		else
		{
			m_nFrameState = FRAME_STATE_NORMAL;
			if (g_config.nPosX>=0 && g_config.nPosY>=0 && g_config.nWidth >= 30 && g_config.nHeight >=30)
			{
				CRect rect;
				rect.left = g_config.nPosX;
				rect.top = g_config.nPosY;
				rect.right = rect.left+g_config.nWidth;
				rect.bottom = rect.top+g_config.nHeight;
				MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());
			}
		}
		

		TCHAR buf[MAX_PATH];
		for (int i=0; ;++i)
		{
			CString index;
			index.Format(_T("%d"), i);
			GetPrivateProfileString(_T("RestoreFiles"),index, _T(""),buf, MAX_PATH,g_config.strConfigFileName);
			if ('\0' == buf[0])
				break;
			else
			{
				if (_T('\\') == buf[0] && _T('\\') != buf[1])
				{
					TCHAR relDir[MAX_PATH*3];
					_stprintf(relDir, _T("%s%s"), g_config.szRootStorageDir, buf);
					OpenFileByName(relDir);
				}
				else
				{
					OpenFileByName(buf);
				}
			}
		}
		GetPrivateProfileString(_T("RestoreFiles"),_T("ActiveView"), _T("0"),buf, MAX_PATH,g_config.strConfigFileName);
		int nActiveView = _tstoi(buf);
		if (nActiveView>-1 && nActiveView<m_viewList.size())
		{
			ActivePage(nActiveView);
		}
	}

	GetSysParms();
	WriteRegistry();
	if(0 != g_config.nSaveAllElapse)
	{
	    SetTimer(m_nTimerID, g_config.nSaveAllElapse, NULL);
	}
	

	for(int i=0; i<g_config.nAccelGlobalCount; ++i)
	{
		RegisterHotKey(this->m_hWnd, g_config.AccelGlobal[i].cmd, g_config.AccelGlobal[i].fVirt, g_config.AccelGlobal[i].key);
	}
	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	return 0;
}

LRESULT CMainFrame::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	for (int i=0, j=0; i<m_viewList.size(); ++i)
	{
		if (_T("")!=m_viewList[i]->m_fileDirectory)
		{
			if (m_viewList[i]->m_bDirty)
			{
				m_viewList[i]->Save();
			}
		}
	}
	bHandled = FALSE;
	return 0;
}
int CMainFrame::WriteBooguNoteIni()
{
	WritePrivateProfileSection(_T("RestoreFiles"), _T(""), g_config.strConfigFileName);
	int nRootStorageDirLen = _tcsclen(g_config.szRootStorageDir);
	for (int i=0, j=0; i<m_viewList.size(); ++i)
	{
		if (_T("")!=m_viewList[i]->m_fileDirectory)
		{
			if (m_viewList[i]->m_bDirty)
			{
				m_viewList[i]->Save();
			}
			TCHAR strIndex[_MAX_PATH];
			_stprintf(strIndex,_T("%d"),j);
			CString tempDir = m_viewList[i]->m_fileDirectory;
			if (nRootStorageDirLen>0)
			{
				CString tempStr(g_config.szRootStorageDir);
				tempStr += _T("\\");
				if (0 == m_viewList[i]->m_fileDirectory.Find(tempStr))
				{
					m_viewList[i]->m_fileDirectory.Delete(0, nRootStorageDirLen);
				}
			}
			WritePrivateProfileString(_T("RestoreFiles"),strIndex,m_viewList[i]->m_fileDirectory,g_config.strConfigFileName);
			m_viewList[i]->m_fileDirectory = tempDir;
			++j;
		}
	}

	TCHAR buf[1024];
	int nIndex = GetActiveView();
	_itot(nIndex, buf, 10);
	WritePrivateProfileString(_T("RestoreFiles"),_T("ActiveView"),buf,g_config.strConfigFileName);

	//dock state
	g_config.FrameState = m_nFrameState;
	CRect rect;
	GetWindowRect(&rect);
	g_config.nWidth = rect.Width();
	g_config.nHeight = rect.Height();
	if (!IsZoomed() && !IsIconic())
	{
		g_config.nPosX = rect.left>0?rect.left:0;
		g_config.nPosY = rect.top>0?rect.top:0;
	}
	else
	{
		g_config.nPosX = 10;
		g_config.nPosY = 10;
	}

	if (APPBAR_DOCKING_RIGHT == m_uCurrentDockingSide)
	{
		g_config.DockSide = DOCK_RIGHT;
		
	}
	else if (APPBAR_DOCKING_TOP == m_uCurrentDockingSide)
	{
		if (FRAME_STATE_DOCK == m_nFrameState)
		{
			g_config.DockSide = DOCK_RIGHT;
		}
		else
		{
			g_config.DockSide = DOCK_TOP;
		}
		
	}
	else if (APPBAR_DOCKING_LEFT == m_uCurrentDockingSide)
	{
		g_config.DockSide = DOCK_LEFT;
		
	}
	else if (APPBAR_DOCKING_BOTTOM == m_uCurrentDockingSide)
	{
		if (FRAME_STATE_DOCK == m_nFrameState)
		{
			g_config.DockSide = DOCK_RIGHT;
		}
		else
		{
			g_config.DockSide = DOCK_BOTTOM;
		}
		
	}
	else
	{
		if (FRAME_STATE_DOCK == m_nFrameState)
		{
			g_config.DockSide = DOCK_RIGHT;
		}
		else
		{
			g_config.DockSide = DOCK_TOP;
		}
	}

	_itot(g_config.FrameState, buf, 10);
	WritePrivateProfileString(_T("MainFrame"),_T("FrameState"),buf,g_config.strConfigFileName);
	_itot(g_config.DockSide, buf, 10);
	WritePrivateProfileString(_T("MainFrame"),_T("DockSide"),buf,g_config.strConfigFileName);
	_itot(g_config.nPosX, buf, 10);
	WritePrivateProfileString(_T("MainFrame"),_T("PosX"),buf,g_config.strConfigFileName);
	_itot(g_config.nPosY, buf, 10);
	WritePrivateProfileString(_T("MainFrame"),_T("PosY"),buf,g_config.strConfigFileName);
	_itot(g_config.nWidth, buf, 10);
	WritePrivateProfileString(_T("MainFrame"),_T("Width"),buf,g_config.strConfigFileName);
	_itot(g_config.nHeight, buf, 10);
	WritePrivateProfileString(_T("MainFrame"),_T("Height"),buf,g_config.strConfigFileName);

	return 0;
}

void CMainFrame::WindowShown(bool bShown)
{
	if (!bShown)
	{
		if (FRAME_STATE_DOCK == m_nFrameState)
		{
			UnDock();
		}
		ShowWindow(SW_HIDE);
	}
	else
	{
		ShowWindow(SW_SHOW);
		
		if (m_bAutoHide)
		{
			BOOL b;
			__super::OnActivate(WM_ACTIVATE, WA_ACTIVE, 0, b);
			
			
		}

		SetForegroundWindow(this->m_hWnd);
		SendMessage(WM_NCPAINT);
		if (FRAME_STATE_DOCK == m_nFrameState)
		{
			Dock();
		}
	}
	if (m_FileTV.IsWindow())
	{
		if (bShown)
		{
			m_FileTV.ShowWindow(SW_SHOW);
		}
		else
		{
			m_FileTV.ShowWindow(SW_HIDE);
		}
	}
}

LRESULT CMainFrame::OnHotKey(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (BOOGUNOTE_HOTKEY_DOCK == wParam)
	{
		BOOL b = FALSE;
		OnDock(0,0,0,b);
	}
	if (BOOGUNOTE_HOTKEY_DOCK_AUTOHIDE == wParam)
	{
		BOOL b = FALSE;
		OnAutoHide(0,0,0,b);
	}
	else if (BOOGUNOTE_HOTKEY_FOCUSING== wParam)
	{
		//ActiveSelf();
		if (!IsWindowVisible())
		{
			WindowShown(true); //don't suprise, for resume window.
		}

		if (FRAME_STATE_DOCK != m_nFrameState && FRAME_STATE_DOCK_AUTOHIDE != m_nFrameState)
		{
			if (this->IsZoomed()||this->IsIconic())
			{
				ShowWindow(SW_RESTORE);
			}
			BringWindowToTop(); 
		}
		BOOL rt = SetForegroundWindow(this->m_hWnd);
		SetFocus();
		BOOL b;
		__super::OnActivate(WM_ACTIVATE, WA_ACTIVE, 0, b);
		//不能使用SetActiveWindow();和重载的OnActivate冲突
		SendMessage(WM_NCPAINT,0,0);

		int nCurrActiveView = GetActiveView();
		if (nCurrActiveView>=0)
		{
			if (NULL == m_viewList[nCurrActiveView]->m_pTextBlockFocused)
			{
				if (m_viewList[nCurrActiveView]->m_nFocusedTextBlockBeforeKillFocus < 0)
					m_viewList[nCurrActiveView]->m_nFocusedTextBlockBeforeKillFocus = 0;
				else if (m_viewList[nCurrActiveView]->m_nFocusedTextBlockBeforeKillFocus >= m_viewList[nCurrActiveView]->m_textBlockList.size())
					m_viewList[nCurrActiveView]->m_nFocusedTextBlockBeforeKillFocus = m_viewList[nCurrActiveView]->m_textBlockList.size()-1;
				m_viewList[nCurrActiveView]->m_pTextBlockFocused = m_viewList[nCurrActiveView]->m_textBlockList[m_viewList[nCurrActiveView]->m_nFocusedTextBlockBeforeKillFocus];
			}
			//for focusing
			BOOL t;
			DWORD tt = 0;
			m_viewList[nCurrActiveView]->OnLBottonDown(0,0,(LPARAM&)tt, t);
		}
		if (FRAME_STATE_DOCK != m_nFrameState && FRAME_STATE_DOCK_AUTOHIDE != m_nFrameState)
		{
			CRect rect;
			GetWindowRect(&rect);
			if (rect.left>::GetSystemMetrics(SM_CXSCREEN))
				rect.left = 20;
			if (rect.left<0)
				rect.left = 20;
			if (rect.top<0)
				rect.top = 20;
			
			SetWindowPos(HWND_NOTOPMOST, rect, /*SWP_NOMOVE | */ SWP_NOSIZE | SWP_FRAMECHANGED);
		}
	}
	else if (BOOGUNOTE_HOTKEY_HIDE== wParam)
	{
		if (IsWindowVisible())
		{
			WindowShown(false);
		}
		else
		{
			WindowShown(true);
			//for focusing
			BOOL t;
			DWORD tt = 0;
			int nCurrActiveView = GetActiveView();
			if (nCurrActiveView>=0)
			{
				m_viewList[nCurrActiveView]->OnLBottonDown(0,0,(LPARAM&)tt, t);
			}
		}
	}
	else if (BOOGUNOTE_HOTKEY_GLOBAL_PASET== wParam)
	{
		HWND hCurrGlobalActiveWindow = GlobalCopy();
		if (NULL!=hCurrGlobalActiveWindow && m_hWnd != hCurrGlobalActiveWindow)
		{
			if (IsClipboardFormatAvailable(CF_UNICODETEXT))
			{
				if (OpenClipboard())
				{
					HGLOBAL hMem = GetClipboardData(CF_UNICODETEXT); 
					if (hMem != NULL) 
					{
						TCHAR* pBuf = (TCHAR*)GlobalLock(hMem);
						int nCopiedLength = lstrlen(pBuf);
						TCHAR szToolTip[128];
						int nLongLen = 30;
						int nShortLen = 18;
						memset(szToolTip, '\0', sizeof(TCHAR)*128);
						_sntprintf(szToolTip, nLongLen, _T("%s"), pBuf);
						StrTrim(szToolTip, _T("\t\n\r "));
						//bool bShowLong = true;
						for (int i=0; i<nLongLen; i++)
						{
							if (WORD(szToolTip[i]) & 0xFF00)
							{
								//bShowLong = false;
								szToolTip[nShortLen] = _T('\0');
								break;
							}
						}
						lstrcat(szToolTip, _T("..."));
						TCHAR szToolTitle[128];
						_sntprintf(szToolTitle, 64, _T("获得 %d 字符："), nCopiedLength);
						TCHAR szNofity[128];
						_sntprintf(szNofity, 120, _T("%s\n%s"), szToolTitle, szToolTip);
						if(nCopiedLength!=0)
						{
							GlobalUnlock(hMem);
							CloseClipboard();
							int nCurrActiveView = GetActiveView();
							if (nCurrActiveView>=0)
							{
								if (NULL == m_viewList[nCurrActiveView]->m_pTextBlockFocused || !m_viewList[nCurrActiveView]->m_pTextBlockFocused->m_bShown)
								{
									for (int i=m_viewList[nCurrActiveView]->m_textBlockList.size()-1; i>=0; --i)
									{
										if (0 == m_viewList[nCurrActiveView]->m_textBlockList[i]->m_nLevel)
										{
											m_viewList[nCurrActiveView]->m_pTextBlockFocused = m_viewList[nCurrActiveView]->m_textBlockList[i];
											break;
										}
									}
								}
								TCHAR* Buf = NULL;
								int nBlockTextLength = 0;
								bool bCreateNew = false;
								if (TEXT_BLOCK_SHRINK==m_viewList[nCurrActiveView]->m_pTextBlockFocused->m_nExpandState)
								{
									bCreateNew = true;
								}
								else if (m_viewList[nCurrActiveView]->m_pTextBlockFocused->m_bFile)
								{
									bCreateNew = true;
								}
								else
								{
									BSTR  bstrCache;
									m_viewList[nCurrActiveView]->m_pTextBlockFocused->GetTextServices()->TxGetText(&bstrCache);
									nBlockTextLength = SysStringLen(bstrCache);
									Buf = new TCHAR[nBlockTextLength+1+1];//+1 for '-' and '+'
									_stprintf(Buf, _T("%s"), (LPCTSTR)bstrCache);
									if (lstrlen(Buf)>1)
									{
										bCreateNew = true;
									}
									SysFreeString(bstrCache);
								}
								if (bCreateNew)
									m_viewList[nCurrActiveView]->CreateNextBlock(false);
								delete []Buf;
								m_viewList[nCurrActiveView]->Paste();
								if (NULL != m_viewList[nCurrActiveView]->m_pTextBlockFocused)
								{
									m_viewList[nCurrActiveView]->m_pTextBlockFocused->DetectFileBlock();
								}
								
								m_viewList[nCurrActiveView]->CalcLayout();
								m_viewList[nCurrActiveView]->CalcScrollBar();
								if (NULL!= m_viewList[nCurrActiveView]->m_pTextBlockFocused && !m_viewList[nCurrActiveView]->m_pTextBlockFocused->m_bFile)
								{
									if (0 == g_config.nBlockStateAfterGlobalPaste)
									{
										m_viewList[nCurrActiveView]->StateNarrow();
									}
									else if (1 == g_config.nBlockStateAfterGlobalPaste)
									{
										m_viewList[nCurrActiveView]->StateWide();
									}
									else if (2 == g_config.nBlockStateAfterGlobalPaste)
									{
										m_viewList[nCurrActiveView]->StateShrink();
									}
								}

								if (g_config.bGlobalCopyWithDocUrl)
								{
									BSTR  bstrCache;
									if (NULL != m_viewList[nCurrActiveView]->m_pTextBlockFocused)
									{
										m_viewList[nCurrActiveView]->m_pTextBlockFocused->GetTextServices()->TxGetText(&bstrCache);
									}
									int length = SysStringLen(bstrCache);
									TCHAR* textBuf = new TCHAR[length+MAX_PATH*10];
									_stprintf(textBuf, _T("%s\r\n"), (LPCTSTR)bstrCache);

									//////////////////////////////////////////////////////////
									if (IsClipboardFormatAvailable(CF_HTML))
									{
										if (OpenClipboard())
										{
											HGLOBAL hMem = GetClipboardData(CF_HTML); 
											if (hMem != NULL) 
											{
												char* pBuf = (char*)GlobalLock(hMem);
												char  szSourceURL[] = "SourceURL";
												char* pSourceURLStart = strstr((char*)pBuf, szSourceURL);
												if (NULL != pSourceURLStart)
												{
													pSourceURLStart += strlen(szSourceURL);
													pSourceURLStart++;
													char* pSourceURLEnd = strstr(pSourceURLStart, "\r\n");
													if (NULL != pSourceURLEnd)
													{
														(*pSourceURLEnd) = '\0';
														pSourceURLStart--;
														(*pSourceURLStart)='\n';
														pSourceURLStart--;
														(*pSourceURLStart)='\r';
														int nUnicodeSourceURLLen = MultiByteToWideChar(CP_UTF8, 0, pSourceURLStart, -1,NULL, 0) ;
														
														TCHAR* pUnicodeSourceURLStart = new TCHAR[nUnicodeSourceURLLen+10];
														MultiByteToWideChar(CP_UTF8, 0, pSourceURLStart, -1,pUnicodeSourceURLStart, nUnicodeSourceURLLen+10) ;
														
														pUnicodeSourceURLStart[nUnicodeSourceURLLen] = _T('\0');
														lstrcat(textBuf, pUnicodeSourceURLStart);
													}
												}

												
												GlobalUnlock(hMem);
											}
											
											CloseClipboard();
										}
									}
									
									if (NULL != m_viewList[nCurrActiveView]->m_pTextBlockFocused)
									{
										m_viewList[nCurrActiveView]->m_pTextBlockFocused->GetTextServices()->TxSendMessage(WM_SETTEXT, 0, (LPARAM)textBuf, 0);
									}

								}
								m_viewList[nCurrActiveView]->CalcLayout();
								m_viewList[nCurrActiveView]->CalcScrollBar();

								ShowNotification(szNofity);
								

							}
						}
					}
					else
					{
						GlobalUnlock(hMem);
						CloseClipboard();
					}
				}
				else
				{
					CloseClipboard();
				}
			}
			else
			{
				int nCurrActiveView = GetActiveView();
				if (nCurrActiveView>=0)
				{
					m_viewList[nCurrActiveView]->Paste();
					
				}
				if (IsClipboardFormatAvailable(CF_HDROP))
				{
					if (NULL != m_viewList[nCurrActiveView]->m_pTextBlockFocused)
					{
						if (OpenClipboard())
						{
							GLOBALHANDLE    hGMem ;
							HDROP    hDrop;
							hGMem = GetClipboardData(CF_HDROP) ;
							hDrop = (HDROP)GlobalLock(hGMem);

							CString strNotify;
							strNotify.Format(L"粘贴了 %d 个文件", DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0));
							ShowNotification(strNotify.GetBuffer(128));
							strNotify.ReleaseBuffer();

							GlobalUnlock(hGMem) ;
							CloseClipboard();
						}
					}
				}
				
			}
			
		}
	}
	else if (BOOGUNOTE_HOTKEY_GLOBAL_PASET_TO_FILE== wParam)
	{
		HWND hCurrGlobalActiveWindow = GlobalCopy();
		if (NULL!=hCurrGlobalActiveWindow && m_hWnd != hCurrGlobalActiveWindow)
		{
			TCHAR textBuf[1024*4];
			::GetWindowText(hCurrGlobalActiveWindow, textBuf, 1024);
			SaveCopiedToFile(textBuf);
		}
	}
	else if (BOOGUNOTE_HOTKEY_GLOBAL_SCREENSHOT== wParam)
	{
		//Sleep(500);
		CDesktopDlg dlg;

		if(IDOK == dlg.DoModal())
		{
			int nCurrActiveView = GetActiveView();
			if (nCurrActiveView>=0)
			{
				m_viewList[nCurrActiveView]->Paste();
			}
			
		}
	}
	return 0;
}

void CMainFrame::SaveCopiedToFile(TCHAR* szTitle)
{
	CPasteDlg pasteDlg;
	//prepare PasteDlg content
	if (IsClipboardFormatAvailable(CF_UNICODETEXT))
	{
		if (OpenClipboard())
		{
			HGLOBAL hMem = GetClipboardData(CF_UNICODETEXT); 
			if (hMem != NULL) 
			{
				GlobalUnlock(hMem);
				CloseClipboard();
				char* pBuf = (char*)GlobalLock(hMem);
				pasteDlg.m_strContent = (TCHAR*)pBuf;
				if(pasteDlg.m_strContent.GetLength()!=0)
				{
					pasteDlg.m_strTitle = szTitle;
					if ( IDOK == pasteDlg.DoModal())
					{
						int nCurrActiveView = GetActiveView();
						if (nCurrActiveView>=0)
						{
							if (NULL == m_viewList[nCurrActiveView]->m_pTextBlockFocused || !m_viewList[nCurrActiveView]->m_pTextBlockFocused->m_bShown)
							{
								for (int i=m_viewList[nCurrActiveView]->m_textBlockList.size()-1; i>=0; --i)
								{
									if (0 == m_viewList[nCurrActiveView]->m_textBlockList[i]->m_nLevel)
									{
										m_viewList[nCurrActiveView]->m_pTextBlockFocused = m_viewList[nCurrActiveView]->m_textBlockList[i];
										break;
									}
								}
							}
							TCHAR* Buf = NULL;
							int nBlockTextLength = 0;
							bool bCreateNew = false;
							if (TEXT_BLOCK_SHRINK==m_viewList[nCurrActiveView]->m_pTextBlockFocused->m_nExpandState)
							{
								bCreateNew = true;
							}
							else if (m_viewList[nCurrActiveView]->m_pTextBlockFocused->m_bFile)
							{
								bCreateNew = true;
							}
							else
							{
								BSTR  bstrCache;
								m_viewList[nCurrActiveView]->m_pTextBlockFocused->GetTextServices()->TxGetText(&bstrCache);
								nBlockTextLength = SysStringLen(bstrCache);
								Buf = new TCHAR[nBlockTextLength+1+1];//+1 for '-' and '+'
								_stprintf(Buf, _T("%s"), (LPCTSTR)bstrCache);
								if (lstrlen(Buf)>1)
								{
									bCreateNew = true;
								}
								SysFreeString(bstrCache);
							}
							if (bCreateNew)
								m_viewList[nCurrActiveView]->CreateNextBlock(false);
							delete []Buf;
							
							m_viewList[nCurrActiveView]->m_pTextBlockFocused->GetTextServices()->TxSendMessage(WM_SETTEXT, 0, (LPARAM)pasteDlg.m_fileDir.GetBuffer(MAX_PATH*10), 0);
							pasteDlg.m_fileDir.ReleaseBuffer();
							
							m_viewList[nCurrActiveView]->m_pTextBlockFocused->DetectFileBlock();
							time(&(m_viewList[nCurrActiveView]->m_pTextBlockFocused->m_ModifyTime));

							m_viewList[nCurrActiveView]->m_bDirty = true;
							m_viewList[nCurrActiveView]->CalcLayout();
							m_viewList[nCurrActiveView]->CalcScrollBar();
						}
					}
				}
			}
			else
			{
				GlobalUnlock(hMem);
				CloseClipboard();
			}
		}
		else
		{
			CloseClipboard();
		}

		
	}
	
}


LRESULT CMainFrame::OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{

		UINT nFiles = DragQueryFile((HDROP)wParam, 0xFFFFFFFF, NULL, 0);
		TCHAR strFileName[65536];
		for (UINT i=0; i<nFiles; ++i)
		{
			DragQueryFile((HDROP)wParam, i, strFileName, 65536);
			OpenFileByName(strFileName);
		}
		WriteBooguNoteIni();

	bHandled = FALSE;
	return 0;
}

LRESULT CMainFrame::OnIconNotify(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch (lParam){
    case WM_LBUTTONDBLCLK:
		switch (wParam) {
			case IDR_MAINFRAME:
				WindowShown(true);
			break;
		}
		break;
	case WM_RBUTTONDOWN:
		{
            ShowCaptionMenu();
		}
		break;
	}

	return 0;
}

void CMainFrame::SetTrayIcon(bool bSet)
{
	
	DWORD dwMessage = NULL;
	if (bSet)
	{
		dwMessage = NIM_ADD;
	}
	else
	{
		dwMessage = NIM_DELETE;
	}
	CString sTip = _T("BooguNote ");
	sTip += BOOGUNOTE_VERSION;
	NOTIFYICONDATA tnd;
	tnd.cbSize = sizeof(NOTIFYICONDATA);
	tnd.hWnd = m_hWnd;
	tnd.uID = IDR_MAINFRAME;
	tnd.uFlags = NIF_MESSAGE|NIF_ICON;
	tnd.uCallbackMessage = WM_USER_ICONNOTIFY;
	tnd.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
	tnd.hIcon = LoadIcon(_Module.m_hInst, MAKEINTRESOURCE (IDI_ICON_TRAY));
	lstrcpyn(tnd.szTip, (LPCTSTR)sTip, sizeof(tnd.szTip));

	Shell_NotifyIcon(dwMessage, &tnd);

}

void CMainFrame::UnDock()
{
		CRect rcWindow;
		GetWindowRect(&rcWindow);
		g_config.nDockWidth  = rcWindow.Width();
		DockAppBar(APPBAR_DOCKING_NONE);

		ShowWindow(SW_HIDE);
		
		ModifyStyleEx(WS_EX_TOOLWINDOW|WS_EX_TOPMOST, WS_EX_APPWINDOW);
		ShowWindow(SW_SHOW);
		
		CRect rect;
		GetWindowRect(&rect);
		if (rect.left>::GetSystemMetrics(SM_CXSCREEN))
			rect.left = 20;
		if (rect.left<0)
			rect.left = 20;
		if (rect.top<0)
			rect.top = 20;
		SetWindowPos(HWND_NOTOPMOST, rect,/* SWP_NOMOVE |*/ SWP_NOSIZE | SWP_FRAMECHANGED);
		g_config.bTopmost = false;
		
}

void CMainFrame::Dock()
{
	UINT uFlag = -1;
	if (DOCK_RIGHT == g_config.DockSide)
	{
		uFlag = APPBAR_DOCKING_RIGHT;
	}
	else if (DOCK_LEFT == g_config.DockSide)
	{
		uFlag = APPBAR_DOCKING_LEFT;
	}
	else
	{
		uFlag = APPBAR_DOCKING_RIGHT;
	}

	if (this->IsZoomed()||this->IsIconic())
	{
		this->ShowWindow(SW_RESTORE); // if dock window when it's maximized, the window edge is unmoveable.
	}
	CRect rect;
	GetWindowRect(&rect);
	UINT nScreenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	if (nScreenWidth/2 < g_config.nWidth)
		g_config.nWidth = FRAME_DEFAULT_WIDTH;
	rect.right = rect.left+g_config.nWidth;
	SetWindowPos(HWND_TOPMOST, rect, SWP_NOMOVE | SWP_NOSIZE |  SWP_FRAMECHANGED);
	MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());
	
	
	ShowWindow(SW_HIDE);
	
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW|WS_EX_TOPMOST);
	ShowWindow(SW_SHOW);
	DockAppBar(uFlag);
	SetAutoHide(false);
	SetTrayIcon(true);
	
	UpdateLayout();
}

LRESULT CMainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	g_config.FrameState = m_nFrameState;
	CRect rect;
	GetWindowRect(&rect);
	g_config.nWidth = rect.Width();
	g_config.nHeight = rect.Height();
	if (!IsZoomed() && !IsIconic())
	{
		g_config.nPosX = rect.left>0?rect.left:0;
		g_config.nPosY = rect.top>0?rect.top:0;
	}
	else
	{
		g_config.nPosX = 10;
		g_config.nPosY = 10;
	}

	if (APPBAR_DOCKING_RIGHT == m_uCurrentDockingSide)
	{
		g_config.DockSide = DOCK_RIGHT;
		
	}
	else if (APPBAR_DOCKING_TOP == m_uCurrentDockingSide)
	{
		if (FRAME_STATE_DOCK == m_nFrameState)
		{
			g_config.DockSide = DOCK_RIGHT;
		}
		else
		{
			g_config.DockSide = DOCK_TOP;
		}
		
	}
	else if (APPBAR_DOCKING_LEFT == m_uCurrentDockingSide)
	{
		g_config.DockSide = DOCK_LEFT;
		
	}
	else if (APPBAR_DOCKING_BOTTOM == m_uCurrentDockingSide)
	{
		if (FRAME_STATE_DOCK == m_nFrameState)
		{
			g_config.DockSide = DOCK_RIGHT;
		}
		else
		{
			g_config.DockSide = DOCK_BOTTOM;
		}
		
	}
	else
	{
		if (FRAME_STATE_DOCK == m_nFrameState)
		{
			g_config.DockSide = DOCK_RIGHT;
		}
		else
		{
			g_config.DockSide = DOCK_TOP;
		}
	}

	_tcscpy(g_config.szFileManDir, m_FileTV.m_Org.GetRootFolder().c_str());
	PathAddBackslash(g_config.szFileManDir);

	if (m_FileTV.IsWindow())
	{
		CRect rcWindow;
		m_FileTV.GetWindowRect(&rcWindow);
		g_config.nFileManLeft = rcWindow.left;
		g_config.nFileManTop = rcWindow.top;
		g_config.nFileManWidth = rcWindow.Width();
		g_config.nFileManHeight = rcWindow.Height();
	}

	g_config.WriteConfigToIni();
	if (FRAME_STATE_DOCK == m_nFrameState || FRAME_STATE_DOCK_AUTOHIDE == m_nFrameState)
	{
		DockAppBar(APPBAR_DOCKING_NONE);
	}
	SetTrayIcon(false);


	if (CloseAll(true))
		bHandled = FALSE;
	else
		bHandled = TRUE;
	return 1;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	
	//m_bDocked = false;

	for(int i=0; i<g_config.nAccelGlobalCount; ++i)
	{
		UnregisterHotKey(this->m_hWnd, g_config.AccelGlobal[i].cmd);
	}
	
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	//return 1;
	return 0;
}

LRESULT CMainFrame::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CClientDC dc(m_hWnd);	
	bHandled = FALSE;
	return 0;
}

CRect CMainFrame::GetCloseButtonRect()
{
	CRect rcWindow;
	GetWindowRect(rcWindow); //hunter : 获取主窗口大小
	int nSideLength = m_nCaptionHeight - m_nSizeFrameBorderX*2;
	CRect rcCloseButton(rcWindow.Width()- m_nSizeFrameBorderY - nSideLength, m_nSizeFrameBorderX, rcWindow.Width()- m_nSizeFrameBorderY, m_nSizeFrameBorderX+nSideLength);
	rcCloseButton.DeflateRect(1,1,1,1);         //hunter : 左右前后缩小一个像素  12*12 -> 10*10
	rcCloseButton.MoveToY(rcCloseButton.top+1); //hunter : 整体在Y轴方向向下移动一个像素. 离边框 4像素之内, 鼠标会变成自由拖动图标. 留一个像素空间
	return rcCloseButton; //返回的是对应客户窗口左上点的相对位置的区域坐标
}

CRect CMainFrame::GetMaximizeButtonRect()
{
	CRect rcMax = GetCloseButtonRect();
	int nSideLength = m_nCaptionHeight - m_nSizeFrameBorderX*2;
	rcMax.MoveToX(rcMax.left -(nSideLength+m_nSizeFrameBorderY));
	return rcMax;
}

CRect CMainFrame::GetMinimizeButtonRect()
{
	CRect rcMin = GetMaximizeButtonRect();
	int nSideLength = m_nCaptionHeight - m_nSizeFrameBorderX*2;
	rcMin.MoveToX(rcMin.left -(nSideLength+m_nSizeFrameBorderY));
	return rcMin;
}


CRect CMainFrame::GetMenuButtonRect()
{
	CRect rcMenu = GetMinimizeButtonRect();
	int nSideLength = m_nCaptionHeight - m_nSizeFrameBorderX*2;
	rcMenu.MoveToX(rcMenu.left -(nSideLength+m_nSizeFrameBorderY));
	return rcMenu;
}

CPoint CMainFrame::GetCurrCurPos()
{
	CPoint pt;
	GetCursorPos(&pt);
	CRect rcWindow;
	GetWindowRect(rcWindow);
	pt.x -= rcWindow.left;
	pt.y -= rcWindow.top;
			
	return pt;
}

LRESULT CMainFrame::OnNcPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	//get current cursor position
	CPoint pt;
	GetCursorPos(&pt);
	CRect rcWindow;
	GetWindowRect(rcWindow);
	pt.x -= rcWindow.left;
	pt.y -= rcWindow.top;

	CWindowDC dcWindow(this->m_hWnd);
	CRect rect;
    GetWindowRect(rect);
	//int nHeadHeight = m_nTopOfView;

	CBrush brushSelectedTextBlockBackground;
	brushSelectedTextBlockBackground.CreateSolidBrush(g_config.clrSelectedTextBlockBackground);
	CBrush frameBrush;
	frameBrush.CreateSolidBrush(g_config.clrFrame);
	CPen framePen;
	framePen.CreatePen(PS_SOLID, 1, g_config.clrFrame);


	//draw frame
	dcWindow.SelectBrush(frameBrush);
	dcWindow.SelectPen(framePen);
	dcWindow.Rectangle(0, 0, rect.Width(), m_nCaptionHeight);// caption
	dcWindow.Rectangle(0, m_nCaptionHeight, m_nSizeFrameBorderX, rect.Height());//left
	dcWindow.Rectangle(rect.Width()-m_nSizeFrameBorderX, m_nCaptionHeight, rect.Width(), rect.Height());//right
	dcWindow.Rectangle(0,rect.Height()-m_nSizeFrameBorderY,rect.Width(),rect.Height());//bottom
	dcWindow.SelectPen((HPEN)GetStockObject(WHITE_PEN));
	dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
	dcWindow.Rectangle(m_nSizeFrameBorderX - 1, m_nCaptionHeight, rect.Width()-m_nSizeFrameBorderX+1,rect.Height() - m_nSizeFrameBorderY+1);

	//draw caption string
	dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
	CPen penFrameBorder;
	penFrameBorder.CreatePen(PS_SOLID, 1, g_config.clrFrameOuterBorder);
	dcWindow.SelectPen(penFrameBorder);
	//Outer border
	dcWindow.Rectangle(0,0,rect.Width(),rect.Height());
	dcWindow.SelectFont(g_config.hCaptionFont);
	dcWindow.SetTextColor(g_config.clrCaptionText);
	dcWindow.SetBkColor(g_config.clrFrame);

	dcWindow.TextOut(m_nSizeFrameBorderX, m_nSizeFrameBorderY, g_config.szCaptionString);


	//buttons on caption
	//caption close button
	CRect rcCaptionCloseButton = GetCloseButtonRect();
	rcCaptionCloseButton.DeflateRect(1,1,1,1);
	CPen penCaptionButton;
	penCaptionButton.CreatePen(PS_SOLID, 1, g_config.clrCaptionButton);
	dcWindow.SelectPen(penCaptionButton);
	//close button
	dcWindow.MoveTo(rcCaptionCloseButton.right-3, rcCaptionCloseButton.top+1);
	dcWindow.LineTo(rcCaptionCloseButton.left, rcCaptionCloseButton.bottom-2);
	dcWindow.MoveTo(rcCaptionCloseButton.left+1, rcCaptionCloseButton.top+2);
	dcWindow.LineTo(rcCaptionCloseButton.right-2, rcCaptionCloseButton.bottom-1);

	dcWindow.MoveTo(rcCaptionCloseButton.right-2, rcCaptionCloseButton.top+2);
	dcWindow.LineTo(rcCaptionCloseButton.left+1, rcCaptionCloseButton.bottom-1);
	dcWindow.MoveTo(rcCaptionCloseButton.left+2, rcCaptionCloseButton.top+1);
	dcWindow.LineTo(rcCaptionCloseButton.right-1, rcCaptionCloseButton.bottom-2);

	dcWindow.SelectPen(penCaptionButton);
	dcWindow.MoveTo(rcCaptionCloseButton.right-2, rcCaptionCloseButton.top+1);
	dcWindow.LineTo(rcCaptionCloseButton.left, rcCaptionCloseButton.bottom-1);
	dcWindow.MoveTo(rcCaptionCloseButton.left+1, rcCaptionCloseButton.top+1);
	dcWindow.LineTo(rcCaptionCloseButton.right-1, rcCaptionCloseButton.bottom-1);

	//caption maximize button
	CRect rcMaximizeButton = GetMaximizeButtonRect();
	rcMaximizeButton.DeflateRect(2,2,2,2);
	dcWindow.SelectPen(penCaptionButton);
	dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
	dcWindow.Rectangle(rcMaximizeButton);
	dcWindow.MoveTo(rcMaximizeButton.left,rcMaximizeButton.top+1);
	dcWindow.LineTo(rcMaximizeButton.right,rcMaximizeButton.top+1);

	//caption minimize button
	CRect rcMinimizeButton = GetMinimizeButtonRect();
	rcMinimizeButton.DeflateRect(2,2,2,2);
	dcWindow.SelectPen(penCaptionButton);
	dcWindow.MoveTo(rcMinimizeButton.left,rcMinimizeButton.top);
	dcWindow.LineTo(rcMinimizeButton.right,rcMinimizeButton.top);
	dcWindow.MoveTo(rcMinimizeButton.left,rcMinimizeButton.top+1);
	dcWindow.LineTo(rcMinimizeButton.right,rcMinimizeButton.top+1);
	
	//caption menu popup button.
	CRect rcMenuButton = GetMenuButtonRect();
	dcWindow.SelectPen(penCaptionButton);
	int pilot = (rcMenuButton.left+rcMenuButton.right)/2-1;
	int vpilot = (rcMenuButton.top+rcMenuButton.bottom)/2+2;
	for(int i=4; i>=0; --i)
	{
		dcWindow.MoveTo(pilot+i, vpilot-i);
		dcWindow.LineTo(pilot-i, vpilot-i);
	}
	

	
	
	int nCurrActiveView = GetActiveView();
	//CRect rcWindow;
	GetWindowRect(&rcWindow);
	if (m_viewList.size()>0)
	{
		//Draw tab height control panel.
		if (m_viewList.size()>1)
		{
			CBrush bushTabBackground;
			bushTabBackground.CreateSolidBrush(g_config.clrTabBackground);
			CPen penTabButton;
			penTabButton.CreatePen(PS_SOLID, 1, g_config.clrTabButton);
			//shrink state
			if (m_bShrinkTabList)
			{
				CString szFileName = m_viewList[nCurrActiveView]->GetFileName();
				if (m_viewList[nCurrActiveView]->m_bDirty)
					szFileName += _T("*");
				DrawTabHead(0, szFileName);
				
				CRect rc = GetTabRectByOrderNumber(0);

				//tab menu button
				dcWindow.SelectPen(penTabButton);
				dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
				int nSideLength = rc.Height() - g_config.nTabBorder*2;
				CRect rcTabMenuButton(rc.left+g_config.nTabBorder,rc.top+g_config.nTabBorder,rc.left+(nSideLength+g_config.nTabBorder),rc.top+(nSideLength+g_config.nTabBorder));

				int pilot = (rcTabMenuButton.left+rcTabMenuButton.right)/2-1;
				int vpilot = (rcTabMenuButton.top+rcTabMenuButton.bottom)/2+2;
				dcWindow.SelectPen(penTabButton);
				for(int i=4; i>=0; --i)
				{
					dcWindow.MoveTo(pilot+i, vpilot-i);
					dcWindow.LineTo(pilot-i, vpilot-i);
				}
				if (rcTabMenuButton.PtInRect(pt))
				{
					rcTabMenuButton.InflateRect(1,1,1,1);
					dcWindow.Draw3dRect(&rcTabMenuButton, RGB(255,255,255), g_config.clrTabButtonShade);
					rcTabMenuButton.DeflateRect(1,1,1,1);
				}



				//shrink button. bottom long button.
				dcWindow.SelectPen((HPEN)GetStockObject(WHITE_PEN));
				dcWindow.SelectBrush(bushTabBackground);
				
				dcWindow.Rectangle(rc.left, m_nTopOfView - m_nHeightOfTabPanel, rc.right, m_nTopOfView);
				pilot = (rc.left + rc.right)/2;
				dcWindow.SetPixel(pilot-2, m_nTopOfView-5, g_config.clrTabButton);
				dcWindow.SetPixel(pilot-1, m_nTopOfView-5, g_config.clrTabButton);
				dcWindow.SetPixel(pilot  , m_nTopOfView-5, g_config.clrTabButton);
				dcWindow.SetPixel(pilot+1, m_nTopOfView-5, g_config.clrTabButton);
				dcWindow.SetPixel(pilot+2, m_nTopOfView-5, g_config.clrTabButton);
				dcWindow.SetPixel(pilot-1, m_nTopOfView-4, g_config.clrTabButton);
				dcWindow.SetPixel(pilot  , m_nTopOfView-4, g_config.clrTabButton);
				dcWindow.SetPixel(pilot+1, m_nTopOfView-4, g_config.clrTabButton);			
				dcWindow.SetPixel(pilot  , m_nTopOfView-3, g_config.clrTabButton);
			}
			//unshrink state
			else
			{
				for (int i=0; i<m_viewList.size(); ++i)
				{
					CString szFileName = m_viewList[i]->GetFileName();
					if (m_viewList[i]->m_bDirty)
						szFileName += _T("*");
					DrawTabHead(i, szFileName);
				}
				HPEN oldPen = dcWindow.SelectPen(penTabButton);
				HBRUSH oldBrush = dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
				CRect rc = GetTabRectByOrderNumber(nCurrActiveView);
				int nSideLength = rc.Height() - g_config.nTabBorder*2;
				CRect rcTabMenuButton(rc.left+g_config.nTabBorder,rc.top+g_config.nTabBorder,rc.left+(nSideLength+g_config.nTabBorder),rc.top+(nSideLength+g_config.nTabBorder));
				
				int pilot = (rcTabMenuButton.left+rcTabMenuButton.right)/2;
				int vpilot = (rcTabMenuButton.top+rcTabMenuButton.bottom)/2-1;
				dcWindow.SelectPen(penTabButton);
				for(int i=4; i>=0; --i)
				{
					dcWindow.MoveTo(pilot-i, vpilot+i);
					dcWindow.LineTo(pilot-i, vpilot-i);
				}

				dcWindow.SelectBrush(oldBrush);
				dcWindow.SelectPen(oldPen);

				dcWindow.SelectPen((HPEN)GetStockObject(WHITE_PEN));
				dcWindow.SelectBrush(bushTabBackground);
				rc = GetTabRectByOrderNumber(0);
				dcWindow.Rectangle(rc.left, m_nTopOfView - m_nHeightOfTabPanel, rc.right, m_nTopOfView);
				pilot = (rc.left + rc.right)/2;
				dcWindow.SetPixel(pilot-2, m_nTopOfView-3, g_config.clrTabButton);
				dcWindow.SetPixel(pilot-1, m_nTopOfView-3, g_config.clrTabButton);
				dcWindow.SetPixel(pilot  , m_nTopOfView-3, g_config.clrTabButton);
				dcWindow.SetPixel(pilot+1, m_nTopOfView-3, g_config.clrTabButton);
				dcWindow.SetPixel(pilot+2, m_nTopOfView-3, g_config.clrTabButton);
				dcWindow.SetPixel(pilot-1, m_nTopOfView-4, g_config.clrTabButton);
				dcWindow.SetPixel(pilot  , m_nTopOfView-4, g_config.clrTabButton);
				dcWindow.SetPixel(pilot+1, m_nTopOfView-4, g_config.clrTabButton);			
				dcWindow.SetPixel(pilot  , m_nTopOfView-5, g_config.clrTabButton);
			}
		}
		else
		{
			CString szFileName = m_viewList[nCurrActiveView]->GetFileName();
			if (m_viewList[0]->m_bDirty)
				szFileName += _T("*");
			DrawTabHead(0, szFileName);
		}
		m_viewList[nCurrActiveView]->OptimizeSendWM_PAINT();
	}
	else
	{

		dcWindow.SelectPen((HPEN)GetStockObject(WHITE_PEN));
		dcWindow.SelectBrush((HBRUSH)GetStockObject(WHITE_BRUSH));
		dcWindow.Rectangle(m_nSizeFrameBorderX, m_nCaptionHeight,rcWindow.Width()-m_nSizeFrameBorderX,rcWindow.Height()-m_nSizeFrameBorderY);
	}

	CPen penFrameInnerBorder;
	penFrameInnerBorder.CreatePen(PS_SOLID, 1, g_config.clrFrameInnerBorder);
	dcWindow.SelectPen(penFrameInnerBorder);
	dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
	dcWindow.Rectangle(m_nSizeFrameBorderX-1, m_nCaptionHeight,rcWindow.Width()-m_nSizeFrameBorderX+1,rcWindow.Height()-m_nSizeFrameBorderY+1);
	bHandled = TRUE;
	return 0;
}

void CMainFrame::DrawTabHead(int index, CString str)
{
	CWindowDC dcWindow(this->m_hWnd);
	CBrush brushTabBackground;
	brushTabBackground.CreateSolidBrush(g_config.clrTabBackground);
	CPen penTabButton;
	penTabButton.CreatePen(PS_SOLID, 1, g_config.clrTabButton);
	CPen penTabBackground;
	penTabBackground.CreatePen(PS_SOLID, 1, g_config.clrTabBackground);
	CPen penTabBorder;
	penTabBorder.CreatePen(PS_SOLID, 1, g_config.clrTabBorder);

	
	dcWindow.SelectPen((HPEN)GetStockObject(WHITE_PEN));
	dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));

	CRect rc = GetTabRectByOrderNumber(index);
	dcWindow.Rectangle(rc);

	dcWindow.SelectPen(penTabBorder);
	dcWindow.SelectBrush(brushTabBackground);
	dcWindow.Rectangle(rc.left+1, rc.top+1, rc.right-1, rc.bottom-1);

	dcWindow.SetTextColor(g_config.clrTabCaptionText);
	dcWindow.SetBkColor(g_config.clrTabBackground);
	dcWindow.SelectFont(g_config.hTabFontBold);
	if (str == _T("*"))
		str = _T("(未命名)*");
	dcWindow.DrawTextW(str, -1, &rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);

	//tab close button
	int nSideLength = rc.Height() - g_config.nTabBorder*2;
	CRect rcCloseButton(rc.right-(nSideLength+g_config.nTabBorder),rc.top+g_config.nTabBorder,rc.right-g_config.nTabBorder,rc.top+(nSideLength+g_config.nTabBorder));
	dcWindow.SelectPen(penTabButton);
	dcWindow.MoveTo(rcCloseButton.left+3, rcCloseButton.top+3);
	dcWindow.LineTo(rcCloseButton.right-3, rcCloseButton.bottom-3);
	dcWindow.MoveTo(rcCloseButton.right-4, rcCloseButton.top+3);
	dcWindow.LineTo(rcCloseButton.left+2, rcCloseButton.bottom-3);
	
	HBRUSH oldBrush = dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
	CPoint pt = GetCurrCurPos();
	if (rcCloseButton.PtInRect(pt))
	{
		dcWindow.SelectPen(penTabButton);
		dcWindow.Draw3dRect(rcCloseButton, RGB(255,255,255), g_config.clrTabButtonShade);
	}
	else
	{
		dcWindow.SelectPen(penTabBackground);
		dcWindow.Rectangle(rcCloseButton);
	}
}

LRESULT CMainFrame::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CPoint pt = GetCurrCurPos();

	CPen penFrame;
	penFrame.CreatePen(PS_SOLID, 1, g_config.clrFrame);
	CPen penTabBackground;
	penTabBackground.CreatePen(PS_SOLID, 1, g_config.clrTabBackground);
	CPen penCaptionButton;
	penCaptionButton.CreatePen(PS_SOLID, 1, g_config.clrCaptionButton);

	CWindowDC dcWindow(this->m_hWnd);
	//buttons on caption
	//close button
	CRect rcCaptionCloseButton = GetCloseButtonRect();
	if (rcCaptionCloseButton.PtInRect(pt))
	{
		dcWindow.SelectPen(penCaptionButton);
		dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
		rcCaptionCloseButton.InflateRect(1,1,1,1);
		dcWindow.Rectangle(rcCaptionCloseButton);
		
	}
	else
	{
		dcWindow.SelectPen(penFrame);
		dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
		rcCaptionCloseButton.InflateRect(1,1,1,1);
		dcWindow.Rectangle(rcCaptionCloseButton);
	}

	//Maximize button
	CRect rcCaptionMaximizeButton = GetMaximizeButtonRect();
	if (rcCaptionMaximizeButton.PtInRect(pt))
	{
		dcWindow.SelectPen(penCaptionButton);
		dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
		rcCaptionMaximizeButton.InflateRect(1,1,1,1);
		dcWindow.Rectangle(rcCaptionMaximizeButton);
	}
	else
	{
		dcWindow.SelectPen(penFrame);
		dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
		rcCaptionMaximizeButton.InflateRect(1,1,1,1);
		dcWindow.Rectangle(rcCaptionMaximizeButton);
	}

	//Minimize button
	CRect rcCaptionMinimizeButton = GetMinimizeButtonRect();
	if (rcCaptionMinimizeButton.PtInRect(pt))
	{
		dcWindow.SelectPen(penCaptionButton);
		dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
		rcCaptionMinimizeButton.InflateRect(1,1,1,1);
		dcWindow.Rectangle(rcCaptionMinimizeButton);
	}
	else
	{
		dcWindow.SelectPen(penFrame);
		dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
		rcCaptionMinimizeButton.InflateRect(1,1,1,1);
		dcWindow.Rectangle(rcCaptionMinimizeButton);
	}

	//Menu button
	CRect rcCaptionMenuButton = GetMenuButtonRect();
	if (rcCaptionMenuButton.PtInRect(pt))
	{
		dcWindow.SelectPen(penCaptionButton);
		dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
		rcCaptionMenuButton.InflateRect(1,1,1,1);
		dcWindow.Rectangle(rcCaptionMenuButton);
	}
	else
	{
		dcWindow.SelectPen(penFrame);
		dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
		rcCaptionMenuButton.InflateRect(1,1,1,1);
		dcWindow.Rectangle(rcCaptionMenuButton);
	}

	//tab close button
	if ((m_viewList.size()>0)&&(pt.y>m_nCaptionHeight)&&(pt.y<GetTabRectByOrderNumber(m_viewList.size()-1).bottom-2))
	{
		CRect rc;
		if (m_bShrinkTabList)
		{
			rc = GetTabRectByOrderNumber(0);
		}
		else
		{
			int nIndex = (pt.y - m_nCaptionHeight+1)/(g_config.nTabBorder*2-PointsToLogical(g_config.fontSize));
			rc = GetTabRectByOrderNumber(nIndex);
		}
		int nSideLength = rc.Height() - g_config.nTabBorder*2;
		CRect rcCloseButton(rc.right-(nSideLength+g_config.nTabBorder),rc.top+g_config.nTabBorder,rc.right-g_config.nTabBorder,rc.top+(nSideLength+g_config.nTabBorder));
		dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
		
		if (rcCloseButton.PtInRect(pt))
		{
			dcWindow.SelectPen(penFrame);
			dcWindow.Draw3dRect(rcCloseButton, RGB(255,255,255), g_config.clrTabButtonShade);
			
		}
		else
		{
			dcWindow.SelectPen(penTabBackground);
			dcWindow.Rectangle(rcCloseButton);
			
		}
	}


	if(m_viewList.size()>1)
	{
		

		CRect rc = GetTabRectByOrderNumber(0);
		CRect rcPanel(rc.left, m_nTopOfView - m_nHeightOfTabPanel, rc.right, m_nTopOfView);
		dcWindow.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
		if (rcPanel.PtInRect(pt))
		{

			CPen penTabButtonShade;
			penTabButtonShade.CreatePen(PS_SOLID, 1, g_config.clrTabButtonShade);
			HPEN oldPen = dcWindow.SelectPen(penTabButtonShade);
			dcWindow.MoveTo(rcPanel.left+1, rcPanel.bottom-1);//(&rcPanel, RGB(255,255,255), g_config.clrTabButtonShade);
			dcWindow.LineTo(rcPanel.right-1, rcPanel.bottom-1);
			dcWindow.SelectPen(oldPen);
		}
		else
		{
			dcWindow.SelectPen((HPEN)GetStockObject(WHITE_PEN));
			dcWindow.Rectangle(rcPanel);
		}
		
		//tab menu button
		if (m_bShrinkTabList)
		{
			int nCurrActiveView = GetActiveView();
			CRect rc = GetTabRectByOrderNumber(0);
			int nSideLength = rc.Height() - g_config.nTabBorder*2;
			CRect rcTabMenuButton(rc.left+g_config.nTabBorder,rc.top+g_config.nTabBorder,rc.left+(nSideLength+g_config.nTabBorder),rc.top+(nSideLength+g_config.nTabBorder));
			
			rcTabMenuButton.InflateRect(1,1,1,1);
			
			if (rcTabMenuButton.PtInRect(pt))
			{
				dcWindow.Draw3dRect(&rcTabMenuButton, RGB(255,255,255), g_config.clrTabButtonShade);
			}
			else
			{
				dcWindow.SelectPen(penTabBackground);
				dcWindow.Rectangle(&rcTabMenuButton);
			}
			
		}
		
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CMainFrame::OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
	CPoint pt;
	GetCursorPos(&pt);
	CRect rcWindow;
	GetWindowRect(rcWindow);

	int xPos = GET_X_LPARAM(lParam); 
    int yPos = GET_Y_LPARAM(lParam);
    
	//CPoint pt (xPos, yPos); //hunter : 此处入参, 不是相对屏幕左上角的绝对参数, 是相对客户窗口区左上角的相对参数. 不能使用	
    if (rcWindow.PtInRect(pt))
    {    
	    pt.x -= rcWindow.left;  //hunter : 转换成相对位置
	    pt.y -= rcWindow.top;   //hunter : 转换成相对位置	
	}
	else
	{
	    bHandled = FALSE;
	    return 0;
	}

	CRect rcHeadTabArea;
	GetClientRect(rcHeadTabArea);
	rcHeadTabArea.top = m_nCaptionHeight;
	rcHeadTabArea.bottom = m_nTopOfView;
	
    CRect rcMostLeftButton = GetMenuButtonRect();
	CRect rcCaption(0, 0, rcMostLeftButton.left, m_nCaptionHeight);	

	if (m_viewList.size()>1)
	{
		if (rcHeadTabArea.PtInRect(pt))
		{
			if (!m_bShrinkTabList)
			{
				m_nTopOfView = GetTabRectByOrderNumber(0).bottom + m_nHeightOfTabPanel;
				m_bShrinkTabList = true;
			}
			else
			{
				m_nTopOfView = GetTabRectByOrderNumber(m_viewList.size()-1).bottom + m_nHeightOfTabPanel;
				m_bShrinkTabList = false;
			}
			UpdateLayout();
			SendMessage(m_hWnd, WM_NCPAINT, 0, 0);
			bHandled = true;
		}
		else
		{
		    bHandled = FALSE;
		}
	}
	else
	{		
	    bHandled = FALSE;
    }
	return 0;
}
LRESULT CMainFrame::OnTaskBarRClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    if(uMsg == WM_NCRBUTTONDOWN)
    {
        CRect rcMostLeftButton = GetMenuButtonRect();
	    CRect rcCaption(0, 0, rcMostLeftButton.left, m_nCaptionHeight);
	    
	    CPoint pt;
	    GetCursorPos(&pt);
	    CRect rcWindow;
	    GetWindowRect(rcWindow);
        if (rcWindow.PtInRect(pt))
        {    
	        pt.x -= rcWindow.left;  //hunter : 转换成相对位置
	        pt.y -= rcWindow.top;   //hunter : 转换成相对位置	
        }	        
	    if(rcCaption.PtInRect(pt))
	    {
	        ;
	    }
	    else
	    {
	        bHandled = FALSE;
	        return 0;
	    }
    }
    ShowCaptionMenu();
	bHandled = TRUE;
	return 0;    
}
LRESULT CMainFrame::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	SendMessage(WM_NCPAINT,0,0);
	bHandled = FALSE;
	CPoint pt;
	GetCursorPos(&pt);
	CRect rcWindow;
	GetWindowRect(rcWindow);
	pt.x -= rcWindow.left;
	pt.y -= rcWindow.top;
	//caption button
	//caption close button.
	CRect rcCaptionCloseButton = GetCloseButtonRect();
	if (rcCaptionCloseButton.PtInRect(pt))
	{
		bHandled = TRUE;
		//if (CloseAll())
			PostMessage(WM_CLOSE);
		return 0;
	}

	CRect rcCaptionMaximizeButton = GetMaximizeButtonRect();
	if (rcCaptionMaximizeButton.PtInRect(pt))
	{
		if(FRAME_STATE_DOCK == m_nFrameState || FRAME_STATE_DOCK_AUTOHIDE == m_nFrameState)
		{
			UnDock();
			m_nFrameState = FRAME_STATE_NORMAL;
			ShowWindow(SW_MAXIMIZE);
		}
		else
		{
			if (this->IsZoomed())
				ShowWindow(SW_RESTORE);
			else
				ShowWindow(SW_MAXIMIZE);
		}
			
		bHandled = TRUE;
		return 0;
	}

	CRect rcCaptionMinimizeButton = GetMinimizeButtonRect();
	if (rcCaptionMinimizeButton.PtInRect(pt))
	{
		if(FRAME_STATE_DOCK == m_nFrameState || FRAME_STATE_DOCK_AUTOHIDE == m_nFrameState)
		{
			WindowShown(false);
			
		}
		else
		{
			ShowWindow(SW_MINIMIZE);
		}
		bHandled = TRUE;
		return 0;
	}

	CRect rcCaptionMenuButton = GetMenuButtonRect();
	if (rcCaptionMenuButton.PtInRect(pt))
	{
        ShowCaptionMenu();
		bHandled = TRUE;
		return 0;
	}

	if(m_viewList.empty())
	{
		
	}
	else
	{
		int nCurrActiveView = GetActiveView();
		//tab close button.
		if (!m_bShrinkTabList)
		{
			int nActiveWindowIndex = GetActiveView();
			for (int i=0;i<m_viewList.size(); ++i)
			{
				CRect rc = GetTabRectByOrderNumber(i);
				int nSideLength = rc.Height() - g_config.nTabBorder*2;
				CRect rcCloseButton(rc.right-(nSideLength+g_config.nTabBorder),rc.top+g_config.nTabBorder,rc.right-g_config.nTabBorder,rc.top+(nSideLength+g_config.nTabBorder));
				//test if on close button
				if (rcCloseButton.PtInRect(pt))
				{
					//ask if save for dirty file
					if (m_viewList[i]->m_bDirty)
					{
						CString msg;
						msg.Format(_T("你想保存 \"%s\" 么？"), m_viewList[i]->GetFileName());
						int re = MessageBox(msg, _T("警告"), MB_YESNOCANCEL|MB_ICONWARNING);
						if (IDCANCEL == re)
						{
							bHandled = TRUE;
							return 0;
						}
						else
						{
							if (IDYES == re)
							{
								m_viewList[i]->Save();
							}
							else if (IDNO == re)
							{
							}
						}
					}
					if (m_viewList.size()>1)
					{
						if (nCurrActiveView == i)
						{
							if (nCurrActiveView >= (m_viewList.size()-1))
							{
								ActivePage(nCurrActiveView-1);
							}
							else
							{
								ActivePage(nCurrActiveView+1);
							}
							WriteBooguNoteIni();
						}
					}
					//hunter : 关闭时删除文档历史列表中的所有该文档的记录
					for (int k=m_ActiveFileChain.size()-1; k>=0; k--)
					{
						if (m_ActiveFileChain[k] == m_viewList[i])
						{
							m_ActiveFileChain.erase(m_ActiveFileChain.begin()+k);
						}
					}
					//hunter : 删除打开文档历史数组时需要更新打开文档历史数组的当前游标
					m_nCaptionHistoryCursor = m_ActiveFileChain.size()-1;
					
					CleanActiveFileChain();

					m_viewList[i]->DestroyWindow();
					delete m_viewList[i];
					m_viewList.erase(m_viewList.begin()+i);

					if (m_viewList.size()==1)
						m_nTopOfView = GetTabRectByOrderNumber(m_viewList.size()-1).bottom;
					else
						m_nTopOfView = GetTabRectByOrderNumber(m_viewList.size()-1).bottom + m_nHeightOfTabPanel;
					UpdateLayout();
					SendMessage(m_hWnd, WM_NCPAINT, 0, 0);
					bHandled = TRUE;
					WriteBooguNoteIni();
					break;
				}
				else if (rc.PtInRect(pt))
				{
					if (nActiveWindowIndex != i)
					{
						ActivePage(i);
						WriteBooguNoteIni();
					}
					break;
				}
			}
		}
		else //hunter : 折叠时, 检查当前激活的文档
		{
			CRect rc = GetTabRectByOrderNumber(0);
			int nSideLength = rc.Height() - g_config.nTabBorder*2;
			CRect rcCloseButton(rc.right-(nSideLength+g_config.nTabBorder),rc.top+g_config.nTabBorder,rc.right-g_config.nTabBorder,rc.top+(nSideLength+g_config.nTabBorder));
			if (rcCloseButton.PtInRect(pt))
			{
                CurrentCaptionClose();
				bHandled = TRUE;
			}
		}
		//shrink
		if (m_viewList.size()>1)
		{
			CRect rc = GetTabRectByOrderNumber(0);
			CRect rcTabPanel(rc.left, m_nTopOfView - m_nHeightOfTabPanel, rc.right, m_nTopOfView);
			if (rcTabPanel.PtInRect(pt))
			{
				if (!m_bShrinkTabList)
				{
					m_nTopOfView = GetTabRectByOrderNumber(0).bottom + m_nHeightOfTabPanel;
					m_bShrinkTabList = true;
				}
				else
				{
					m_nTopOfView = GetTabRectByOrderNumber(m_viewList.size()-1).bottom + m_nHeightOfTabPanel;
					m_bShrinkTabList = false;
				}
				UpdateLayout();
				SendMessage(m_hWnd, WM_NCPAINT, 0, 0);
			}
		}
		//tab menu button.
		if (m_bShrinkTabList)
		{
			int nCurrActiveView = GetActiveView();
			CRect rc = GetTabRectByOrderNumber(0);
			int nSideLength = rc.Height() - g_config.nTabBorder*2;
			CRect rcTabMenuButton(rc.left+g_config.nTabBorder,rc.top+g_config.nTabBorder,rc.left+(nSideLength+g_config.nTabBorder),rc.top+(nSideLength+g_config.nTabBorder));
			if (rcTabMenuButton.PtInRect(pt))
			{
				PopFileList();
				bHandled = TRUE;
			}
			return 0;
		}
	}

	return 0;
}
int CMainFrame::CurrentCaptionClose()
{
	int nCurrActiveView = GetActiveView();
	if (nCurrActiveView<0)
		return -1;
	//ask if save for dirty file
	if (m_viewList[nCurrActiveView]->m_bDirty)
	{
		CString msg;
		msg.Format(_T("你想保存 \"%s\" 么？"), m_viewList[nCurrActiveView]->GetFileName());
		int re = MessageBox(msg, _T("警告"), MB_YESNOCANCEL|MB_ICONWARNING);
		if (IDCANCEL == re)
		{
			return 0;
		}
		else
		{
			if (IDYES == re)
			{
				m_viewList[nCurrActiveView]->Save();
			}
			else if (IDNO == re)
			{
			}
		}
	}
	if (m_viewList.size()>1)
	{
		if (nCurrActiveView >= (m_viewList.size()-1))
		{
			ActivePage(nCurrActiveView-1);
		}
		else
		{
			ActivePage(nCurrActiveView+1);
		}
		WriteBooguNoteIni();
	}
	
    //hunter : 关闭时删除文档历史列表中的所有该文档的记录
    for (int k=m_ActiveFileChain.size()-1; k>=0; k--)
    {
    	if (m_ActiveFileChain[k] == m_viewList[nCurrActiveView])
    	{
    		m_ActiveFileChain.erase(m_ActiveFileChain.begin()+k);
    	}
    }
    //hunter : 删除打开文档历史数组时需要更新打开文档历史数组的当前游标
    m_nCaptionHistoryCursor = m_ActiveFileChain.size()-1;
    
    CleanActiveFileChain();
    	
	m_viewList[nCurrActiveView]->DestroyWindow();
	delete m_viewList[nCurrActiveView];
	m_viewList.erase(m_viewList.begin()+nCurrActiveView);
	if (m_bShrinkTabList)
	{
		if (m_viewList.size()==1)
			m_nTopOfView = GetTabRectByOrderNumber(0).bottom;
		else
			m_nTopOfView = GetTabRectByOrderNumber(0).bottom + m_nHeightOfTabPanel;
	}
	else
	{
		if (m_viewList.size()==1)
			m_nTopOfView = GetTabRectByOrderNumber(m_viewList.size()-1).bottom;
		else
			m_nTopOfView = GetTabRectByOrderNumber(m_viewList.size()-1).bottom + m_nHeightOfTabPanel;
	}
	UpdateLayout();
	SendMessage(m_hWnd, WM_NCPAINT, 0, 0);
	WriteBooguNoteIni();    
}
LRESULT CMainFrame::OnCaptionClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CurrentCaptionClose();
	return 0;
}


LRESULT CMainFrame::OnPreference(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CPreferenceDlg prefDlg ( _T("选项"), 0 );



	ACCEL accelView[50];
	memcpy(accelView, g_config.AccelView, 50*sizeof(ACCEL));

	int ret = prefDlg.DoModal ( GetTopLevelParent());

    if ( IDOK == ret )
	{
		if (prefDlg.m_pgShortcut.m_bDirty)
		{
			for(int i=0; i<g_config.nAccelGlobalCount; ++i)
			{
				UnregisterHotKey(this->m_hWnd, g_config.AccelGlobal[i].cmd);
				RegisterHotKey(this->m_hWnd, g_config.AccelGlobal[i].cmd, g_config.AccelGlobal[i].fVirt, g_config.AccelGlobal[i].key);
			}
			LoadAccel();
			for (int i=0; i<m_viewList.size(); ++i)
			{
				m_viewList[i]->LoadAccel();
			}
		}
		if (prefDlg.m_pgUISetting.m_bDirty)
		{
			PostMessage(WM_USER_MAINFRM_REFRESH);
		}
		if (prefDlg.m_pgMainFrame.nSaveAllElapse*1000!=g_config.nSaveAllElapse)
		{
			g_config.nSaveAllElapse = prefDlg.m_pgMainFrame.nSaveAllElapse*1000;
			//hunter : 值与原来值不一致时, 杀死原来定时器. 自动保存时间设为0时, 取消自动保存.
			//  KillTimer 返回值 参见  https://msdn.microsoft.com/en-us/library/windows/desktop/ms644903(v=vs.85).aspx
			//  BOOL WINAPI KillTimer( _In_opt_  HWND hWnd, _In_ UINT_PTR uIDEvent);    
			//  Return value  Type: Type: BOOL  
            //  If the function succeeds, the return value is nonzero.
            //  If the function fails, the return value is zero. To get extended error information, call GetLastError
			BOOL bKillTimerResult = KillTimer(m_nTimerID);			
			if(0 == g_config.nSaveAllElapse)
			{
			    ;
			}
			else
			{
			    SetTimer(m_nTimerID, g_config.nSaveAllElapse, NULL);
			}
		}
		g_config.WriteConfigToIni();
	}
	else
	{
		if (prefDlg.m_pgShortcut.m_bDirty)
		{
			if(NULL!=m_haccelerator)
			{
				CopyAcceleratorTable(m_haccelerator, g_config.AccelMainFrm, g_config.nAccelMainFrmCount);
			}
			memcpy(g_config.AccelView, accelView, 50*sizeof(ACCEL));
		}
	}
	return 0;
}

LRESULT CMainFrame::OnFileList(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ActivePage(((unsigned int)wID) - ID_FILELIST_BASE);
	WriteBooguNoteIni();
	return 0;
}

void CMainFrame::PopFileList()
{
	CRect rc = GetTabRectByOrderNumber(0);
	int nSideLength = rc.Height() - g_config.nTabBorder*2;
	CRect rcTabMenuButton(rc.left+g_config.nTabBorder,rc.top+g_config.nTabBorder,rc.left+(nSideLength+g_config.nTabBorder),rc.top+(nSideLength+g_config.nTabBorder));
	CMenu menuFileList;
	menuFileList.CreatePopupMenu();
	int nScreenHeight = ::GetSystemMetrics(SM_CYSCREEN);
	int nHeightOfMenu = ::GetSystemMetrics(SM_CYMENU);
	int nBreakItem = nScreenHeight/nHeightOfMenu;
	for (int i=0; i<m_viewList.size(); ++i)
	{
		CString str = m_viewList[i]->GetFileName();
		if (str == _T(""))
			str = _T("(未命名)");
		if (m_viewList[i]->m_bDirty)
			str += _T("*");
		if (i%nBreakItem == 0 && i !=0)
			menuFileList.AppendMenu(MF_STRING|MFT_MENUBARBREAK, ID_FILELIST_BASE+i, str);
		else
			menuFileList.AppendMenu(MF_STRING, ID_FILELIST_BASE+i, str);
	}
	menuFileList.CheckMenuItem(GetActiveView(), MF_BYPOSITION|MF_CHECKED);
	menuFileList.SetMenuDefaultItem(GetActiveView(), TRUE);

	// Display the shortcut menu
	CPoint ptMenu(rcTabMenuButton.left, rcTabMenuButton.bottom);
	CRect rcWindow;
	GetWindowRect(&rcWindow);
 	ptMenu.x += rcWindow.left;
 	ptMenu.y += rcWindow.top;
	ptMenu.x -= 1;
	ptMenu.y += 3;
	if (!menuFileList.TrackPopupMenu(TPM_LEFTALIGN|TPM_TOPALIGN|TPM_LEFTBUTTON,
		(int)ptMenu.x, (int)ptMenu.y, m_hWnd))
	{
		ATLTRACE(_T("Context menu was not displayed successfully!\n"));
	}

	// Destroy the menu and free any memory that the menu occupies
	menuFileList.DestroyMenu();
	menuFileList.m_hMenu = NULL;
}

void CMainFrame::ActivePage(int index)
{
	if ((index>=0) 
	    &&(index<m_viewList.size())
	    &&(0 < m_viewList.size())
	   )
	{        
        
        int nActiveWindowIndex = GetActiveView();
        
        //hunter : 文档已经激活了, 不重复激活文档
        if(index == nActiveWindowIndex)
        {
            return ;
        }
        
        if (nActiveWindowIndex != -1)
        {				
        	m_viewList[nActiveWindowIndex]->ShowWindow(SW_HIDE);
        	m_viewList[nActiveWindowIndex]->m_bActive = false;				
        }
        		
		if(m_ActiveFileChain.size() > 10000)
		{    
			m_ActiveFileChain.erase(m_ActiveFileChain.begin());
		    m_ActiveFileChain.push_back(m_viewList[index]);
		}
		else
		{
		    m_ActiveFileChain.push_back(m_viewList[index]);
		}
		m_viewList[index]->m_bActive = true;
		m_viewList[index]->ShowWindow(SW_SHOW);
		m_viewList[index]->SetActiveWindow();
		m_viewList[index]->SetFocus();
		//hunter : 增加打开文档历史数组时需要更新打开文档历史数组的当前游标
		m_nCaptionHistoryCursor = m_ActiveFileChain.size()-1;
		
		m_pCurrView = m_viewList[index];
		
		SendMessage(m_hWnd, WM_NCPAINT, 0, 0);
	}
	
}
int CMainFrame::CleanActiveFileChain()
{
    if(m_ActiveFileChain.size()>1)
    {
        //hunter : 删除文档历史列表中的所有连续文档的重复记录
        for (int k=0;    (m_ActiveFileChain.size() >1)&& (k<(m_ActiveFileChain.size()-1)); )
        {
        	if (m_ActiveFileChain[k] == m_ActiveFileChain[k+1])
        	{
        		m_ActiveFileChain.erase(m_ActiveFileChain.begin()+k+1);
        	}
        	else
        	{
        	    k++;
        	}
        }
        //hunter : 更新打开文档历史数组时需要更新打开文档历史数组的当前游标
        m_nCaptionHistoryCursor = m_ActiveFileChain.size()-1;
    }
	return 0;
}
LRESULT CMainFrame::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CRect rcWindow;
	GetWindowRect(rcWindow);

	int xPos = GET_X_LPARAM(lParam); 
    int yPos = GET_Y_LPARAM(lParam);
    CPoint pt (xPos, yPos); //hunter : 修改成参数传入的值, 而不是使用临时获取的鼠标坐标值
	pt.x -= rcWindow.left;  //hunter : 转换成相对位置
	pt.y -= rcWindow.top;   //hunter : 转换成相对位置
	
	//hunter : 另一种处理方式, 是把默认的值   HTCAPTION  修改成 HTCLIENT 
	//LRESULT	lres = 0;//hunter : 
	//lres = ::DefWindowProc(m_hWnd, uMsg,wParam,lParam);//hunter :	
	//CRect rcOldTitleArea(0, m_nCaptionHeight, rcWindow.Width(), m_nCaptionHeight+10);
	//if(HTCAPTION == lres)
	//{        		
	//	if (rcOldTitleArea.PtInRect(pt))
	//	{
	//		bHandled = TRUE;
	//		return HTCLIENT;
	//	}	    
	//}	

	CRect rcHeadTabArea;
	GetClientRect(rcHeadTabArea);
	rcHeadTabArea.top       = m_nCaptionHeight;
	rcHeadTabArea.bottom    = m_nTopOfView;
	
	if(!(FRAME_STATE_DOCK == m_nFrameState || FRAME_STATE_DOCK_AUTOHIDE == m_nFrameState))
	{
		//hunter : 
	    if (rcHeadTabArea.PtInRect(pt))
	    {
	    	bHandled = TRUE;
	    	return HTCLIENT;
	    }
	    
		CRect rcMostLeftButton = GetMenuButtonRect();
		CRect rcCaption(0, 0, rcMostLeftButton.left, m_nCaptionHeight);
		if (rcCaption.PtInRect(pt))
		{
			bHandled = TRUE;
			return HTCAPTION;
		}	    

		CRect rcCaptionButtonArea(rcMostLeftButton.left, 0, GetCloseButtonRect().right, m_nCaptionHeight);		
		if (rcCaptionButtonArea.PtInRect(pt))
		{
			bHandled = TRUE;
			return HTCLIENT;
		}
		else
		{
			bHandled = FALSE;
			return 0;
		}
	}
	else
	{
		CRect rcMostLeftButton = GetMenuButtonRect();
		CRect rcCaptionButtonArea(rcMostLeftButton.left, 0, GetCloseButtonRect().right, m_nCaptionHeight);
		if (rcCaptionButtonArea.PtInRect(pt))
		{
			bHandled = TRUE;
			return HTCLIENT;
		}
		else
		{
			bHandled = FALSE;
			return 0;
		}
	}
	
}
LRESULT CMainFrame::OnNcActive(UINT uMsg, WPARAM &wParam, LPARAM &lParam, BOOL& bHandled)
{
	SendMessage(m_hWnd, WM_NCPAINT, 0, 0);
	bHandled = FALSE;
	return 0;
}


LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

CBooguNoteView* CMainFrame::CreateNewView()
{
	CBooguNoteView* pView = new CBooguNoteView();
	pView->m_fileDirectory = _T("");
	pView->Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE /*| WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL*//* , WS_EX_CLIENTEDGE*/);

	pView->m_bActive = false;
	m_viewList.push_back(pView);
	sort(m_viewList.begin(), m_viewList.end(), CMainFrame::NameCompare);
	int i=0;
	for (; i<m_viewList.size(); ++i)
	{
		if (m_viewList[i] == pView)
		{
			
			break;
		}
	}
	m_nTopOfView = GetTabRectByOrderNumber(0).bottom;
	//m_nShownView = 0;
	if (!m_bShrinkTabList)
	{
		m_nTopOfView = GetTabRectByOrderNumber(m_viewList.size()-1).bottom;
	}
	else
	{
		m_nTopOfView = GetTabRectByOrderNumber(0).bottom;
	}
	if (m_viewList.size()>1)
		m_nTopOfView += m_nHeightOfTabPanel;
	UpdateLayout();
	ActivePage(i);
	WriteBooguNoteIni();
	return pView;
}

LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CreateNewView();
	return 0;
}
LRESULT CMainFrame::OnFileRename(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	sort(m_viewList.begin(), m_viewList.end(), CMainFrame::NameCompare);
	SendMessage(WM_NCPAINT);
	return 0;
}

LRESULT CMainFrame::OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialogFilter strFilter(_T("BooguNote File (*.boo)||FreeMind File (*.mm)||"));
	CString fileName;
	CMultiFileDialog fd(_T("*.boo"), _T(""), OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, strFilter);
	fd.m_ofn.lpstrInitialDir = g_config.szRootStorageDir;
	
	if (IDCANCEL == fd.DoModal())
	{
		return 1;
	}
	TCHAR szPath[_MAX_PATH];
	fd.GetFirstPathName(szPath,_MAX_PATH);
	OpenFileByName(szPath);
	while (0!=fd.GetNextPathName(szPath,_MAX_PATH))
	{
		OpenFileByName(szPath);
	}
	WriteBooguNoteIni();
	return 0;
}

LRESULT CMainFrame::OnMainFrmOpenFile(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	OpenFileByName((TCHAR*)wParam);
	WriteBooguNoteIni();
	return 0;
}

int CMainFrame::OpenFileByName(TCHAR* szFileName)
{
	CString fn(szFileName);
	int dot = fn.ReverseFind(_T('.'));
	CString ext = fn;
	ext.Delete(0, dot+1);
	ext.MakeLower();
	if ((0!=ext.Compare(_T("boo")))&&(0!=ext.Compare(_T("mm"))))
	{
		return -1;
	}
	
	int nOpenIndex = -1;
	HANDLE hFile = CreateFileW(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile || ERROR_FILE_NOT_FOUND == GetLastError())
	{
		CloseHandle(hFile);
		return -1;
	}
	CloseHandle(hFile);
	for (int i=0; i<m_viewList.size(); ++i)
	{
		if (m_viewList[i]->m_fileDirectory == szFileName)
		{
			nOpenIndex = i;
			break;
		}
	}
	if (nOpenIndex == -1)
	{
		CBooguNoteView* pView = new CBooguNoteView();
		pView->m_fileDirectory = szFileName;
		HWND hWnd = pView->Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE);
		if (NULL == hWnd)
			return -1;
		pView->m_bActive = false;;
		m_viewList.push_back(pView);
		sort(m_viewList.begin(), m_viewList.end(), CMainFrame::NameCompare);
		int i=0;
		for (; i<m_viewList.size(); ++i)
		{
			if (m_viewList[i] == pView)
			{
				//m_nShownView = i;
				break;
			}
		}
		
		if (!m_bShrinkTabList)
		{
			m_nTopOfView = GetTabRectByOrderNumber(m_viewList.size()-1).bottom;
		}
		else
		{
			m_nTopOfView = GetTabRectByOrderNumber(0).bottom;
		}
		if (m_viewList.size()>1)
			m_nTopOfView += m_nHeightOfTabPanel;
		UpdateLayout();
		ActivePage(i);
		nOpenIndex = i;
	}
	else
	{
		ActivePage(nOpenIndex);
	}
	return nOpenIndex;
}

LRESULT CMainFrame::OnDock(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (!IsWindowVisible())
	{
		ShowWindow(SW_SHOW);
		if (FRAME_STATE_DOCK == m_nFrameState)
		{
			Dock();
		}
	}

	if (FRAME_STATE_NORMAL == m_nFrameState)
	{
		Dock();
	}
	else if (FRAME_STATE_DOCK_AUTOHIDE == m_nFrameState)
	{
		
		Dock();
	}
	m_nFrameState = FRAME_STATE_DOCK;

	return 0;
}

LRESULT CMainFrame::OnAutoHide(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	GetWindowRect(&m_rcOriginal);
	if (!IsWindowVisible())
	{
		ShowWindow(SW_SHOW);
	}
	if (FRAME_STATE_NORMAL == m_nFrameState)
	{
		//Dock();
		SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |  SWP_FRAMECHANGED);
		ShowWindow(SW_HIDE);
		
		ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW|WS_EX_TOPMOST);
		ShowWindow(SW_SHOW);
		SetTrayIcon(true);
		SetKeepSize(true);
		SetAutoHide(true);
		UINT uFlag = -1;
		if (DOCK_RIGHT == g_config.DockSide)
		{
			uFlag = APPBAR_DOCKING_RIGHT;
		}
		else if (DOCK_TOP == g_config.DockSide)
		{
			uFlag = APPBAR_DOCKING_TOP;
		}
		else if (DOCK_LEFT == g_config.DockSide)
		{
			uFlag = APPBAR_DOCKING_LEFT;
		}
		else if (DOCK_BOTTOM == g_config.DockSide)
		{
			uFlag = APPBAR_DOCKING_BOTTOM;
		}
		else
		{
			uFlag = APPBAR_DOCKING_TOP;
		}
		DockAppBar(uFlag);
	}
	else if (FRAME_STATE_DOCK == m_nFrameState)
	{
		SetAutoHide(true);
	}
	m_nFrameState = FRAME_STATE_DOCK_AUTOHIDE;

	return 0;
}


LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

bool CMainFrame::NameCompare(CBooguNoteView* e1, CBooguNoteView* e2)
{
	return e1->GetFileName() < e2->GetFileName();
}

bool CMainFrame::Compare(basic_string<TCHAR> e1, basic_string<TCHAR> e2)
{
	return e1 > e2;
}
LRESULT CMainFrame::OnCaptionNormalWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (!IsWindowVisible())
	{
		ShowWindow(SW_SHOW);
		SetForegroundWindow(this->m_hWnd);
		SendMessage(WM_NCPAINT);
	}

	if (FRAME_STATE_DOCK == m_nFrameState)
	{
		UnDock();
	}
	else if (FRAME_STATE_DOCK_AUTOHIDE == m_nFrameState)
	{
		UnDock();
		SetAutoHide(false);
	}

	m_nFrameState = FRAME_STATE_NORMAL;

	return 0;
}

LRESULT CMainFrame::OnCaptionHiden(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (IsWindowVisible())
	{
		WindowShown(false);
	}
	else
	{
		WindowShown(true);
	}

	return 0;
}

LRESULT CMainFrame::ShowNotification(TCHAR* szNofity)
{
	if (m_pwndNotify && m_pwndNotify->IsWindow())
	{
		m_pwndNotify->KillTimer(NOTIFY_TIMER_ID );
		m_pwndNotify->DestroyWindow();
		delete m_pwndNotify;
	}
	m_pwndNotify=new CNotifyWnd(m_hWnd,235,60);
	m_pwndNotify->m_strNotify = szNofity;
	m_pwndNotify->CreateNotifyWindow();
	return 0;
}

LRESULT CMainFrame::OnCaptionmenuTopMost(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (FRAME_STATE_NORMAL == m_nFrameState)
	{
		if (!g_config.bTopmost)
		{
			g_config.bTopmost = true;
			SetWindowPos(HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		}
		else
		{
			g_config.bTopmost = false;
			SetWindowPos(HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		}
	}

	return 0;
}

LRESULT CMainFrame::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SendMessage(WM_NCPAINT,0,0);

	if (WA_INACTIVE != wParam)
	{
		int nCurrActiveView = GetActiveView();
		if (nCurrActiveView>=0)
		{
			if (NULL != m_viewList[nCurrActiveView]->m_pTextBlockFocused)
			{
				BOOL t;
				DWORD tt = 0;
				m_viewList[nCurrActiveView]->OnLBottonDown(0,0,(LPARAM&)tt, t);
			}
		}
	}
	else
	{
		bHandled = FALSE;
	}
	return 0;
}


LRESULT CMainFrame::OnCaptionmenuFilesTreeview(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (!m_FileTV.IsWindow())
	{
		CRect rect;
		rect.left = g_config.nFileManLeft;
		rect.top = g_config.nFileManTop;
		if (rect.left<=0)
		{
			rect.left = 100;
		}
		if (rect.top <= 0)
		{
			rect.top =  100;
		}
		rect.right = rect.left + g_config.nFileManWidth;
		rect.bottom = rect.top + g_config.nFileManHeight;
		m_FileTV.Create(m_hWnd, rect, NULL, WS_CAPTION | WS_THICKFRAME | WS_CLIPSIBLINGS | WS_POPUP | WS_VISIBLE | WS_SYSMENU, WS_EX_TOOLWINDOW);
	}
	else
	{
		m_FileTV.PostMessage(WM_USER_CHANG_DIR, 0, 0);
	}

	return 0;
}

LRESULT CMainFrame::OnUserRefreshFileTreeView(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (m_FileTV.IsWindow())
	{
		m_FileTV.PostMessage(WM_USER_CHANG_DIR, 0, 0);
	}
	return 0;
}

LRESULT CMainFrame::OnUserSlideOut(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (m_FileTV.IsWindow())
	{
		m_FileTV.ShowWindow(SW_SHOW);
	}
	bHandled = TRUE;
	return 0;
}

LRESULT CMainFrame::OnUserSlideIn(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (m_FileTV.IsWindow())
	{
		m_FileTV.ShowWindow(SW_HIDE);
	}
	bHandled = TRUE;
	return 0;
}
LRESULT CMainFrame::OnUserShare(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (!m_wndShare.IsWindow())
	{
		m_wndShare.Create(NULL, CRect(100,100,500,500), L"BooguOnline", WS_CAPTION | WS_THICKFRAME | WS_CLIPSIBLINGS | WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX );
		m_wndShare.ShowWindow(SW_SHOW);
	}
	else
	{
		SetForegroundWindow(m_wndShare.m_hWnd);
	}
	
	return 0;
}
