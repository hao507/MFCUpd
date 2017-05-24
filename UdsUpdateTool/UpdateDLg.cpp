// UpdateDLg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "UdsUpdateTool.h"
#include "UpdateDLg.h"
#include "afxdialogex.h"


// CUpdateDLg �Ի���

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

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	m_CstrPath = _T("E:\\");
	//GetDlgItem(IDC_EDIT_FILE_PATH)->SetWindowText(m_CstrPath);
	m_EditPath.SetWindowTextW(m_CstrPath);
	//UpdateData(false);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}

// CUpdateDLg ��Ϣ�������



void CUpdateDLg::OnBnClickedButtonFilePath()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	BOOL isOpen = TRUE;     //�Ƿ��(����Ϊ����)  
	CString defaultDir = _T("E:\\FileTest");   //Ĭ�ϴ򿪵��ļ�·��  
	CString fileName = _T("");         //Ĭ�ϴ򿪵��ļ���  
	CString filter = _T("�ļ� (*.hex; *.bin)|*.hex;*.bin||");   //�ļ����ǵ�����  
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);//��������

}


void CUpdateDLg::OnOK()
{
	// TODO: �ڴ����ר�ô����/����û���
	CDialogEx::OnOK();
}
