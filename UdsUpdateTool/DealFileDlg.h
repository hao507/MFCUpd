#pragma once


// CDealFileDlg �Ի���

class CDealFileDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDealFileDlg)

public:
	CDealFileDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDealFileDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DEALFILE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
};
