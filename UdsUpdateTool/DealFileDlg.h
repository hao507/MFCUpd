#pragma once


// CDealFileDlg 对话框

class CDealFileDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDealFileDlg)

public:
	CDealFileDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDealFileDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DEALFILE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
