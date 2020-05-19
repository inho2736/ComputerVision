
// RGBDlg.cpp: 구현 파일
//
#include "math.h"
#include "vector"
#include "limits"
//
#include "pch.h"
#include "framework.h"
//#include "stdafx.h"
#include "RGB.h"
#include "RGBDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define FOREWARD 3
#define INF std::numeric_limits<double>::infinity()
using namespace std;
int findThreshold(Mat &m);
//
void RGBsep(int i, Mat img, Mat& copy, char* str);
void Grayscale(Mat img, Mat& copy, char* str);
void Otsu_binary(Mat img, Mat& copy, char* str);
void printAndsave(char* str, Mat& copy);
//
Mat Opening(Mat img, Mat& copy, int index);
Mat Closing(Mat img, Mat& copy, int index);
void Dilation(Mat img, Mat& copy, int element_x, int element_y);
void Erosion(Mat img, Mat& copy, int element_x, int element_y);
//
void read_neighbor8(int y, int x, int neighbor8[], Mat src_img);
void BTracing8(int y, int x, int label, int tag, int num_region[], Mat src_img, Mat* label_img, Mat* output_img, vector <pair<int, int>> &boundary);
Mat contourTracing(Mat input_img, int index, vector <pair<int, int>> &boundary);
void calCoord(int i, int* y, int* x);
//
double getLength(int x1, int y1, int x2, int y2, int rx, int ry);
void lcs(vector <pair<int, int>> &boundary, vector<double> &arr_lcs);
//
pair<double, int> calMin(double a, double b, double c);
double Dtw(vector <double> &arr_lcs1, vector <double> &arr_lcs2);
double d_func(double a, double b);
// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
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

// CRGBDlg 대화 상자

CRGBDlg::CRGBDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_RGB_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRGBDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Img, m_pic);
}

BEGIN_MESSAGE_MAP(CRGBDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_Img_Search, &CRGBDlg::OnBnClickedImgSearch)
	ON_BN_CLICKED(IDC_Img_Save, &CRGBDlg::OnBnClickedImgSave)
	
	ON_BN_CLICKED(IDC_Img_Search2, &CRGBDlg::OnBnClickedImgSearch2)
END_MESSAGE_MAP()
// CRGBDlg 메시지 처리기

BOOL CRGBDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CRGBDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CRGBDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CRGBDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CRGBDlg::OnBnClickedImgSearch()
{
	static TCHAR BASED_CODE szFilter[] = _T("이미지 파일(*.BMP, *.GIF, *.JPG) | *.BMP;*.GIF;*.JPG;*.bmp;*.jpg;*.gif |모든파일(*.*)|*.*||");
	CFileDialog dlg(TRUE, _T("*.jpg"), _T("image"), OFN_HIDEREADONLY, szFilter);
	if (IDOK == dlg.DoModal())
	{
		pathName = dlg.GetPathName();
		CT2CA pszConvertedAnsiString_up(pathName);
		std::string up_pathName_str(pszConvertedAnsiString_up);
		img = cv::imread(up_pathName_str);
		DisplayImage(img, 3);
		
	}
}
void CRGBDlg::OnBnClickedImgSearch2()
{
	static TCHAR BASED_CODE szFilter[] = _T("이미지 파일(*.BMP, *.GIF, *.JPG) | *.BMP;*.GIF;*.JPG;*.bmp;*.jpg;*.gif |모든파일(*.*)|*.*||");
	CFileDialog dlg(TRUE, _T("*.jpg"), _T("image"), OFN_HIDEREADONLY, szFilter);
	if (IDOK == dlg.DoModal())
	{
		pathName = dlg.GetPathName();
		CT2CA pszConvertedAnsiString_up(pathName);
		std::string up_pathName_str(pszConvertedAnsiString_up);
		img2 = cv::imread(up_pathName_str);
		//DisplayImage(img2, 3);

	}
}
void CRGBDlg::DisplayImage(Mat targetMat, int channel)
{
	CDC *pDC = nullptr;
	CImage* mfcImg = nullptr;

	pDC = m_pic.GetDC();
	CStatic *staticSize = (CStatic *)GetDlgItem(IDC_Img);
	staticSize->GetClientRect(rect);

	cv::Mat tempImage;
	cv::resize(targetMat, tempImage, Size(rect.Width(), rect.Height()));

	BITMAPINFO bitmapInfo;
	bitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	bitmapInfo.bmiHeader.biBitCount = 24;
	bitmapInfo.bmiHeader.biWidth = tempImage.cols;
	bitmapInfo.bmiHeader.biHeight = tempImage.rows;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	bitmapInfo.bmiHeader.biClrImportant = 0;
	bitmapInfo.bmiHeader.biClrUsed = 0;
	bitmapInfo.bmiHeader.biSizeImage = 0;
	bitmapInfo.bmiHeader.biXPelsPerMeter = 0;

	if (targetMat.channels() == 3)
	{
		mfcImg = new CImage();
		mfcImg->Create(tempImage.cols, tempImage.rows, 24);
	}
	else if (targetMat.channels() == 1)
	{
		cvtColor(tempImage, tempImage, COLOR_GRAY2RGB);
		mfcImg = new CImage();
		mfcImg->Create(tempImage.cols, tempImage.rows, 24);
	}
	else if (targetMat.channels() == 4)
	{
		bitmapInfo.bmiHeader.biBitCount = 32;
		mfcImg = new CImage();
		mfcImg->Create(tempImage.cols, tempImage.rows, 32);
	}
	cv::flip(tempImage, tempImage, 0);
	::StretchDIBits(mfcImg->GetDC(), 0, 0, tempImage.cols, tempImage.rows,
		0, 0, tempImage.cols, tempImage.rows, tempImage.data, &bitmapInfo,
		DIB_RGB_COLORS, SRCCOPY);

	mfcImg->BitBlt(::GetDC(m_pic.m_hWnd), 0, 0);

	if (mfcImg)
	{
		mfcImg->ReleaseDC();
		delete mfcImg;// mfcImg = nullptr;
	}
	tempImage.release();
	ReleaseDC(pDC);
}
int findThreshold(Mat &m)
{
	long double histogram[256] = { 0, };
	long double avg = 0;

	for (int y = 0; y < m.rows; y++)
	{
		unsigned char* ptr1 = m.data + 3 * (m.cols*y);
		for (int x = 0; x < m.cols; x++)
			histogram[(int)ptr1[3 * x + 0]] += 1;
	}

	for (int i = 0; i < 256; i++)
	{
		histogram[i] /= (m.cols * m.rows);
		avg += histogram[i] * i;
	}

	long double max = 0;
	int index = 0;
	for (int i = 1; i < 255; i++)
	{
		long double m0 = 0;
		long double m1 = 0;
		long double w0 = 0;
		long double w1 = 0;
		for (int j = 0; j < 256; j++)
		{
			if (j <= i)
			{
				w0 += histogram[j];
				m0 += (histogram[j] * j);
			}
			else
			{
				w1 += histogram[j];
				m1 += histogram[j] * j;
			}
		}

		if (w0 != 0)
			m0 /= w0;
		if (w1 != 0)
			m1 /= w1;

		long double b = w0 * (m0 - avg) * (m0 - avg) + w1 * (m1 - avg) * (m1 - avg);
		if (b >= max)
		{
			max = b;
			index = i;
		}
	}
	return index;
}

void CRGBDlg::OnBnClickedImgSave()
{
	char str_rgb[3][100] = { "red", "green", "blue" };
	char str_gray[3][100] = { "red_grayscale", "green_grayscale", "blue_grayscale" };
	char str_otsu[3][100] = { "red_otsu", "green_otsu", "blue_otsu" };
	
	Mat copy, gray, otsu, open, close, contour;	
	vector<double> arr_lcs1;
	vector<double> arr_lcs2;
	
	/*for (int i = 0; i < 3; i++) // 0-red, 1-green, 2-blue
	{
		copy = img.clone();
		RGBsep(i, img, copy, str_rgb[i]);
		gray = copy.clone();
		Grayscale(copy, gray, str_gray[i]);
		otsu = gray.clone();
		Otsu_binary(gray, otsu, str_otsu[i]);
		open = otsu.clone();
		open = Opening(otsu, open, i);
		close = otsu.clone();
		close = Closing(otsu, close, i);

		Mat open_c1 = Mat::zeros(open.rows, open.cols, CV_8UC1);

		for (int i = 0; i < open.rows; i++) {
			for (int j = 0; j < open.cols; j++) {
				if (open.data[3 * (open.cols * i + j)] == 255) {
					open_c1.data[open.cols * i + j] = 255;
				}
			}
		}
		Mat close_c1 = Mat::zeros(close.rows, close.cols, CV_8UC1);
		for (int i = 0; i < close.rows; i++) {
			for (int j = 0; j < close.cols; j++) {
				if (close.data[3 * (close.cols * i + j)] == 255) {
					close_c1.data[close.cols * i + j] = 255;
				}
			}
		}
		vector<pair<int, int>> boundary;
		
		contour = contourTracing(open_c1, i, boundary);
		
		getLength(1, 2, 2, 1, 2, 4);
		lcs(boundary, arr_lcs1);
	}*/
	
	copy = img.clone();
	RGBsep(0, img, copy, str_rgb[0]);
	gray = copy.clone();
	Grayscale(copy, gray, str_gray[0]);
	otsu = gray.clone();
	Otsu_binary(gray, otsu, str_otsu[0]);
	open = otsu.clone();
	open = Opening(otsu, open, 0);
	close = otsu.clone();
	close = Closing(otsu, close, 0);

	Mat open_c1 = Mat::zeros(open.rows, open.cols, CV_8UC1);

	for (int i = 0; i < open.rows; i++) {
		for (int j = 0; j < open.cols; j++) {
			if (open.data[3 * (open.cols * i + j)] == 255) {
				open_c1.data[open.cols * i + j] = 255;
			}
		}
	}
	Mat close_c1 = Mat::zeros(close.rows, close.cols, CV_8UC1);
	for (int i = 0; i < close.rows; i++) {
		for (int j = 0; j < close.cols; j++) {
			if (close.data[3 * (close.cols * i + j)] == 255) {
				close_c1.data[close.cols * i + j] = 255;
			}
		}
	}
	vector<pair<int, int>> boundary1;

	contour = contourTracing(open_c1, 0, boundary1);	
	lcs(boundary1, arr_lcs1);
	//////////////////////2222////////////////////////

	copy = img2.clone();
	RGBsep(0, img2, copy, str_rgb[0]);
	gray = copy.clone();
	Grayscale(copy, gray, str_gray[0]);
	otsu = gray.clone();
	Otsu_binary(gray, otsu, str_otsu[0]);
	open = otsu.clone();
	open = Opening(otsu, open, 0);
	close = otsu.clone();
	close = Closing(otsu, close, 0);

	Mat open_c2 = Mat::zeros(open.rows, open.cols, CV_8UC1);

	for (int i = 0; i < open.rows; i++) {
		for (int j = 0; j < open.cols; j++) {
			if (open.data[3 * (open.cols * i + j)] == 255) {
				open_c2.data[open.cols * i + j] = 255;
			}
		}
	}
	Mat close_c2 = Mat::zeros(close.rows, close.cols, CV_8UC1);
	for (int i = 0; i < close.rows; i++) {
		for (int j = 0; j < close.cols; j++) {
			if (close.data[3 * (close.cols * i + j)] == 255) {
				close_c2.data[close.cols * i + j] = 255;
			}
		}
	}
	vector<pair<int, int>> boundary2;

	contour = contourTracing(open_c2, 0, boundary2);
	lcs(boundary2, arr_lcs2);

	double k = Dtw(arr_lcs1, arr_lcs2);
	int l = 0;
}
void RGBsep(int i, Mat img, Mat& copy, char* str)
{
	if (i == 0)      //red
	{
		for (int y = 0; y < copy.rows; y++)
		{
			unsigned char* ptr1 = img.data + 3 * (img.cols*y);
			unsigned char* resultptr = copy.data + 3 * (copy.cols*y);
			for (int x = 0; x < copy.cols; x++)
			{
				resultptr[3 * x + 0] = 0;            //blue      
				resultptr[3 * x + 1] = 0;            //green
				resultptr[3 * x + 2] = ptr1[3 * x + 2];   //red
			}
		}
	}
	else if (i == 1)   //green
	{
		for (int y = 0; y < copy.rows; y++)
		{
			unsigned char* ptr1 = img.data + 3 * (img.cols*y);
			unsigned char* resultptr = copy.data + 3 * (copy.cols*y);
			for (int x = 0; x < copy.cols; x++)
			{
				resultptr[3 * x + 0] = 0;            //blue      
				resultptr[3 * x + 1] = ptr1[3 * x + 1];   //green
				resultptr[3 * x + 2] = 0;            //red
			}
		}
	}
	else if (i == 2)   //blue
	{
		for (int y = 0; y < copy.rows; y++)
		{
			unsigned char* ptr1 = img.data + 3 * (img.cols*y);
			unsigned char* resultptr = copy.data + 3 * (copy.cols*y);
			for (int x = 0; x < copy.cols; x++)
			{
				resultptr[3 * x + 0] = ptr1[3 * x + 0];
				resultptr[3 * x + 1] = 0;
				resultptr[3 * x + 2] = 0;
			}
		}
	}

	//printAndsave(str, copy);
}

void Grayscale(Mat img, Mat& copy, char* str)
{
	
	for (int y = 0; y < copy.rows; y++)
	{
		unsigned char* ptr1 = img.data + 3 * (img.cols*y);
		unsigned char* resultptr = copy.data + 3 * (copy.cols*y);
		for (int x = 0; x < copy.cols; x++)
		{
			unsigned char gray = (unsigned char)(ptr1[3 * x + 0] + ptr1[3 * x + 1] + ptr1[3 * x + 2]) / 3;
			resultptr[3 * x + 0] = gray;
			resultptr[3 * x + 1] = gray;
			resultptr[3 * x + 2] = gray;
		}
	}

	//printAndsave(str, copy);
}

void Otsu_binary(Mat img, Mat& copy, char* str)
{
	
	int threshold = findThreshold(img);
	for (int y = 0; y < copy.rows; y++)
	{
		unsigned char* ptr1 = img.data + 3 * (img.cols*y);
		unsigned char* resultptr = copy.data + 3 * (copy.cols*y);
		for (int x = 0; x < copy.cols; x++)
		{
			int bin = 0;
			if ((int)ptr1[3 * x] > threshold)
				bin = 255;

			resultptr[3 * x + 0] = bin;
			resultptr[3 * x + 1] = bin;
			resultptr[3 * x + 2] = bin;
		}
	}

	//printAndsave(str, copy);
}
Mat Opening(Mat img, Mat& copy, int index)
{
	Erosion(img, copy, 5, 5);
	Mat opening = copy.clone();
	Dilation(copy, opening, 5, 5);
	copy = opening.clone();
	if (index == 0) {
		printAndsave("red Opening", opening);		
	}
	else if (index == 1) {
		printAndsave("green Opening", opening);
	}
	else {
		printAndsave("blue Opening", opening);
	}
	return opening;
}

Mat Closing(Mat img, Mat& copy, int index)
{
	Dilation(img, copy, 5, 5);
	Mat closing = copy.clone();
	Erosion(copy, closing, 5, 5);
	if (index == 0) {
		printAndsave("red Closing", closing);
	}
	else if (index == 1) {
		printAndsave("green Closing", closing);
	}
	else {
		printAndsave("blue Closing", closing);
	}
	return closing;
	
}

void Dilation(Mat img, Mat& copy, int element_x, int element_y)
{

	int half_element_x = (int)element_x / 2;
	int half_element_y = (int)element_y / 2;
	for (int y = half_element_y; y < copy.rows - half_element_y; y++)
	{
		unsigned char* ptr1 = img.data + 3 * (img.cols*y);
		for (int x = half_element_x; x < copy.cols - half_element_x; x++)
		{
			if (ptr1[3 * x] == 255)
			{
				unsigned char* dilation = copy.data + 3 * (copy.cols*(y - half_element_y) + x - half_element_x);
				for (int i = 0; i < element_y; i++)
					for (int j = 0; j < element_x; j++)
					{
						dilation[3 * (i * copy.cols + j) + 0] = 255;
						dilation[3 * (i * copy.cols + j) + 1] = 255;
						dilation[3 * (i * copy.cols + j) + 2] = 255;
					}
			}
		}
	}
}

void Erosion(Mat img, Mat& copy, int element_x, int element_y)
{

	int half_element_x = (int)element_x / 2;
	int half_element_y = (int)element_y / 2;
	for (int y = half_element_y; y < copy.rows - half_element_y; y++)
	{
		for (int x = half_element_x; x < copy.cols - half_element_x; x++)
		{
			bool flag = true;
			
			unsigned char* ptr1 = img.data + 3 * (img.cols*(y - half_element_y) + x - half_element_x);
			for (int i = 0; i < element_y; i++)
				for (int j = 0; j < element_x; j++)
					if (ptr1[3 * (i * copy.cols + j)] != 255)
						flag = false;
			if (flag == false)
			{
				unsigned char* erosion = copy.data + 3 * (copy.cols*y);
				erosion[3 * x + 0] = 0;
				erosion[3 * x + 1] = 0;
				erosion[3 * x + 2] = 0;
			}
		}
	}
}
void printAndsave(char* str, Mat& copy)
{
	char tmp[100] = "\0";
	strcat_s(tmp, 100, str);
	strcat_s(tmp, 100, ".jpg");

	cv::imshow(str, copy);
	cv::imwrite(tmp, copy);
}

void calCoord(int i, int* y, int* x)
{
	switch (i)
	{
	case 0: *x = *x + 1; break;
	case 1: *y = *y + 1;  *x = *x + 1; break;
	case 2: *y = *y + 1; break;
	case 3: *y = *y + 1;  *x = *x - 1; break;
	case 4: *x = *x - 1; break;
	case 5: *y = *y - 1;  *x = *x - 1; break;
	case 6: *y = *y - 1; break;
	case 7: *y = *y - 1;  *x = *x + 1; break;
	}
}

Mat contourTracing(Mat input_img, int index, vector <pair<int, int>> &boundary)
{
	Mat src_img = input_img.clone();
	int rows = src_img.rows;
	int cols = src_img.cols;
	int labelnumber = 1;

	const int FORWARD = 0;
	const int BACKWARD = 1;

	int cur_p, ref_p1, ref_p2 = 0;

	int num_region[256];

	Mat label_img = Mat::zeros(src_img.rows, src_img.cols, CV_8UC1);
	Mat output_img = Mat::zeros(src_img.rows, src_img.cols, CV_8UC1);
	
	for (int i = 0; i < cols; i++) {
		src_img.data[i] = 0;
		src_img.data[cols * (rows - 1) + i] = 0;
	}
	for (int i = 0; i < rows; i++) {
		src_img.data[cols * i] = 0;
		src_img.data[cols * i + cols - 1] = 0;
	}

	for (int k = 0; k < 256; k++)
	{
		num_region[k] = 0;
	}

	for (int i = 1; i < rows - 1; i++) {
		for (int j = 1; j < cols - 1; j++) {
			cur_p = src_img.data[cols * i + j];
			if (cur_p == 255) {
				ref_p1 = label_img.data[cols * i + j - 1];
				ref_p2 = label_img.data[cols * (i - 1) + j - 1];
				if (ref_p1 > 1) { // propagation
					num_region[ref_p1]++;
					label_img.data[cols * i + j] = ref_p1;
				}
				else if ((ref_p1 == 0) && (ref_p2 >= 2)) { // hole
					if (label_img.data[cols * i + j] != 0)
						continue;
					num_region[ref_p2]++;
					label_img.data[cols * i + j] = ref_p2;
					BTracing8(i, j, ref_p2, BACKWARD, num_region, src_img, &label_img, &output_img, boundary);
				}
				else if ((ref_p1 == 0) && (ref_p2 == 0)) { // region start
					if (label_img.data[cols * i + j] != 0)
						continue;
					labelnumber++;
					num_region[labelnumber]++;
					label_img.data[cols * i + j] = labelnumber;
					BTracing8(i, j, labelnumber, FORWARD, num_region, src_img, &label_img, &output_img, boundary);
				}
			}
			else label_img.data[cols * i + j] = 0;
		}
	}

	for (int i = 1; i < rows - 1; i++) {
		for (int j = 1; j < cols - 1; j++) {
			if (label_img.data[cols * i + j] != 0)
				label_img.data[cols * i + j] = 255;
		}
	}
	if (index == 0) {
		printAndsave("red contour", output_img);
	}
	else if (index == 1) {
		printAndsave("green contour", output_img);
	}
	if (index == 2) {
		printAndsave("blue contour", output_img);
	}
	return output_img;
}

void BTracing8(int y, int x, int label, int tag, int num_region[], Mat src_img, Mat* label_img, Mat* output_img, vector <pair<int, int>> &boundary) {
	int cur_orient, pre_orient, end_x, end_y, pre_x, pre_y;
	int neighbor8[8];
	int start_o, add_o;
	int i;
	int rows = src_img.rows; int cols = src_img.cols;
	if (tag == 0) {
		cur_orient = pre_orient = 0;
	}
	else {
		cur_orient = pre_orient = 6;
	}
	end_x = pre_x = x; end_y = pre_y = y;


	do {
		read_neighbor8(y, x, neighbor8, src_img);
		start_o = (8 + cur_orient - 2) % 8;
		for (i = 0; i < 8; i++) {
			add_o = (start_o + i) % 8;
			if (neighbor8[add_o] != 0)
				break;
		}

		if (i < 8) {
			calCoord(add_o, &y, &x);
			cur_orient = add_o;
			if (x > 600 && y > 600) {
				printf("ff");
			}
		}
		else {
			printf("OMG");
		}

		
		num_region[label]++;
		(*label_img).data[cols * pre_y + pre_x] = label;
		(*output_img).data[cols * pre_y + pre_x] = 255;

		boundary.push_back(make_pair(pre_x, pre_y));

		pre_x = x; pre_y = y; pre_orient = cur_orient;
	} while ((y != end_y) || (x != end_x));
}

void read_neighbor8(int y, int x, int neighbor8[], Mat src_img) {
	int rows = src_img.rows;
	int cols = src_img.cols;

	neighbor8[0] = src_img.data[cols * y + x + 1];
	neighbor8[1] = src_img.data[cols * (y + 1) + x + 1];
	neighbor8[2] = src_img.data[cols * (y + 1) + x];
	neighbor8[3] = src_img.data[cols * (y + 1) + x - 1];
	neighbor8[4] = src_img.data[cols * y + x - 1];
	neighbor8[5] = src_img.data[cols * (y - 1) + x - 1];
	neighbor8[6] = src_img.data[cols * (y - 1) + x];
	neighbor8[7] = src_img.data[cols * (y - 1) + x + 1];
}

double getLength(int x1,  int y1, int x2, int y2, int rx, int ry) {
	
	
	if (x1 == x2) {
		return abs(x1 - rx);
	}

	double m = (double)(y2 - y1) / (x2 - x1);
	double c = y1 - (m * x1);	
	double bottom = sqrt((m * m) + 1);
	double top = abs((m * rx) + (-1 * ry) + c);
	double d = top / bottom;	
	return d;

}

void lcs(vector <pair<int, int>> &boundary, vector <double> &arr_lcs) {
	
	// 경계선 사이즈
	int length = boundary.size();

	// 임의의 윈도우 사이즈
	int window = length / 8;

	// 윈도우는 홀수여야 함.
	if (window % 2 == 0) {
		window++;
	}

	// 좌 우로 움직일 윈도우 절반 사이즈
	int half_window = (window - 1) / 2;	

	// 경계선 값을 돌면서
	for (int i = 0; i < length; i++) {
		int left = i - half_window;
		int right = i + half_window;

		// 값이 원형으로 이어질 수 있도록 인덱스 값 조정
		if (left < 0) {
			left = length + left;
		}
		if (right >= length) {
			right = right - length;
		}

		// left right를 잇는 직선과 바운더리 중앙값 사이의 거리 구하기
		double value = getLength(boundary[left].first, boundary[left].second, boundary[right].first, boundary[right].second, boundary[i].first, boundary[i].second);
		arr_lcs.push_back(value);
	}
	
}

pair<double, int> calMin(double a, double b, double c) {
	
	pair<double, int> tmp = make_pair(a, 1);
	
	if (b < tmp.first) {
		tmp.first = b;
		tmp.second = 2;
	}
	if (c < tmp.first) {
		tmp.first = c;
		tmp.second = 3;
	}
	return tmp;
}
double d_func(double a, double b) {
	return abs(a - b);
}

double Dtw(vector <double> &arr_lcs1, vector <double> &arr_lcs2) {

	// lcs1과 lcs2의 길이
	int arr1_length = arr_lcs1.size();
	int arr2_length = arr_lcs2.size();
	
	// D 배열 선언
	double **d;
	d = (double**)malloc(sizeof(double*) * arr1_length);
	for (int i = 0; i < arr1_length; i++) {
		d[i] = (double*)malloc(sizeof(double) * arr2_length);
	}

	// G 배열 선언
	int **g;
	g = (int**)malloc(sizeof(int*) * arr1_length);
	for (int i = 0; i < arr1_length; i++) {
		g[i] = (int*)malloc(sizeof(int) * arr2_length);
	}
	
	// 0, 0칸 초기화
	d[0][0] = d_func(arr_lcs1[0], arr_lcs2[0]);
	g[0][0] = 0;

	// 가로 첫번째줄 초기화
	for (int i = 1; i < arr2_length; i++) {
		d[0][i] = d[0][i - 1] + d_func(arr_lcs1[0], arr_lcs2[i]);
		g[0][i] = 2;
	}

	// 세로 첫번째줄 초기화
	for (int i = 1; i < arr1_length; i++) {
		d[i][0] = INF;
	}

	// FORWARD
	for (int i = 1; i < arr1_length; i++) {
		for (int j = 1; j < arr2_length; j++) {
			pair<double, int> tmp = calMin(d[i-1][j], d[i][j-1], d[i-1][j-1]);
			d[i][j] = d_func(arr_lcs1[i], arr_lcs2[j]) + tmp.first;
			g[i][j] = tmp.second;
		}
	}

	// 백트래킹
	int i = arr1_length - 1;
	int j = arr2_length - 1 ;
	int k = 1;

	while ((i != 0) && (j != 0)) {
		switch (g[i][j]) {
		case 1:
			i--; k++; break;
		case 2:
			j--; k++; break;
		case 3:
			i--; j--; k++; break;
		}
	}

	double dissimilarity = d[arr1_length - 1][arr2_length - 1] / k;
	
	for (int i = 0; i < arr1_length; i++) {
		free(d[i]);
	}
	free(d);

	for (int i = 0; i < arr1_length; i++) {
		free(g[i]);
	}
	free(g);

	return dissimilarity;

}

void Sdtw(vector <double> &arr_lcs1, vector <double> &arr_lcs2, vector <double> &arr_lcs3) {

}


