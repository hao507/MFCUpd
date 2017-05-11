#pragma once
#include "UdsClient.h"
#include "MainPageDlg.h"
// CStartUpdateDLg �Ի���

class CUpdateDLg : public CDialogEx
{
	DECLARE_DYNAMIC(CUpdateDLg)

public:
	CUpdateDLg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CUpdateDLg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_UPDATE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
private:
	CEdit   m_EditPath;

public:
	CUdsClient *pClient;
	CMainPageDlg *pMainPage;

public:
	CString m_CstrPath;
	BOOL    m_ResetUpdate;
	afx_msg void OnBnClickedButtonFilePath();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCheckResetUpdate();
	virtual void OnOK();
};
