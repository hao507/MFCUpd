// UpdateDLg.cpp : 实现文件
//

#include "stdafx.h"
#include "UdsUpdateTool.h"
#include "UpdateDLg.h"
#include "afxdialogex.h"


// CUpdateDLg 对话框

IMPLEMENT_DYNAMIC(CUpdateDLg, CDialogEx)

CUpdateDLg::CUpdateDLg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_UPDATE, pParent)
{

}

CUpdateDLg::~CUpdateDLg()
{
}

void CUpdateDLg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FILE_PATH, m_EditPath);
	DDX_Check  (pDX, IDC_CHECK_RESET_UPDATE, m_ResetUpdate);
}


BEGIN_MESSAGE_MAP(CUpdateDLg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_FILE_PATH, &CUpdateDLg::OnBnClickedButtonFilePath)
	ON_BN_CLICKED(IDC_CHECK_RESET_UPDATE, &CUpdateDLg::OnBnClickedCheckResetUpdate)
END_MESSAGE_MAP()


BOOL CUpdateDLg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_CstrPath = _T("E:\\");
	//GetDlgItem(IDC_EDIT_FILE_PATH)->SetWindowText(m_CstrPath);
	m_EditPath.SetWindowTextW(m_CstrPath);
	//UpdateData(false);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

// CUpdateDLg 消息处理程序



void CUpdateDLg::OnBnClickedButtonFilePath()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL isOpen = TRUE;     //是否打开(否则为保存)  
	CString defaultDir = _T("E:\\FileTest");   //默认打开的文件路径  
	CString fileName = _T("");         //默认打开的文件名  
	CString filter = _T("文件 (*.hex; *.bin)|*.hex;*.bin||");   //文件过虑的类型  
	CFileDialog openFileDlg(isOpen, defaultDir, fileName, OFN_HIDEREADONLY | OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = _T("E:\\FileTest\\test.doc");
	INT_PTR result = openFileDlg.DoModal();
	m_CstrPath = (defaultDir + _T("\\test.doc"));
	if (result == IDOK) {
		m_CstrPath = openFileDlg.GetPathName();
	}
	m_EditPath.SetWindowTextW(m_CstrPath);
}

void CUpdateDLg::OnBnClickedCheckResetUpdate()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);//更新数据

}


void CUpdateDLg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	CDialogEx::OnOK();
}
