#pragma once
#include "ColoredListCtrl.h"
#include "ControlCAN.h"

// CMainPageDlg 对话框

class CMainPageDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMainPageDlg)

public:
	CMainPageDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMainPageDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_MAIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	CColoredListCtrl m_RecvList;

	CString m_Cstrprint;
	CEdit   m_Editprint;

	CProgressCtrl m_ProgUpdate;

	CCriticalSection m_PrintMutex;
	INT nextrow = 0;
public:
	virtual BOOL OnInitDialog();
	void InsertItem(INT Dire, VCI_CAN_OBJ * pCanObj);
	void PrintLog(UINT LogColor, CString LogContent);
	afx_msg void OnNMRClickListCanmsg(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuClrlist();
};
