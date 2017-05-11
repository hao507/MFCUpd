// DealFileDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "UdsUpdateTool.h"
#include "DealFileDlg.h"
#include "afxdialogex.h"


// CDealFileDlg 对话框

IMPLEMENT_DYNAMIC(CDealFileDlg, CDialogEx)

CDealFileDlg::CDealFileDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_DEALFILE, pParent)
{

}

CDealFileDlg::~CDealFileDlg()
{
}

void CDealFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDealFileDlg, CDialogEx)
END_MESSAGE_MAP()


// CDealFileDlg 消息处理程序
