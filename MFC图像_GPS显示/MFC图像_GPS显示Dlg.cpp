
// MFCͼ��_GPS��ʾDlg.cpp : ʵ���ļ�
//
#include "stdafx.h"
#include "MFCͼ��_GPS��ʾ.h"
#include "MFCͼ��_GPS��ʾDlg.h"
#include "afxdialogex.h"
#include "opencv2\opencv.hpp"
#include <iostream>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\highgui\highgui_c.h>
#include "opencv2/imgproc/imgproc_c.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include "SerialCom.h"
#include <chrono>//����ϵͳʱ����غ���

using namespace cv;
using namespace std;

VideoCapture video1;
VideoWriter  output;
Mat img1, img2;
IplImage img3;
CSerialCom port1;
double qw, qx, qy, qz;											//��̬��Ԫ��
double roll, pitch, yaw;										//��̬��
int	   compass_x, compass_y, compass_z;							//���������
double latitude, longitude, altitude;							//γ�ȣ����ȣ��߶�
double acceleration_x, acceleration_y, acceleration_z;			 //������ٶ�
double velocity_x,velocity_y, velocity_z;						//�����ٶ�
double anguler_rate_x, anguler_rate_y, anguler_rate_z;			//������ٶ�
double gimbal_angles_pitch, gimbal_angles_roll, gimbal_angles_yaw;//������̨�Ƕ�
char   buff[64];												//���յ��ַ���
char   *p_data = buff;
int    recv_flag = 0;											//�жϽ�������

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CMFCͼ��_GPS��ʾDlg �Ի���



CMFCͼ��_GPS��ʾDlg::CMFCͼ��_GPS��ʾDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFC_GPS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCͼ��_GPS��ʾDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_serial1);
}

BEGIN_MESSAGE_MAP(CMFCͼ��_GPS��ʾDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_start, &CMFCͼ��_GPS��ʾDlg::OnBnClickedstart)
	ON_BN_CLICKED(IDC_stop, &CMFCͼ��_GPS��ʾDlg::OnBnClickedstop)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_open_serial, &CMFCͼ��_GPS��ʾDlg::OnBnClickedopenserial)
	ON_MESSAGE(WM_COMM_RXCHAR, &OnReceiveData)//��������Ϣ
END_MESSAGE_MAP()


// CMFCͼ��_GPS��ʾDlg ��Ϣ�������

BOOL CMFCͼ��_GPS��ʾDlg::OnInitDialog()
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
	cvNamedWindow("view", WINDOW_AUTOSIZE);//cvNamedWindow
	HWND hWnd = (HWND)cvGetWindowHandle("view");
	HWND hParent = ::GetParent(hWnd); 
	::SetParent(hWnd, GetDlgItem(IDC_video)->m_hWnd); 
	::ShowWindow(hParent, SW_HIDE);

	m_serial1.AddString(_T("1")); 
	m_serial1.AddString(_T("2")); 
	m_serial1.AddString(_T("3"));
	m_serial1.AddString(_T("4")); 
	m_serial1.AddString(_T("5"));
	m_serial1.AddString(_T("6")); 
	m_serial1.AddString(_T("7")); 
	m_serial1.AddString(_T("8")); 
	m_serial1.AddString(_T("9")); 
	m_serial1.SetCurSel(0);			//��ʼʱ�����б�ΪCOM1
	
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMFCͼ��_GPS��ʾDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMFCͼ��_GPS��ʾDlg::OnPaint()
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
HCURSOR CMFCͼ��_GPS��ʾDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMFCͼ��_GPS��ʾDlg::OnBnClickedstart()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GetDlgItem(IDC_start)->EnableWindow(FALSE);					//��ť���
	video1 = VideoCapture("C:/Users/DELL/Desktop/DJI_0221.MOV");//VideoCapture(1)

	struct tm now_time;
	time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	localtime_s(&now_time, &t); 
	stringstream ss; 
	ss << "F:/vs_project/MFCͼ��_GPS��ʾ/������Ƶ/";
	ss <<  put_time(&now_time, "%F ");
	ss << now_time.tm_hour << "ʱ" << now_time.tm_min << "��" << now_time .tm_sec<< "��" << ".mp4";
	output = VideoWriter(ss.str(), CV_FOURCC('H','2','6','4'),30,Size(1280,720));
	
	GetDlgItem(IDC_stop)->EnableWindow(TRUE);
	//port1.RestartMonitoring();
	
	SetTimer(1, 30, NULL);
}

void CMFCͼ��_GPS��ʾDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	video1 >> img1;
	if (img1.empty())
	{
		KillTimer(1);
		MessageBox(TEXT("��Ƶ���Ž�����"));
		//port1.StopMonitoring();
		GetDlgItem(IDC_start)->EnableWindow(TRUE); 
		output.release();
		return;
	}
	output << img1;

	resize(img1, img1, Size(768, 432));
	img3 = img1; //IplImage img3;
	//cvFlip(&img3, &img3, 1);
	cvShowImage("view", &img3);//cvShowImage
	CDialogEx::OnTimer(nIDEvent);
}

void CMFCͼ��_GPS��ʾDlg::OnBnClickedstop()
{
	//dakaishexiangtou.release();
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	port1.StopMonitoring();

	KillTimer(1);
	video1.release();
	output.release();
	GetDlgItem(IDC_start)->EnableWindow(TRUE);
	GetDlgItem(IDC_stop)->EnableWindow(TRUE);
	GetDlgItem(IDC_open_serial)->EnableWindow(TRUE);
}


void CMFCͼ��_GPS��ʾDlg::OnBnClickedopenserial()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GetDlgItem(IDC_open_serial)->EnableWindow(FALSE);
	CString key1,key2;
	GetDlgItem(IDC_COMBO1)->GetWindowText(key1);
	//GetDlgItem(IDC_EDITdata)->SetWindowText((LPCTSTR)(LPCTSTR)key1);
	CString key;
	GetDlgItem(IDC_EDITdata)->SetWindowText( key +"COM"+key1+"\r\n");
	
	if (port1.InitPort(this, atoi(CW2A(key1.GetString())), 57600, 'N', 8, 1, EV_RXFLAG | EV_RXCHAR, 512))
	{						//atoi��stringת����int��CW2A�����ַ�����Unicode��ת��Ϊ���ַ�����ASCII��
		MessageBox(TEXT("����1��"));
		port1.StartMonitoring();
		GetDlgItem(IDC_start)->EnableWindow(TRUE);

		//m_bSerialPortOpened = TRUE;
	}
	else
	{
		MessageBox(TEXT("û�з��ִ˴��ڻ�ռ��"));
		GetDlgItem(IDC_open_serial)->EnableWindow(TRUE);
		//m_bSerialPortOpened = FALSE;
	}
}
LRESULT CMFCͼ��_GPS��ʾDlg::OnReceiveData(WPARAM wParam, LPARAM lParam)//WPARAM ch, LPARAM port
{
	if ((char)wParam == 'A') { recv_flag = 1; return 0; }
	if ((char)wParam == 'B') { recv_flag = 2; return 0; }
	if ((char)wParam == 'C') { recv_flag = 3; return 0; }
	if ((char)wParam == 'D') { recv_flag = 4; return 0; }
	if ((char)wParam == 'E') { recv_flag = 5; return 0; }
	if ((char)wParam == 'F') { recv_flag = 6; return 0; }
	if ((char)wParam == 'G') { recv_flag = 7; return 0; }
	if ((char)wParam != '\n')
	{
		*p_data = (char)wParam;
		p_data++;
		return 0;
	}
	else {
		p_data = buff;
		switch (recv_flag) {
		case 1: 
			sscanf_s(buff, "%d\t%d\t%d",    &compass_x, &compass_y, &compass_z);
			break;
		case 2:
			sscanf_s(buff, "%lf\t%lf\t%lf", &latitude, &longitude, &altitude);
			break;
		case 3:
			sscanf_s(buff, "%lf\t%lf\t%lf", &acceleration_x, &acceleration_y, &acceleration_z);
			break;
		case 4:
			sscanf_s(buff, "%lf\t%lf\t%lf", &velocity_x, &velocity_y, &velocity_z);
			break;
		case 5:
			sscanf_s(buff, "%lf\t%lf\t%lf", &anguler_rate_x, &anguler_rate_y, &anguler_rate_z);
			break;
		case 6:
			sscanf_s(buff, "%lf\t%lf\t%lf", &gimbal_angles_pitch, &gimbal_angles_roll, &gimbal_angles_yaw);
			break;
		case 7:
			sscanf_s(buff, "%lf\t%lf\t%lf\t%lf", &qw, &qx, &qy, &qz);
			yaw = atan2(2.0f*(qw*qx + qy*qz), 1.0f - 2.0f*(qx*qx + qy*qy))* 180.0f / 3.14159265;
			pitch = asin(2.0f*(qw*qy - qx*qz))* 180.0f / 3.14159265;
			roll = atan2(2.0f*(qw*qz + qx*qy),1.0f-2.0f*(qy*qy + qz*qz))* 180.0f / 3.14159265;			
			/*
			(fai) angle.yaw   = atan2(2*(q.w*q.x + q.y*q.z),1-2*(q.x*q.x + q.y*q.y)) * 180.0 / M_PI;
			(sita)angle.pitch = asin(2*(q.w*q.y - q.z*q.x)) * 180.0 / M_PI;
			(kesi)angle.roll  = atan2(2*(q.w*q.z + q.x*q.y),1-2*(q.y*q.y + q.z*q.z)) * 180.0 / M_PI;*/
			break;
		}
	}
	//GetDlgItem(IDC_EDITdata);
	stringstream s1,s2,s3,s4,s5,s6,s7;
	CString key;
	//key.Format(_T("%d"), lParam);
	//key1 =lat;
	//if (key == '\r') key1 = L"/n";
	//GetDlgItem(IDC_EDITdata)->SetWindowText( key+"port:  "+key1+"/r/n");
	//((CEdit*)GetDlgItem(IDC_EDITdata))->SetSel(GetDlgItem(IDC_EDITdata)->GetWindowTextLength(), GetDlgItem(IDC_EDITdata)->GetWindowTextLength());
	//((CEdit*)GetDlgItem(IDC_EDITdata))->ReplaceSel(key);//+ L"\n"
	s1 << latitude;	
	key = CA2W(s1.str().c_str());//CA2W    �����ַ�����ASCII��ת��Ϊ���ַ�����Unicode��
	GetDlgItem(IDC_EDIT1)->SetWindowTextW(key);
	s2 << longitude;
	key = CA2W(s2.str().c_str());//CA2W    �����ַ�����ASCII��ת��Ϊ���ַ�����Unicode��
	GetDlgItem(IDC_EDIT2)->SetWindowTextW(key);
	s3 << altitude;
	key = CA2W(s3.str().c_str());//CA2W    �����ַ�����ASCII��ת��Ϊ���ַ�����Unicode��
	GetDlgItem(IDC_EDIT3)->SetWindowTextW(key);
	s4 << (double)(atan2((double)compass_y, (double)compass_x) * ((double)180.0f / 3.14159265));
	key = CA2W(s4.str().c_str());//CA2W    �����ַ�����ASCII��ת��Ϊ���ַ�����Unicode��
	GetDlgItem(IDC_EDIT4)->SetWindowTextW(key);
	s5 << yaw;
	key = CA2W(s5.str().c_str());//CA2W    �����ַ�����ASCII��ת��Ϊ���ַ�����Unicode��
	GetDlgItem(IDC_EDIT5)->SetWindowTextW(key);
	s6 << pitch;
	key = CA2W(s6.str().c_str());//CA2W    �����ַ�����ASCII��ת��Ϊ���ַ�����Unicode��
	GetDlgItem(IDC_EDIT6)->SetWindowTextW(key);
	s7 << roll;
	key = CA2W(s7.str().c_str());//CA2W    �����ַ�����ASCII��ת��Ϊ���ַ�����Unicode��
	GetDlgItem(IDC_EDIT7)->SetWindowTextW(key);
	return 0;
}