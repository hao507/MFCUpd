#pragma once


// CLimitEdit

class CLimitHexEdit : public CEdit
{
	DECLARE_DYNAMIC(CLimitHexEdit)

public:
	CLimitHexEdit();
	virtual ~CLimitHexEdit();

protected:
	DECLARE_MESSAGE_MAP()

public:

	// Set the Max length of input
	void SetLimitLen(UINT nLimitLen);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};


#pragma once


// CLimitDecEdit

class CLimitDecEdit : public CEdit
{
	DECLARE_DYNAMIC(CLimitDecEdit)

public:
	CLimitDecEdit();
	virtual ~CLimitDecEdit();

protected:
	DECLARE_MESSAGE_MAP()

public:
	// Set the Max length of input
	void SetLimitLen(UINT nLimitLen);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};


