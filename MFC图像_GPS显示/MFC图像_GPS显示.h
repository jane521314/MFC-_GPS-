
// MFCͼ��_GPS��ʾ.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMFCͼ��_GPS��ʾApp: 
// �йش����ʵ�֣������ MFCͼ��_GPS��ʾ.cpp
//

class CMFCͼ��_GPS��ʾApp : public CWinApp
{
public:
	CMFCͼ��_GPS��ʾApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMFCͼ��_GPS��ʾApp theApp;