
// UdsUpdateToolDlg.h : 头文件
//

#pragma once
#include "ColoredListCtrl.h"
#include "MainPageDlg.h"
#include "DealFileDlg.h"
#include "afxwin.h"
#include "ControlCAN.h"
#include "UdsClient.h"
#include "uds_program.h"

// CUdsUpdateToolDlg 对话框
class CUdsUpdateToolDlg : public CDialogEx
{
// 构造
public:
	CUdsUpdateToolDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UDSUPDATETOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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

	CDialog* pDialog[5];  //用来保存对话框对象指针

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
