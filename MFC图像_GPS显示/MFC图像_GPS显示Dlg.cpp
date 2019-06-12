
// MFC图像_GPS显示Dlg.cpp : 实现文件
//
#include "stdafx.h"
#include "MFC图像_GPS显示.h"
#include "MFC图像_GPS显示Dlg.h"
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
#include <chrono>//包含系统时间相关函数

using namespace cv;
using namespace std;

VideoCapture video1;
VideoWriter  output;
Mat img1, img2;
IplImage img3;
CSerialCom port1;
double qw, qx, qy, qz;											//姿态四元数
double roll, pitch, yaw;										//姿态角
int	   compass_x, compass_y, compass_z;							//三轴磁力计
double latitude, longitude, altitude;							//纬度，经度，高度
double acceleration_x, acceleration_y, acceleration_z;			 //三轴加速度
double velocity_x,velocity_y, velocity_z;						//三轴速度
double anguler_rate_x, anguler_rate_y, anguler_rate_z;			//三轴角速度
double gimbal_angles_pitch, gimbal_angles_roll, gimbal_angles_yaw;//三轴云台角度
char   buff[64];												//接收的字符串
char   *p_data = buff;
int    recv_flag = 0;											//判断接收类型

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CMFC图像_GPS显示Dlg 对话框



CMFC图像_GPS显示Dlg::CMFC图像_GPS显示Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFC_GPS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFC图像_GPS显示Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_serial1);
}

BEGIN_MESSAGE_MAP(CMFC图像_GPS显示Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_start, &CMFC图像_GPS显示Dlg::OnBnClickedstart)
	ON_BN_CLICKED(IDC_stop, &CMFC图像_GPS显示Dlg::OnBnClickedstop)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_open_serial, &CMFC图像_GPS显示Dlg::OnBnClickedopenserial)
	ON_MESSAGE(WM_COMM_RXCHAR, &OnReceiveData)//处理串口消息
END_MESSAGE_MAP()


// CMFC图像_GPS显示Dlg 消息处理程序

BOOL CMFC图像_GPS显示Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
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
	m_serial1.SetCurSel(0);			//初始时下拉列表为COM1
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFC图像_GPS显示Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFC图像_GPS显示Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFC图像_GPS显示Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMFC图像_GPS显示Dlg::OnBnClickedstart()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_start)->EnableWindow(FALSE);					//按钮变灰
	video1 = VideoCapture("C:/Users/DELL/Desktop/DJI_0221.MOV");//VideoCapture(1)

	struct tm now_time;
	time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	localtime_s(&now_time, &t); 
	stringstream ss; 
	ss << "F:/vs_project/MFC图像_GPS显示/采样视频/";
	ss <<  put_time(&now_time, "%F ");
	ss << now_time.tm_hour << "时" << now_time.tm_min << "分" << now_time .tm_sec<< "秒" << ".mp4";
	output = VideoWriter(ss.str(), CV_FOURCC('H','2','6','4'),30,Size(1280,720));
	
	GetDlgItem(IDC_stop)->EnableWindow(TRUE);
	//port1.RestartMonitoring();
	
	SetTimer(1, 30, NULL);
}

void CMFC图像_GPS显示Dlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	video1 >> img1;
	if (img1.empty())
	{
		KillTimer(1);
		MessageBox(TEXT("视频播放结束！"));
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

void CMFC图像_GPS显示Dlg::OnBnClickedstop()
{
	//dakaishexiangtou.release();
	// TODO: 在此添加控件通知处理程序代码
	port1.StopMonitoring();

	KillTimer(1);
	video1.release();
	output.release();
	GetDlgItem(IDC_start)->EnableWindow(TRUE);
	GetDlgItem(IDC_stop)->EnableWindow(TRUE);
	GetDlgItem(IDC_open_serial)->EnableWindow(TRUE);
}


void CMFC图像_GPS显示Dlg::OnBnClickedopenserial()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_open_serial)->EnableWindow(FALSE);
	CString key1,key2;
	GetDlgItem(IDC_COMBO1)->GetWindowText(key1);
	//GetDlgItem(IDC_EDITdata)->SetWindowText((LPCTSTR)(LPCTSTR)key1);
	CString key;
	GetDlgItem(IDC_EDITdata)->SetWindowText( key +"COM"+key1+"\r\n");
	
	if (port1.InitPort(this, atoi(CW2A(key1.GetString())), 57600, 'N', 8, 1, EV_RXFLAG | EV_RXCHAR, 512))
	{						//atoi将string转数字int，CW2A将宽字符集（Unicode）转化为多字符集（ASCII）
		MessageBox(TEXT("串口1打开"));
		port1.StartMonitoring();
		GetDlgItem(IDC_start)->EnableWindow(TRUE);

		//m_bSerialPortOpened = TRUE;
	}
	else
	{
		MessageBox(TEXT("没有发现此串口或被占用"));
		GetDlgItem(IDC_open_serial)->EnableWindow(TRUE);
		//m_bSerialPortOpened = FALSE;
	}
}
LRESULT CMFC图像_GPS显示Dlg::OnReceiveData(WPARAM wParam, LPARAM lParam)//WPARAM ch, LPARAM port
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
	key = CA2W(s1.str().c_str());//CA2W    将多字符集（ASCII）转化为宽字符集（Unicode）
	GetDlgItem(IDC_EDIT1)->SetWindowTextW(key);
	s2 << longitude;
	key = CA2W(s2.str().c_str());//CA2W    将多字符集（ASCII）转化为宽字符集（Unicode）
	GetDlgItem(IDC_EDIT2)->SetWindowTextW(key);
	s3 << altitude;
	key = CA2W(s3.str().c_str());//CA2W    将多字符集（ASCII）转化为宽字符集（Unicode）
	GetDlgItem(IDC_EDIT3)->SetWindowTextW(key);
	s4 << (double)(atan2((double)compass_y, (double)compass_x) * ((double)180.0f / 3.14159265));
	key = CA2W(s4.str().c_str());//CA2W    将多字符集（ASCII）转化为宽字符集（Unicode）
	GetDlgItem(IDC_EDIT4)->SetWindowTextW(key);
	s5 << yaw;
	key = CA2W(s5.str().c_str());//CA2W    将多字符集（ASCII）转化为宽字符集（Unicode）
	GetDlgItem(IDC_EDIT5)->SetWindowTextW(key);
	s6 << pitch;
	key = CA2W(s6.str().c_str());//CA2W    将多字符集（ASCII）转化为宽字符集（Unicode）
	GetDlgItem(IDC_EDIT6)->SetWindowTextW(key);
	s7 << roll;
	key = CA2W(s7.str().c_str());//CA2W    将多字符集（ASCII）转化为宽字符集（Unicode）
	GetDlgItem(IDC_EDIT7)->SetWindowTextW(key);
	return 0;
}