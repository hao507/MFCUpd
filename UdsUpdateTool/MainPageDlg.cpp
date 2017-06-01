// MainPageDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "UdsUpdateTool.h"
#include "MainPageDlg.h"
#include "UpdateDLg.h"
#include "ControlCAN.h"
#include "afxdialogex.h"

#define DISP_TIME
// CMainPageDlg 对话框

IMPLEMENT_DYNAMIC(CMainPageDlg, CDialogEx)

CMainPageDlg::CMainPageDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_MAIN, pParent)
{

}

CMainPageDlg::~CMainPageDlg()
{
}

void CMainPageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PRINT, m_Editprint);
	DDX_Control(pDX, IDC_PROGRESS_UPDATE, m_ProgUpdate);

	DDX_Control(pDX, IDC_LIST_CANMSG, m_RecvList);

}


BEGIN_MESSAGE_MAP(CMainPageDlg, CDialogEx)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_CANMSG, &CMainPageDlg::OnNMRClickListCanmsg)
	ON_COMMAND(ID_MENU_CLRLIST, &CMainPageDlg::OnMenuClrlist)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CMainPageDlg::OnBnClickedButtonClear)
END_MESSAGE_MAP()


// CMainPageDlg 消息处理程序


BOOL CMainPageDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	//Init Information Window
	PrintLog(0, _T("欢迎使用UdsUpdateTool(CAN)"));
	PrintLog(0, _T("Copyright* 深圳车联天下信息科技有限公司"));
	PrintLog(0, _T(">>"));

	//Init Update Progress

	m_ProgUpdate.SetRange(0, 100);
	m_ProgUpdate.SetStep(1);
	m_ProgUpdate.StepIt();


	//Recieve Lst
	m_RecvList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	m_RecvList.InsertColumn(0, _T("Seq"));
	m_RecvList.SetColumnWidth(0, 40);
	m_RecvList.InsertColumn(1, _T("TxRx"));
	m_RecvList.SetColumnWidth(1, 40);
	m_RecvList.InsertColumn(2, _T("MSG ID"));
	m_RecvList.SetColumnWidth(2, 55);
	m_RecvList.InsertColumn(3, _T("DLC"));
	m_RecvList.SetColumnWidth(3, 40);
	m_RecvList.InsertColumn(4, _T("Data"));
	m_RecvList.SetColumnWidth(4, 150);

#ifdef DISP_TIME
	m_RecvList.InsertColumn(5, _T("TimeStamp"));
	m_RecvList.SetColumnWidth(5, 50);
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}



// Insert a Can msg to list
void CMainPageDlg::InsertItem(INT Dire, VCI_CAN_OBJ * pCanObj)
{

	UINT JustnowItem;

	//if ((theApp.m_FilterEn == TRUE) && (pCanObj->ID < theApp.m_Bgnid || pCanObj->ID > theApp.m_Endid)) return;

	//发送信息列表显示
	CString str;
	CString str1;

	if (nextrow >= 10000)
	{
		m_RecvList.DeleteAllItems();
		nextrow = 0;
	}

	str.Format(_T("%d"), nextrow);

	if (Dire == 1)
		m_RecvList.ItemColorFlag[nextrow] = 1;
	else
		m_RecvList.ItemColorFlag[nextrow] = 0;

	JustnowItem = m_RecvList.InsertItem(nextrow, str);
	nextrow++;

	if (Dire == 1)
		m_RecvList.SetItemText(JustnowItem, 1, _T("Send"));
	else
		m_RecvList.SetItemText(JustnowItem, 1, _T("Recv"));


	if ((pCanObj->ExternFlag) == 1)
	{
		str1.Format(_T("%08X"), pCanObj->ID);
	}
	else
	{
		str1.Format(_T("%04X"), pCanObj->ID);
	}

	m_RecvList.SetItemText(JustnowItem, 2, str1);	//ID信息

	str = _T("");
	str.Format(_T("%d"), pCanObj->DataLen);
	m_RecvList.SetItemText(JustnowItem, 3, str);

	INT i;
	str = _T("");
	for (i = 0; i<pCanObj->DataLen; i++)
	{
		str1.Format(_T("%02X"), pCanObj->Data[i]);
		str = (str + str1 + _T(" "));
	}
	m_RecvList.SetItemText(JustnowItem, 4, str);

#ifdef DISP_TIME
	CString strTime;
	SYSTEMTIME   systime;
	GetLocalTime(&systime);
	strTime.Format(_T("%02d:%02d:%02d:%03d"), systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
	m_RecvList.SetItemText(JustnowItem, 5, strTime);
#endif

	CSize size;
	size.cx = 0;
	size.cy = 50;
	m_RecvList.Scroll(size);
	//发送信息列表显示完毕
}

void CMainPageDlg::PrintLog(UINT LogColor, CString LogContent)
{
	m_PrintMutex.Lock();
	m_Cstrprint += LogContent;
	m_Cstrprint += _T("\r\n");

	INT Index = m_Editprint.GetLineCount();
	m_Editprint.LineScroll(Index, 0);

	if (Index > 5000)
	{
		m_Cstrprint.Empty();
	}
	m_PrintMutex.Unlock();
	//GetDlgItem(IDC_EDIT_PRINT)->SetWindowText(m_Cstrprint);
	m_Editprint.SetWindowTextW(m_Cstrprint);
}


void CMainPageDlg::OnNMRClickListCanmsg(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CRect rect;
	GetWindowRect(&rect);
	CPoint p;
	GetCursorPos(&p);//鼠标点的屏幕坐标  
	if (rect.PtInRect(p))
	{
		CMenu RMenu;
		RMenu.LoadMenu(IDR_MENU_RECV);//编辑好的菜单资源  
		CMenu *m_SubMenu = RMenu.GetSubMenu(0);

		m_SubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, this);
	}
	*pResult = 0;
}


void CMainPageDlg::OnMenuClrlist()
{
	// TODO: 在此添加命令处理程序代码
	m_RecvList.DeleteAllItems();
	nextrow = 0;
	UpdateData(FALSE);//更新数据
}


void CMainPageDlg::OnBnClickedButtonClear()
{
	// TODO: 在此添加控件通知处理程序代码
	m_RecvList.DeleteAllItems();
	nextrow = 0;
	UpdateData(FALSE);//更新数据
}
