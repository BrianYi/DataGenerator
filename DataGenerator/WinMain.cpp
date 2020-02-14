#include "Header.h"
#include "resource.h"
#include "Mutex.h"
#include <vector>
#include "FeatureTable.h"
#include "ClssTable.h"

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#define MYDEBUG

#define WM_UPDATESCROLLINFO (WM_USER+1)
#define WM_RECEIVELOG		(WM_USER+2)


TCHAR    g_szAppName[16], g_szCaption[64];
HINSTANCE g_hInst;
HWND g_hTabControl;
HWND g_hTabWin, g_hGrpBoxFeat, g_hWinFeat, g_hGrpBoxClss, g_hWinClss, g_hGrpBoxParam, g_hWinParam, g_hGrpBoxDecTree, g_hWinDecTree, g_hLogWin;
HWND g_hBtnAddFeat, g_hBtnBuildDecTree, g_hBtnGenDataSet;
//HANDLE g_hLogThread;
//BOOL g_bStopLogThread;
WNDPROC oldTabProc, oldFeatGrpBoxProc, oldClssGrpBoxProc,oldDecTreeGrpBoxProc;
FeatureTable ft;
ClssTable ct;

void logWrite(const TCHAR *format, ...)
{
	TCHAR buffer[4096];
	va_list args;
	va_start(args, format);
	_vstprintf(buffer, sizeof(buffer), format, args);
	SendMessage(g_hLogWin, WM_RECEIVELOG, (WPARAM)buffer, 0);
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
int InsertTabItem(HWND hTab, LPTSTR pszText, int iid);
BOOL CALLBACK EnumChildWinProc(HWND hwnd, LPARAM lParam);
LRESULT CALLBACK TabProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK FeatureGrpBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK FeatureWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ClssGrpBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ClssWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DecTreeGrpBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DecTreeWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LogWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SelFeatDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
{
	HWND     hwnd;
	MSG      msg;
	WNDCLASS wndclass;
	LoadString(hInstance, IDS_APPNAME, g_szAppName, sizeof g_szAppName / sizeof g_szAppName[0]);
	LoadString(hInstance, IDS_CAPTION, g_szCaption, sizeof g_szCaption / sizeof g_szCaption[0]);

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DG));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
	wndclass.lpszClassName = g_szAppName;
	g_hInst = hInstance;

	//g_hLogThread = CreateThread(0, 0, LogThreadProc, (LPVOID)g_hLogWin, NULL, NULL);

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"),
			g_szAppName, MB_ICONERROR);
		return 0;
	}

	wndclass.lpfnWndProc = FeatureWinProc;
	wndclass.lpszClassName = TEXT("FeatureWindow");
	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"),
			g_szAppName, MB_ICONERROR);
		return 0;
	}

	wndclass.lpfnWndProc = ClssWinProc;
	wndclass.lpszClassName = TEXT("ClssWindow");
	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"),
			g_szAppName, MB_ICONERROR);
		return 0;
	}

	wndclass.lpfnWndProc = DecTreeWinProc;
	wndclass.lpszClassName = TEXT("DecTreeWindow");
	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"),
			g_szAppName, MB_ICONERROR);
		return 0;
	}

	wndclass.lpfnWndProc = LogWinProc;
	wndclass.lpszClassName = TEXT("LogWindow");
	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"),
			g_szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(g_szAppName, g_szCaption,
		WS_OVERLAPPEDWINDOW,
		100, 100, 775, 830,
		NULL, NULL, hInstance, NULL);
	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

// DWORD WINAPI LogThreadProc(LPVOID lpParameter)
// {
// 	TCHAR *pszLog = NULL;
// 	while (!g_bStopLogThread)
// 	{
// 		g_logMtx.Lock();
// 		while (!g_logList.empty() && g_hLogWin)
// 		{
// 			pszLog = g_logQueue.front();
// 			SendMessage(g_hLogWin, WM_RECEIVELOG, (WPARAM)pszLog, 0);
// 			g_logQueue.pop();
// 		}
// 		g_logMtx.Unlock();
// 		Sleep(100);
// 	}
// 	return 0;
// }

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	HMENU hMenu;
	TCHAR szBuffer[256];
	RECT rc;
	int i,j;
	switch (message)
	{
	case WM_CREATE:
	{
		INITCOMMONCONTROLSEX icex;

		// Initialize common controls.
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_TAB_CLASSES;
		InitCommonControlsEx(&icex);

		// Tab Control

		g_hTabControl = CreateWindowEx(0, WC_TABCONTROL, NULL,
			TCS_FIXEDWIDTH | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
			0, 0, 0, 0,
			hwnd, NULL, g_hInst, NULL);
		wsprintf(szBuffer, TEXT("新数据集1*"));
		InsertTabItem(g_hTabControl, szBuffer, 0);
		oldTabProc = (WNDPROC)SetWindowLongPtr(g_hTabControl, GWLP_WNDPROC, (LONG)TabProc);


		EnumChildWindows(hwnd, EnumChildWinProc, 0);
		break;
	}
	case WM_SIZE:
	{
		// Tab Control
		GetClientRect(hwnd, &rc);
		TabCtrl_AdjustRect(hwnd, TRUE, &rc);
		MoveWindow(g_hTabControl, rc.left, rc.top, rc.right + 2, rc.bottom + 2, TRUE);


#ifdef MYDEBUG
		logWrite(TEXT("开始初始化主窗口..."));
		logWrite(TEXT("主窗口创建完毕."));
		logWrite(TEXT("开始创建Tab Control"));
		logWrite(TEXT("开始创建 特征GroupBox"));
		logWrite(TEXT("开始创建 特征Window"));
		logWrite(TEXT("开始创建 类别GroupBox"));
		logWrite(TEXT("开始创建 类别Window"));
		logWrite(TEXT("开始创建 参数GroupBox"));
		logWrite(TEXT("开始创建 决策树GroupBox"));
		logWrite(TEXT("开始创建 决策树Window"));
		logWrite(TEXT("开始创建 日志Edit Control"));
		logWrite(TEXT("开始创建 按钮们"));
		logWrite(TEXT("枚举所有子窗口 设置字体DEFAULT_GUI_FONT"));
#endif
		return 0;
	}
	case WM_NOTIFY:
	{
		int iCurTab = 0;
		if (((LPNMHDR)lParam)->code == TCN_SELCHANGE)
		{
			iCurTab = TabCtrl_GetCurSel(g_hTabControl);
			// TODO:
		}
		return 0;
	}
	case WM_COMMAND:
		hMenu = GetMenu(hwnd);
		switch (LOWORD(wParam))
		{
		case IDM_FILE_OPEN:
			break;
		case IDM_FILE_SAVE:
			break;
		case IDM_APP_EXIT:
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		case IDM_APP_ABOUT:
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUT), hwnd, About);
			break;
		default:
			break;
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}


LRESULT CALLBACK TabProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i, j;
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rc;
	switch (message)
	{
	case WM_SIZE:
	{
		if (!g_hLogWin)
			g_hLogWin = CreateWindow(TEXT("LogWindow"), TEXT("日志记录..."),
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
				0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);
		if (!g_hGrpBoxFeat)
		{
			g_hGrpBoxFeat = CreateWindowEx(0, TEXT("button"), TEXT("特征"),
				WS_CHILD | WS_VISIBLE | BS_GROUPBOX | WS_CLIPCHILDREN,
				0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);
			oldFeatGrpBoxProc = (WNDPROC)SetWindowLongPtr(g_hGrpBoxFeat, GWLP_WNDPROC, (LONG)FeatureGrpBoxProc);
		}
		if (!g_hGrpBoxClss)
		{
			g_hGrpBoxClss = CreateWindowEx(0, TEXT("button"), TEXT("类别"),
				WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);
			oldClssGrpBoxProc = (WNDPROC)SetWindowLongPtr(g_hGrpBoxClss, GWLP_WNDPROC, (LONG)ClssGrpBoxProc);
		}
		if (!g_hGrpBoxParam)
			g_hGrpBoxParam = CreateWindowEx(0, TEXT("button"), TEXT("参数"),
				WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);
		if (!g_hGrpBoxDecTree)
		{
			g_hGrpBoxDecTree = CreateWindowEx(0, TEXT("button"), TEXT("决策树"),
				WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);
			oldDecTreeGrpBoxProc = (WNDPROC)SetWindowLongPtr(g_hGrpBoxDecTree, GWLP_WNDPROC, (LONG)DecTreeGrpBoxProc);
		}
		if (!g_hBtnGenDataSet)
			g_hBtnGenDataSet = CreateWindowEx(0, TEXT("button"), TEXT("生成数据集"),
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);
		MoveWindow(g_hLogWin, 9, 605, 743, 125, TRUE);
		MoveWindow(g_hGrpBoxFeat, 9, 31, 370, 264, TRUE);
		MoveWindow(g_hGrpBoxClss, 389, 31, 363, 90, TRUE);
		MoveWindow(g_hGrpBoxParam, 389, 143, 363, 152, TRUE);
		MoveWindow(g_hGrpBoxDecTree, 9, 295, 743, 300, TRUE);
		MoveWindow(g_hBtnGenDataSet, 652, 735, 100, 30, TRUE);

		return 0;
	}
	case WM_COMMAND:
		if ((HWND)lParam == g_hBtnGenDataSet && HIWORD(wParam) == BN_CLICKED)
		{
			// 生成数据集
		}
		break;
	default:
		break;
	}
	return CallWindowProc(oldTabProc,hwnd, message, wParam, lParam);
}

LRESULT CALLBACK FeatureGrpBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int cxClient, cyClient;
	HDC hdc;
	PAINTSTRUCT ps;
	TCHAR szBuffer[256];
	switch (message)
	{
	case WM_SIZE:
	{
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		
		if (!g_hBtnAddFeat)
			g_hBtnAddFeat = CreateWindowEx(0, TEXT("button"), TEXT("添加特征"),
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);
		if (!g_hWinFeat)
			g_hWinFeat = CreateWindowEx(0, TEXT("FeatureWindow"), NULL,
				WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL | WS_CLIPCHILDREN,
				0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);
		
		MoveWindow(g_hBtnAddFeat, cxClient - 110, cyClient - 40, 100, 30, TRUE);
		MoveWindow(g_hWinFeat, 10, 20, cxClient - 20, cyClient - 70, TRUE);
		break;
	}
	case WM_KEYDOWN:
	{
		logWrite(TEXT("Press %x", wParam));
		break;
	}
	case WM_COMMAND:
	{
		if ((HWND)lParam == g_hBtnAddFeat && HIWORD(wParam) == BN_CLICKED)
		{
			ft.AddFeat(new Feat(g_hWinFeat));
			SendMessage(g_hWinFeat, WM_UPDATESCROLLINFO, 0, 0);
			InvalidateRect(g_hWinFeat, NULL, TRUE);
		}
		break;
	}
	default:
		break;
	}
	return CallWindowProc(oldFeatGrpBoxProc,hwnd, message, wParam, lParam);
}


LRESULT CALLBACK FeatureWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i, j;
	HDC hdc;
	PAINTSTRUCT ps;
	TEXTMETRIC tm;
	RECT rc;
	static int cxClient, cyClient, cxUnit, cyUnit, iHorzPos, iVerPos;
	static int iAccumDelta, iDeltaPerLine;
	int ulScrollLines;
	SCROLLINFO si;
	TCHAR szBuffer[256];
	switch (message)
	{
	case WM_CREATE:
	{
		cxUnit = 10;
		cyUnit = 10;

		SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ulScrollLines, 0);
		iDeltaPerLine = ulScrollLines ? WHEEL_DELTA / ulScrollLines : 0;

		// features
		ft.AddFeat(new Feat(hwnd));
		ft.BackFeat()->SetFocus();
		break;
	}
	case WM_SIZE:
	{
		cxClient = LOWORD(lParam); // 348
		cyClient = HIWORD(lParam); // 192

		// 滚动条初始化
		si.cbSize = sizeof si;
	
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = 1;
		si.nPage = 1;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = 1;
		si.nPage = 1;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
		break;
	}
	case WM_KEYDOWN:
	{
		logWrite(TEXT("Press %x", wParam));
		switch (wParam)
		{
		case VK_TAB:
			logWrite(TEXT("Press TAB"));
			break;
		default:
			break;
		}

		break;
	}
	case WM_UPDATESCROLLINFO:
	{
		iHorzPos = GetScrollPos(hwnd, SB_HORZ);
		iVerPos = GetScrollPos(hwnd, SB_VERT);
		rc=ft.MoveWindow(0, 0, TRUE);
		ScrollWindow(hwnd, iHorzPos, iVerPos, NULL, NULL);

		si.cbSize = sizeof si;
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		if (rc.bottom < cyClient)
			si.nMax = si.nPage = rc.bottom;
		else
		{
			si.nMax = rc.bottom;
			si.nPage = cyClient;
		}
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		if (iDeltaPerLine == 0)
			break;
		iAccumDelta += (short)HIWORD(wParam);
		while (iAccumDelta >= iDeltaPerLine)
		{
			SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0);
			iAccumDelta -= iDeltaPerLine;
		}
		while (iAccumDelta <= -iDeltaPerLine)
		{
			SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
			iAccumDelta += iDeltaPerLine;
		}
		break;
	}
	case WM_VSCROLL:
	{
		si.cbSize = sizeof si;
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_VERT, &si);
		iVerPos = si.nPos;
		switch (LOWORD(wParam))
		{
		case SB_TOP:
			si.nPos = si.nMin;
			break;
		case SB_BOTTOM:
			si.nPos = si.nMax;
			break;
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;
		case SB_LINEUP:
			si.nPos -= 10;
			break;
		case SB_LINEDOWN:
			si.nPos += 10;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;
		default:
			break;
		}
		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hwnd, SB_VERT, &si);
		if (iVerPos != si.nPos)
		{
			ScrollWindow(hwnd, 0, iVerPos - si.nPos, NULL, NULL);
			UpdateWindow(hwnd);
		}
		break;
	}
	case WM_HSCROLL:
	{
		si.cbSize = sizeof si;
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_HORZ, &si);
		iHorzPos = si.nPos;
		switch (LOWORD(wParam))
		{
		case SB_LEFT:
			si.nPos = si.nMin;
			break;
		case SB_RIGHT:
			si.nPos = si.nMax;
			break;
		case SB_PAGELEFT:
			si.nPos -= si.nPage;
			break;
		case SB_PAGERIGHT:
			si.nPos += si.nPage;
			break;
		case SB_LINELEFT:
			si.nPos -= 10;
			break;
		case SB_LINERIGHT:
			si.nPos += 10;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;
		default:
			break;
		}
		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
		GetScrollInfo(hwnd, SB_HORZ, &si);
		if (iHorzPos != si.nPos)
		{
			ScrollWindow(hwnd, iHorzPos - si.nPos, 0, NULL, NULL);
			UpdateWindow(hwnd);
		}
		break;
	}
	case WM_COMMAND:
	{
		GetWindowText((HWND)lParam, szBuffer, sizeof szBuffer);
		if (szBuffer[0] == TEXT('+'))
		{
			Feat *f = (Feat *)GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
			f->AddFeatVal(new FeatVal(hwnd));
			f->BackFeatVal()->SetFocus();
			iHorzPos=GetScrollPos(hwnd, SB_HORZ);
			iVerPos=GetScrollPos(hwnd, SB_VERT);
			rc = ft.MoveWindow(0, 0, TRUE);
			ScrollWindow(hwnd, -iHorzPos, -iVerPos, NULL, NULL);
		}
		else if (szBuffer[0] == TEXT('-'))
		{
			Feat *f = (Feat *)GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
			f->DelFeatVal(f->CountVal() - 1);
			iHorzPos = GetScrollPos(hwnd, SB_HORZ);
			iVerPos = GetScrollPos(hwnd, SB_VERT);
			rc = ft.MoveWindow(0, 0, TRUE);
			ScrollWindow(hwnd, -iHorzPos, -iVerPos, NULL, NULL);
		}

		if (szBuffer[0] == TEXT('+') ||
			szBuffer[0] == TEXT('-'))
		{
			si.cbSize = sizeof si;
			si.fMask = SIF_RANGE | SIF_PAGE;
			si.nMin = 0;
			if (rc.right < cxClient)
				si.nMax = si.nPage = rc.right;
			else
			{
				si.nMax = rc.right;
				si.nPage = cxClient;
			}
			SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
			InvalidateRect(hwnd, NULL, TRUE);
		}
		break;
	}
	case WM_PAINT:
	{
		si.cbSize = sizeof si;
		si.fMask = SIF_POS;
		GetScrollInfo(hwnd, SB_HORZ, &si);
		iHorzPos = si.nPos;
		GetScrollInfo(hwnd, SB_VERT, &si);
		iVerPos = si.nPos;
		hdc = BeginPaint(hwnd, &ps);

		
		ft.MoveWindow(-iHorzPos, -iVerPos,TRUE);

		EndPaint(hwnd, &ps);
		break;
	}
	default:
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

// VOID SetFeatureScrollInfo(HWND hwnd)
// {
// 	RECT rc/*, rcClient*/;
// 	SCROLLINFO si;
// 	/*rc = DrawFeatureWin(0, 0, TRUE);*/
// 
// 	si.cbSize = sizeof si;
// 
// 	si.fMask = SIF_RANGE | SIF_PAGE;
// 	si.nMin = rc.top;
// 	si.nPage = 60 * 3;
// 	si.nMax = max(rc.bottom, si.nPage);
// 	SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
// 
// 	si.fMask = SIF_RANGE | SIF_PAGE;
// 	si.nMin = rc.left;
// 	si.nPage = 80 * 4;
// 	si.nMax = max(rc.right, si.nPage);
// 	SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
// }

LRESULT CALLBACK ClssGrpBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int cxClient, cyClient;
	HDC hdc;
	PAINTSTRUCT ps;
	TCHAR szBuffer[256];
	switch (message)
	{
	case WM_SIZE:
	{
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);

		if (!g_hWinClss)
			g_hWinClss = CreateWindowEx(0, TEXT("ClssWindow"), NULL,
			WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN,
				0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);
		
		MoveWindow(g_hWinClss, 10, 20, cxClient - 20, cyClient - 30, TRUE);

		break;
	}
	default:
		break;
	}
	return CallWindowProc(oldClssGrpBoxProc,hwnd, message, wParam, lParam);
}

LRESULT CALLBACK ClssWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i, j;
	HDC hdc;
	PAINTSTRUCT ps;
	TEXTMETRIC tm;
	static RECT rc;
	static int cxClient, cyClient, cxChar, cyChar, cxMax, cyMax, iCurX, iCurY;
	static int iAccumDelta, iDeltaPerLine;
	int ulScrollLines;
	SCROLLINFO si;
	TCHAR szBuffer[256];
	switch (message)
	{
	case WM_CREATE:
	{
		hdc = GetDC(hwnd);
		GetTextMetrics(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		ReleaseDC(hwnd, hdc);

		// class
		ct.AddClss(new Clss(hwnd));
		break;
	}
	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);
		ct.MoveWindow(0, 0, TRUE);
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_COMMAND:
	{
		break;
	}
	default:
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}


LRESULT CALLBACK DecTreeGrpBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int cxClient, cyClient;
	switch (message)
	{
	case WM_SIZE:
	{
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);

		if (!g_hWinDecTree)
			g_hWinDecTree = CreateWindowEx(0, TEXT("DecTreeWindow"), NULL,
				WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL,
				0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);

		if (!g_hBtnBuildDecTree)
			g_hBtnBuildDecTree = CreateWindowEx(0, TEXT("button"), TEXT("构造决策树"),
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);
		
		MoveWindow(g_hWinDecTree, 10, 20, cxClient - 20, cyClient - 70, TRUE);
		MoveWindow(g_hBtnBuildDecTree, cxClient - 110, cyClient - 40, 100, 30, TRUE);
		break;
	}
	case WM_COMMAND:
	{
		if ((HWND)lParam == g_hBtnBuildDecTree && HIWORD(wParam) == BN_CLICKED)
		{
			// 检查
			if (ft.AnyEmpty()||ct.AnyEmpty()) 
				return 0;
			
			// 清空树
			
			// 构造树

			// 构造决策树
			INT_PTR ret = DialogBoxParam(g_hInst, 
				MAKEINTRESOURCE(IDD_DLG_SELFEAT), NULL, SelFeatDlgProc, 0);

			//
			InvalidateRect(g_hWinDecTree, NULL, TRUE);
			return 0;
		}
		break;
	}
	default:
		break;
	}
	return CallWindowProc(oldDecTreeGrpBoxProc, hwnd, message, wParam, lParam);
}



LRESULT CALLBACK DecTreeWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i, j;
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rc;
	switch (message)
	{
	case WM_CREATE:
	{

		break;
	}
	case WM_SIZE:
	{
		break;
	}
	case WM_HSCROLL:
	{
		break;
	}
	case WM_VSCROLL:
	{
		break;
	}
	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_COMMAND:
	{
		break;
	}
	default:
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK LogWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i, j;
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rc;
	static int cxChar, cyChar, cxClient, cyClient, iHorzPos, iVerPos, iCurLine, iMaxLines, iMaxCols;
	static int iHorzPosMax;
	TCHAR szBuffer[1024];
	SCROLLINFO si;
	TEXTMETRIC tm;
	static std::vector<TCHAR*> logArry;
	static int iAccumDelta, iDeltaPerLine;
	int ulScrollLines;
	switch (message)
	{
	case WM_CREATE:
	{
		SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ulScrollLines, 0);
		iDeltaPerLine = ulScrollLines ? WHEEL_DELTA / ulScrollLines : 0;

		hdc = GetDC(hwnd);
		GetTextMetrics(hdc, &tm);
		cxChar = 14;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		ReleaseDC(hwnd, hdc);

		return 0;
	}
	case WM_SIZE:
	{
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		iMaxLines = cyClient / cyChar;
		iMaxCols = cxClient / cxChar;
		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		if (iDeltaPerLine == 0)
			break;
		iAccumDelta += (short)HIWORD(wParam);	// 向下滚动则wParam为负,向上则为正(即滚轮向鼠标线方向滚动为正)
		while (iAccumDelta >= iDeltaPerLine)
		{
			SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0);
			iAccumDelta -= iDeltaPerLine;
		}
		while (iAccumDelta <= -iDeltaPerLine)
		{
			SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
			iAccumDelta += iDeltaPerLine;
		}
		break;
	}
	case WM_RECEIVELOG:
	{
 		DWORD iSize = _tcslen((TCHAR*)wParam);
		TCHAR *pszBuffer = (TCHAR *)malloc((iSize + 1) * sizeof TCHAR);
		CopyMemory(pszBuffer, (TCHAR*)wParam, iSize * sizeof TCHAR);
		pszBuffer[iSize] = '\0';
		logArry.push_back(pszBuffer);
		++iCurLine;
		iHorzPosMax = max(iHorzPosMax, iSize - 1);

		// 设置scrollbar
		si.cbSize = sizeof si;

		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = max(iMaxLines, iCurLine) - 1;
		si.nPage = min(iMaxLines,max(iMaxLines, iCurLine));
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

		if (iCurLine >= iMaxLines)
		{
			si.fMask = SIF_POS;
			si.nPos = si.nMax - si.nPage + 1;
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
			ScrollWindow(hwnd, 0, -cyChar, NULL, NULL);
			InvalidateRect(hwnd, NULL, TRUE);
			UpdateWindow(hwnd);
		}

		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = iHorzPosMax;
		si.nPage = min(iMaxCols, iHorzPosMax + 1);
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
		UpdateWindow(hwnd);

		return 0;
	}
	case WM_HSCROLL:
	{
		si.cbSize = sizeof si;
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_HORZ, &si);
		iHorzPos = si.nPos;
		switch (LOWORD(wParam))
		{
		case SB_LEFT:
			si.nPos = 0;
			break;
		case SB_RIGHT:
			si.nPos = si.nMax;
			break;
		case SB_PAGELEFT:
			si.nPos -= si.nPage;
			break;
		case SB_PAGERIGHT:
			si.nPos += si.nPage;
			break;
		case SB_LINELEFT:
			si.nPos -= 1;
			break;
		case SB_LINERIGHT:
			si.nPos += 1;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;
		default:
			break;
		}
		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
		GetScrollInfo(hwnd, SB_HORZ, &si);
		if (iHorzPos != si.nPos)
		{
			ScrollWindow(hwnd, (iHorzPos - si.nPos)*cxChar, 0, NULL, NULL);
			//InvalidateRect(hwnd, NULL, TRUE);
			//UpdateWindow(hwnd);
		}
		break;
	}
	case WM_VSCROLL:
	{
		si.cbSize = sizeof si;
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_VERT, &si);
		iVerPos = si.nPos;
		switch (LOWORD(wParam))
		{
		case SB_TOP:
			si.nPos = si.nMin;
			break;
		case SB_BOTTOM:
			si.nPos = si.nMax;
			break;
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;
		case SB_LINEUP:
			si.nPos -= 1;
			break;
		case SB_LINEDOWN:
			si.nPos += 1;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;
		default:
			break;
		}
		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hwnd, SB_VERT, &si);
		if (iVerPos != si.nPos)
		{
			ScrollWindow(hwnd, 0, (iVerPos - si.nPos) * cyChar, NULL, NULL);
			UpdateWindow(hwnd);
		}
		break;
	}
	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);
		SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));	// 只有SYSTEM_FIXED_FONT
		
		si.cbSize = sizeof si;
		si.fMask = SIF_POS;
		GetScrollInfo(hwnd, SB_VERT, &si);
		iVerPos = si.nPos;
		GetScrollInfo(hwnd, SB_HORZ, &si);
		iHorzPos = si.nPos;
		int iPaintBeg = max(0, iVerPos + ps.rcPaint.top / cyChar);
		int iPaintEnd = min((int)logArry.size() - 1, iVerPos + ps.rcPaint.bottom / cyChar);
		for (i = iPaintBeg; i <= iPaintEnd; ++i)
		{
			int x = cxChar * (1 - iHorzPos);
			int y = cyChar * (i - iVerPos);
			TextOut(hdc, x, y, logArry[i], _tcslen(logArry[i]));
		}
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_COMMAND:
	{
		break;
	}
	case WM_DESTROY:
	{
		for (auto it : logArry)
			free(it);
		break;
	}
	default:
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

INT_PTR CALLBACK SelFeatDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i,j;
	TCHAR szBuffer[256];
	HWND hComboFeat = GetDlgItem(hwnd, IDC_COMBO_FEAT);
	HWND hComboClss = GetDlgItem(hwnd, IDC_COMBO_CLSS);
	switch (message)
	{
	case WM_INITDIALOG:
	{

		// fill features
		for (i=0;i<ft.CountFeature();++i)
		{
			Feat * f = ft.GetFeat(i);
			if (f->Flag() == FALSE)
			{
				f->GetNamEdtText(szBuffer, sizeof szBuffer);
				if (szBuffer[0] != '\0')
					ComboBox_AddString(hComboFeat, szBuffer);
			}
		}
		ComboBox_SetCurSel(hComboFeat, 0);

		// fill class
		for (i = 0; i < ct.CountClss(); ++i)
		{
			Clss *c = ct.GetClss(i);
			for (j = 0; j < c->CountVal(); ++j)
			{
				ClssVal *cv = c->GetClssVal(j);
				cv->GetText(szBuffer, sizeof szBuffer);
				ComboBox_AddString(hComboClss, szBuffer);
			}
		}
		ComboBox_SetCurSel(hComboClss, 0);
	}
	case WM_SETFOCUS:
		SetFocus(hComboFeat);
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDFEAT:
			i = ComboBox_GetCurSel(hComboFeat);
			EndDialog(hwnd, i);
			break;
		case IDCLSS:
			i = ComboBox_GetCurSel(hComboClss);
			EndDialog(hwnd, i);
			break;
		case IDCANCEL:
			EndDialog(hwnd, -1);
			break;
		default:
			break;
		}
		return FALSE;
	default:
		break;
	}
	return FALSE;
}


BOOL CALLBACK EnumChildWinProc(HWND hwnd, LPARAM lParam)
{
	SendMessage(hwnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), NULL);

	return TRUE;
}


int InsertTabItem(HWND hTab, LPTSTR pszText, int iid)
{
	TCITEM ti = { 0 };
	ti.mask = TCIF_TEXT;
	ti.pszText = pszText;
	ti.cchTextMax = wcslen(pszText);

	return (int)SendMessage(hTab, TCM_INSERTITEM, iid, (LPARAM)&ti);
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
