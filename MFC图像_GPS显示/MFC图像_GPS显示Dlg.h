
// MFCͼ��_GPS��ʾDlg.h : ͷ�ļ�
//

#pragma once
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "afxwin.h"
#include "SerialCom.h"
using namespace cv;

// CMFCͼ��_GPS��ʾDlg �Ի���
class CMFCͼ��_GPS��ʾDlg : public CDialogEx
{
// ����
public:
	CMFCͼ��_GPS��ʾDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFC_GPS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
