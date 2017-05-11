// LimitEdit.cpp : 实现文件
//

#include "stdafx.h"
#include "LimitEdit.h"


// CLimitEdit Class

IMPLEMENT_DYNAMIC(CLimitHexEdit, CEdit)

CLimitHexEdit::CLimitHexEdit()
{

}

CLimitHexEdit::~CLimitHexEdit()
{
}


BEGIN_MESSAGE_MAP(CLimitHexEdit, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()



// CLimitEdit 消息处理程序




// Set the Max length of input
void CLimitHexEdit::SetLimitLen(UINT nLimitLen)
{
	SetLimitText(nLimitLen);
}


void CLimitHexEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (!(nChar >= _T('0') && nChar <= _T('9') || nChar >= _T('A') && nChar <= _T('F') || nChar >= _T('a') && nChar <= _T('f') || nChar == VK_BACK || nChar == VK_SPACE))
		return;

	CEdit::OnChar(nChar, nRepCnt, nFlags);
}



// CLimitDecEdit Class

IMPLEMENT_DYNAMIC(CLimitDecEdit, CEdit)

CLimitDecEdit::CLimitDecEdit()
{

}

CLimitDecEdit::~CLimitDecEdit()
{
}


BEGIN_MESSAGE_MAP(CLimitDecEdit, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()



// CLimitDecEdit 消息处理程序


// Set the Max length of input
void CLimitDecEdit::SetLimitLen(UINT nLimitLen)
{
	SetLimitText(nLimitLen);
}


void CLimitDecEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (!(nChar >= _T('0') && nChar <= _T('9') || nChar == VK_BACK || nChar == VK_SPACE))
		return;

	CEdit::OnChar(nChar, nRepCnt, nFlags);
}