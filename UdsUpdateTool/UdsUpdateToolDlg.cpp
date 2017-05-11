
// UdsUpdateToolDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "UdsUpdateTool.h"
#include "UdsUpdateToolDlg.h"
#include "UpdateDLg.h"
#include "afxdialogex.h"
#include "UdsUtil.h"
#include "ControlCAN.h"
#include "OpenDevDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//Global Variable
HANDLE hUpdateEvent = NULL;


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CUdsUpdateToolDlg 对话框



CUdsUpdateToolDlg::CUdsUpdateToolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_UDSUPDATETOOL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void CUdsUpdateToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TAB_MAIN, m_Table);
}

BEGIN_MESSAGE_MAP(CUdsUpdateToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MAIN, &CUdsUpdateToolDlg::OnTcnSelchangeTabMain)
	ON_COMMAND(ID_MENU_OPENDEV, &CUdsUpdateToolDlg::OnMenuOpendev)
	ON_COMMAND(ID_MENU_START_UPDATE, &CUdsUpdateToolDlg::OnMenuStartUpdate)
	ON_COMMAND(ID_MENU_STOP_UPDATE,  &CUdsUpdateToolDlg::OnMenuStopUpdate)
	ON_COMMAND(ID_MENU_CLOSDEV, &CUdsUpdateToolDlg::OnMenuClosdev)
	ON_COMMAND(ID_MENU_ECU_RESET, &CUdsUpdateToolDlg::OnMenuEcuReset)
	ON_COMMAND(ID_MENU_SESSION_STD, &CUdsUpdateToolDlg::OnMenuSessionStd)
	ON_COMMAND(ID_MENU_SESSION_EOL, &CUdsUpdateToolDlg::OnMenuSessionEol)
END_MESSAGE_MAP()


// CUdsUpdateToolDlg 消息处理程序

BOOL CUdsUpdateToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	//添加Menu
	m_Menu.LoadMenu(IDR_MENU_MAIN);
	SetMenu(&m_Menu);

	//为Table control 增加页面
	m_Table.InsertItem(0, _T("升级"));
	m_Table.InsertItem(1, _T("文件"));

	//创建对话框
	m_MainPage.Create(IDD_DIALOG_MAIN, &m_Table);
	m_DealFile.Create(IDD_DIALOG_DEALFILE, &m_Table);

	//设定在Table内显示的范围
	CRect rc;
	m_Table.GetClientRect(rc);
	rc.top += 23;
	rc.bottom -= 0;
	rc.left += 0;
	rc.right -= 0;
	m_MainPage.MoveWindow(&rc);
	m_DealFile.MoveWindow(&rc);


	//把对话框对象指针保存起来
	pDialog[0] = &m_MainPage;
	pDialog[1] = &m_DealFile;
	//显示初始页面
	pDialog[0]->ShowWindow(SW_SHOW);
	pDialog[1]->ShowWindow(SW_HIDE);

	//保存当前选择
	m_CurSelTab = 0;

	//开启接收线程
	AfxBeginThread(ReceiveThread, 0);
	//开启UDS线程
	AfxBeginThread(UdsMainThread, &UdsClient);

	//Create Event
	hUpdateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CUdsUpdateToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CUdsUpdateToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CUdsUpdateToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CUdsUpdateToolDlg::OnTcnSelchangeTabMain(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	//把当前的页面隐藏起来
	pDialog[m_CurSelTab]->ShowWindow(SW_HIDE);
	//得到新的页面索引
	m_CurSelTab = m_Table.GetCurSel();
	//把新的页面显示出来
	pDialog[m_CurSelTab]->ShowWindow(SW_SHOW);
	*pResult = 0;
}

INT CUdsUpdateToolDlg::TransmitCanmsg(VCI_CAN_OBJ *SendObj)
{
	CUdsUpdateToolDlg *dlg = (CUdsUpdateToolDlg*)AfxGetApp()->GetMainWnd();

	INT flag;
	//调用动态链接库发送函数
	flag = VCI_Transmit(VCI_USBCAN2, CAN_DEVINDEX, dlg->UdsClient.m_CanChnl, SendObj, 2);//CAN message send

	if (flag < 1)
	{
		return flag;
	}

	dlg->m_MainPage.InsertItem(1, SendObj);
	return flag;
}

UINT CUdsUpdateToolDlg::ReceiveThread(LPVOID v)
{
	CUdsUpdateToolDlg *dlg = (CUdsUpdateToolDlg*)AfxGetApp()->GetMainWnd();

	INT NumValue;
	INT num = 0;
	VCI_CAN_OBJ pCanObj[200];

	CString str;

	DWORD ReceivedID;

	while (1)
	{

		//调用动态链接库接收函数
		NumValue = VCI_Receive(VCI_USBCAN2, CAN_DEVINDEX, dlg->UdsClient.m_CanChnl, pCanObj, 200, 0);

		for (num = 0; num<NumValue; num++)
		{
			ReceivedID = pCanObj[num].ID;
			if (ReceivedID == dlg->UdsClient.m_Rspid)
				dlg->UdsClient.netowrk_recv_frame(0, pCanObj[num].Data, pCanObj[num].DataLen);
			dlg->m_MainPage.InsertItem(0, &pCanObj[num]);
		}

		if (dlg->UdsClient.n_ResultErr == TRUE)
		{
			dlg->UdsClient.n_ResultErr = FALSE;
			str.Format(_T(">>NetWork Err %u"), dlg->UdsClient.n_Result);
			dlg->m_MainPage.PrintLog(0, str);
		}

		Sleep(5);
	}

	return 1;
}

UINT CUdsUpdateToolDlg::UdsMainThread(LPVOID v)
{
	BYTE main_ret;
	CString str;
	CUdsClient *pObj = (CUdsClient *)v;
	CUdsUpdateToolDlg *dlg = (CUdsUpdateToolDlg*)AfxGetApp()->GetMainWnd();
	while (1)
	{
		main_ret = pObj->main_loop();
		if (main_ret != 0) {
			str.Format(_T(">>uds main loop. error %d"), main_ret);
			dlg->m_MainPage.PrintLog(0, str);
		}
		Sleep(1); /* 1ms */
	}
}

// CUdsUpdateToolDlg 消息处理程序

void CUdsUpdateToolDlg::OnMenuOpendev()
{
	// TODO: 在此添加命令处理程序代码
	COpenDevDlg  Dlg;
	INT DlgRet;

	m_MainPage.PrintLog(0, _T(">>Open Can Device"));
	DlgRet = Dlg.DoModal();

	m_Bgnid = Dlg.m_Bgnid;
	m_Endid = Dlg.m_Endid;
	m_CanChnl = Dlg.m_CanChnl;
	m_FilterEn = Dlg.m_FilterEn;
}

void CUdsUpdateToolDlg::OnMenuClosdev()
{
	// TODO: 在此添加命令处理程序代码
	m_MainPage.PrintLog(0, _T(">>Close Can Device"));

	if (VCI_CloseDevice(VCI_USBCAN2, CAN_DEVINDEX) != 1)
	{
		m_MainPage.PrintLog(0, _T("      Fail"));
	}
	else
	{
		m_MainPage.PrintLog(0, _T("      Done"));
	}
}


void CUdsUpdateToolDlg::OnMenuStartUpdate()
{
	// TODO: 在此添加命令处理程序代码
	CUpdateDLg Dlg;
	Dlg.pClient = &UdsClient;
	Dlg.pMainPage = &m_MainPage;
	INT nRet = Dlg.DoModal();
	if (nRet == IDOK)
	{
		if (UdsClient.start_upgrade() == 0) {
			m_MainPage.PrintLog(0, _T(">>Start Update MCU"));
		}
		else {
			m_MainPage.PrintLog(0, _T(">>Update Task Is Already Running"));
		}
	}
	else
	{
		m_MainPage.PrintLog(0, _T(">>Start Update MCU Failed"));
	}

}


void CUdsUpdateToolDlg::OnMenuStopUpdate()
{
	// TODO: 在此添加命令处理程序代码
}



void CUdsUpdateToolDlg::OnMenuEcuReset()
{
	// TODO: 在此添加命令处理程序代码
	UdsCmd CmdNew;

	//Request Externded session
	CmdNew.SID = SID_10;
	CmdNew.CmdBuf[0] = 0x03;
	CmdNew.CmdLen = 1;
	UdsClient.push_cmd(CmdNew);

	//Push request cmd, request seed
	CmdNew.SID = SID_27;
	CmdNew.CmdBuf[0] = 0x01;
	CmdNew.CmdLen = 1;
	UdsClient.push_cmd(CmdNew);

	//Push request cmd, send key
	CmdNew.SID = SID_27;
	CmdNew.CmdBuf[0] = 0x02;
	CmdNew.CmdLen = 5;
	UdsClient.push_cmd(CmdNew);

	//Request Reset
	CmdNew.SID = SID_11;
	CmdNew.CmdBuf[0] = 0x01;
	CmdNew.CmdLen = 1;
	UdsClient.push_cmd(CmdNew);
}


void CUdsUpdateToolDlg::OnMenuSessionStd()
{
	// TODO: 在此添加命令处理程序代码
	UdsCmd CmdNew;
	//Request Externded session
	CmdNew.SID = SID_10;
	CmdNew.CmdBuf[0] = 0x01;
	CmdNew.CmdLen = 1;
	UdsClient.push_cmd(CmdNew);
}


void CUdsUpdateToolDlg::OnMenuSessionEol()
{
	// TODO: 在此添加命令处理程序代码
	UdsCmd CmdNew;
	//Request Externded session
	CmdNew.SID = SID_10;
	CmdNew.CmdBuf[0] = 0x03;
	CmdNew.CmdLen = 1;
	UdsClient.push_cmd(CmdNew);
}
