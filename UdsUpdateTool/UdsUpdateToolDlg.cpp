
// UdsUpdateToolDlg.cpp : ʵ���ļ�
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


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CUdsUpdateToolDlg �Ի���



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


// CUdsUpdateToolDlg ��Ϣ�������

BOOL CUdsUpdateToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	//���Menu
	m_Menu.LoadMenu(IDR_MENU_MAIN);
	SetMenu(&m_Menu);

	//ΪTable control ����ҳ��
	m_Table.InsertItem(0, _T("����"));
	m_Table.InsertItem(1, _T("�ļ�"));

	//�����Ի���
	m_MainPage.Create(IDD_DIALOG_MAIN, &m_Table);
	m_DealFile.Create(IDD_DIALOG_DEALFILE, &m_Table);

	//�趨��Table����ʾ�ķ�Χ
	CRect rc;
	m_Table.GetClientRect(rc);
	rc.top += 23;
	rc.bottom -= 0;
	rc.left += 0;
	rc.right -= 0;
	m_MainPage.MoveWindow(&rc);
	m_DealFile.MoveWindow(&rc);


	//�ѶԻ������ָ�뱣������
	pDialog[0] = &m_MainPage;
	pDialog[1] = &m_DealFile;
	//��ʾ��ʼҳ��
	pDialog[0]->ShowWindow(SW_SHOW);
	pDialog[1]->ShowWindow(SW_HIDE);

	//���浱ǰѡ��
	m_CurSelTab = 0;

	//���������߳�
	AfxBeginThread(ReceiveThread, 0);
	//����UDS�߳�
	AfxBeginThread(UdsMainThread, &UdsClient);

	//Create Event
	hUpdateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CUdsUpdateToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CUdsUpdateToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CUdsUpdateToolDlg::OnTcnSelchangeTabMain(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//�ѵ�ǰ��ҳ����������
	pDialog[m_CurSelTab]->ShowWindow(SW_HIDE);
	//�õ��µ�ҳ������
	m_CurSelTab = m_Table.GetCurSel();
	//���µ�ҳ����ʾ����
	pDialog[m_CurSelTab]->ShowWindow(SW_SHOW);
	*pResult = 0;
}

INT CUdsUpdateToolDlg::TransmitCanmsg(VCI_CAN_OBJ *SendObj)
{
	CUdsUpdateToolDlg *dlg = (CUdsUpdateToolDlg*)AfxGetApp()->GetMainWnd();

	INT flag;
	//���ö�̬���ӿⷢ�ͺ���
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

		//���ö�̬���ӿ���պ���
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

// CUdsUpdateToolDlg ��Ϣ�������

void CUdsUpdateToolDlg::OnMenuOpendev()
{
	// TODO: �ڴ���������������
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
	// TODO: �ڴ���������������
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
	// TODO: �ڴ���������������
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
	// TODO: �ڴ���������������
}



void CUdsUpdateToolDlg::OnMenuEcuReset()
{
	// TODO: �ڴ���������������
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
	// TODO: �ڴ���������������
	UdsCmd CmdNew;
	//Request Externded session
	CmdNew.SID = SID_10;
	CmdNew.CmdBuf[0] = 0x01;
	CmdNew.CmdLen = 1;
	UdsClient.push_cmd(CmdNew);
}


void CUdsUpdateToolDlg::OnMenuSessionEol()
{
	// TODO: �ڴ���������������
	UdsCmd CmdNew;
	//Request Externded session
	CmdNew.SID = SID_10;
	CmdNew.CmdBuf[0] = 0x03;
	CmdNew.CmdLen = 1;
	UdsClient.push_cmd(CmdNew);
}
