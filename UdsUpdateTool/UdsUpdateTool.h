
// UdsUpdateTool.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CUdsUpdateToolApp: 
// �йش����ʵ�֣������ UdsUpdateTool.cpp
//

class CUdsUpdateToolApp : public CWinApp
{
public:
	CUdsUpdateToolApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CUdsUpdateToolApp theApp;