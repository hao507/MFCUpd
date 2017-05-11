
// UdsUpdateToolDlg.h : ͷ�ļ�
//

#pragma once
#include "ColoredListCtrl.h"
#include "MainPageDlg.h"
#include "DealFileDlg.h"
#include "afxwin.h"
#include "ControlCAN.h"
#include "UdsClient.h"
#include "uds_program.h"

// CUdsUpdateToolDlg �Ի���
class CUdsUpdateToolDlg : public CDialogEx
{
// ����
public:
	CUdsUpdateToolDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UDSUPDATETOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CTabCtrl m_Table;
	CMenu m_Menu;
	CColoredListCtrl *m_List;

	int m_CurSelTab;
	CMainPageDlg m_MainPage;
	CDealFileDlg m_DealFile;

	CDialog* pDialog[5];  //��������Ի������ָ��

	CUdsClient UdsClient;

public:
	afx_msg void OnTcnSelchangeTabMain(NMHDR *pNMHDR, LRESULT *pResult);
	static INT  TransmitCanmsg(VCI_CAN_OBJ *SendObj);
	static UINT UdsMainThread(void *param);
	static UINT ReceiveThread(void *param);

	afx_msg void OnMenuOpendev();
	afx_msg void OnMenuStartUpdate();
	afx_msg void OnMenuStopUpdate();
};
