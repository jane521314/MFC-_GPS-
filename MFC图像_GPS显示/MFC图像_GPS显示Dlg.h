
// MFC图像_GPS显示Dlg.h : 头文件
//

#pragma once
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "afxwin.h"
#include "SerialCom.h"
using namespace cv;

// CMFC图像_GPS显示Dlg 对话框
class CMFC图像_GPS显示Dlg : public CDialogEx
{
// 构造
public:
	CMFC图像_GPS显示Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFC_GPS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedstart();
	afx_msg void OnBnClickedstop();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CComboBox m_serial1;
	afx_msg void OnBnClickedopenserial();
	afx_msg LRESULT OnReceiveData(WPARAM wParam, LPARAM lParam);//WPARAM ch, LPARAM port
	
};
