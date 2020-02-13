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
HWND DoCreateTabControl(HWND hwndParent);
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
		// Tab Control
		g_hTabControl = DoCreateTabControl(hwnd);
		wsprintf(szBuffer, TEXT("新数据集1*"));
		InsertTabItem(g_hTabControl, szBuffer, 0);
		oldTabProc = (WNDPROC)SetWindowLongPtr(g_hTabControl, GWLP_WNDPROC, (LONG)TabProc);


		// 特征GroupBox
		g_hGrpBoxFeat = CreateWindowEx(0, TEXT("button"), TEXT("特征"),
			WS_CHILD | WS_VISIBLE | BS_GROUPBOX | WS_CLIPCHILDREN,
			0, 0, 0, 0, g_hTabControl, NULL, g_hInst, NULL);
		oldFeatGrpBoxProc = (WNDPROC)SetWindowLongPtr(g_hGrpBoxFeat, GWLP_WNDPROC, (LONG)FeatureGrpBoxProc);


		g_hWinFeat = CreateWindowEx(0, TEXT("FeatureWindow"), NULL,
			WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL | WS_CLIPCHILDREN,
			0, 0, 0, 0, g_hGrpBoxFeat, NULL, g_hInst, NULL);


		// 类别GroupBox
		g_hGrpBoxClss = CreateWindowEx(0, TEXT("button"), TEXT("类别"),
			WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			0, 0, 0, 0, g_hTabControl, NULL, g_hInst, NULL);
		oldClssGrpBoxProc = (WNDPROC)SetWindowLongPtr(g_hGrpBoxClss, GWLP_WNDPROC, (LONG)ClssGrpBoxProc);


		g_hWinClss = CreateWindowEx(0, TEXT("ClssWindow"), NULL,
			WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN,
			0, 0, 0, 0, g_hGrpBoxClss, NULL, g_hInst, NULL);
		

		// 参数GroupBox
		g_hGrpBoxParam = CreateWindowEx(0, TEXT("button"), TEXT("参数"),
			WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			0, 0, 0, 0, g_hTabControl, NULL, g_hInst, NULL);


		// 决策树GroupBox
		g_hGrpBoxDecTree = CreateWindowEx(0, TEXT("button"), TEXT("决策树"),
			WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			0, 0, 0, 0, g_hTabControl, NULL, g_hInst, NULL);
		oldDecTreeGrpBoxProc = (WNDPROC)SetWindowLongPtr(g_hGrpBoxDecTree, GWLP_WNDPROC, (LONG)DecTreeGrpBoxProc);


		g_hWinDecTree = CreateWindowEx(0, TEXT("DecTreeWindow"), NULL,
			WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL,
			0, 0, 0, 0, g_hGrpBoxDecTree, NULL, g_hInst, NULL);
		//oldDecTreeWinProc = (WNDPROC)SetWindowLongPtr(g_hWinDecTree, GWLP_WNDPROC, (LONG)DecTreeWinProc);


		// -----------------------
		// 日志edit
		g_hLogWin = CreateWindow(TEXT("LogWindow"), TEXT("日志记录..."),
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
			9, 605, 743, 125, g_hTabControl, NULL, g_hInst, NULL);

#ifdef MYDEBUG
		logWrite(TEXT("开始初始化主窗口..."));
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
#endif
		//------------------------
		//按钮们
		g_hBtnAddFeat = CreateWindowEx(0, TEXT("button"), TEXT("添加特征"),
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			0, 0, 0, 0, g_hGrpBoxFeat, NULL, g_hInst, NULL);

		g_hBtnBuildDecTree = CreateWindowEx(0, TEXT("button"), TEXT("构造决策树"),
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			0, 0, 0, 0, g_hGrpBoxDecTree, NULL, g_hInst, NULL);

		g_hBtnGenDataSet = CreateWindowEx(0, TEXT("button"), TEXT("生成数据集"),
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			0, 0, 0, 0, g_hTabControl, NULL, g_hInst, NULL);

#ifdef MYDEBUG
		logWrite(TEXT("枚举所有子窗口 设置字体DEFAULT_GUI_FONT"));
#endif
		EnumChildWindows(hwnd, EnumChildWinProc, 0);

#ifdef MYDEBUG
		logWrite(TEXT("主窗口创建完毕."));
#endif
		break;
	}
	case WM_SIZE:
	{
		// Tab Control
		GetClientRect(hwnd, &rc);
		TabCtrl_AdjustRect(hwnd, TRUE, &rc);
		MoveWindow(g_hTabControl, rc.left, rc.top, rc.right + 2, rc.bottom + 2, TRUE);

		MoveWindow(g_hGrpBoxFeat, 9, 31, 370, 264, TRUE);

		MoveWindow(g_hGrpBoxClss, 389, 31, 363, 90, TRUE);

		MoveWindow(g_hGrpBoxParam, 389, 143, 363, 152, TRUE);

		MoveWindow(g_hGrpBoxDecTree, 9, 295, 743, 300, TRUE);

		MoveWindow(g_hLogWin, 9, 605, 743, 125, TRUE);

		MoveWindow(g_hBtnGenDataSet, 652, 735, 100, 30, TRUE);

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
		break;
	case WM_COMMAND:
		if ((HWND)lParam == g_hBtnGenDataSet && HIWORD(wParam) == BN_CLICKED)
		{
			// 生成数据集
		}
		break;
	default:
		break;
	}
	return oldTabProc(hwnd, message, wParam, lParam);
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
		MoveWindow(g_hBtnAddFeat, cxClient - 110, cyClient - 40, 100, 30, TRUE);
		MoveWindow(g_hWinFeat, 10, 20, cxClient - 20, cyClient - 70, TRUE);
		break;
	}
	case WM_COMMAND:
	{
		if ((HWND)lParam == g_hBtnAddFeat && HIWORD(wParam) == BN_CLICKED)
		{

// 			Feat *f = new Feat(hwnd);
// 			f->SetNamLblText(TEXT("特征名%i"), ft.CountFeature());
// 			f->SetValLblText(TEXT("特征值"));
// 			ft.AddFeat(f);


// 			if (!AddOneFeature(g_feat, g_hWinFeat))
// 			{
// 				wsprintf(szBuffer, TEXT("抱歉,当前最多添加%i个特征,如需更多请开通vip服务"), MAX_FEAT_NUM);
// 				MessageBox(NULL, szBuffer,
// 					g_szCaption, MB_ICONWARNING);
// 			}
			SendMessage(g_hWinFeat, WM_UPDATESCROLLINFO, 0, 0);
			InvalidateRect(g_hWinFeat, NULL, TRUE);
		}
		break;
	}
	default:
		break;
	}
	return oldFeatGrpBoxProc(hwnd, message, wParam, lParam);
}


LRESULT CALLBACK FeatureWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i, j;
	HDC hdc;
	PAINTSTRUCT ps;
	TEXTMETRIC tm;
	static RECT rc;
	static int cxClient, cyClient, cxChar, cyChar, iHorzPos, iVerPos;
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

		//SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ulScrollLines, 0);
		//iDeltaPerLine = WHEEL_DELTA / ulScrollLines : 0;
		iDeltaPerLine = WHEEL_DELTA;

		// features
		//AddOneFeature(g_feat, hwnd);
		
		
// 		Feat *f = new Feat(hwnd);
// 		f->SetNamLblText(TEXT("特征名%i"), ft.CountFeature());
// 		f->SetValLblText(TEXT("特征值"));
// 		ft.AddFeat(f);
		
		break;
	}
	case WM_SIZE:
	{
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);

		si.cbSize = sizeof si;

		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nPage = 0;
		si.nMax = 0;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nPage = 0;
		si.nMax = 0;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
		break;
	}
	case WM_UPDATESCROLLINFO:
	{
		//SetFeatureScrollInfo(hwnd);
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
			si.nPos -= 60;
			break;
		case SB_LINEDOWN:
			si.nPos += 60;
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
			iVerPos = si.nPos;
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
			si.nPos -= 80;
			break;
		case SB_LINERIGHT:
			si.nPos += 80;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			si.nPos = ceil(si.nTrackPos/80.0)*80;
			break;
		default:
			break;
		}
		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
		GetScrollInfo(hwnd, SB_HORZ, &si);
		if (iHorzPos != si.nPos)
		{
			ScrollWindow(hwnd, 0, iHorzPos - si.nPos, NULL, NULL);
			iHorzPos = si.nPos;
			UpdateWindow(hwnd);
		}
		break;
	}
	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);

		rc=ft.MoveWindow(22-iHorzPos, 5-iVerPos,TRUE);

		EndPaint(hwnd, &ps);
		break;
	}
	case WM_COMMAND:
	{
		GetWindowText((HWND)lParam, szBuffer, sizeof szBuffer);
		if (szBuffer[0] == TEXT('+'))
		{
			Feat *f = (Feat *)GetWindowLongPtr((HWND)lParam, 0);
			f->AddFeatVal(new FeatVal(hwnd));
			ScrollWindow(hwnd, -60, 0, NULL, NULL);
		}
		else if (szBuffer[0] == TEXT('-'))
		{
			Feat *f = (Feat *)GetWindowLongPtr((HWND)lParam, 0);
			f->DelFeatVal(f->CountVal() - 1);
			ScrollWindow(hwnd, 60, 0, NULL, NULL);
		}
// 		for (i = 0; i < g_feat.iFeatNamNum; ++i)
// 		{
// 			if (g_feat.name[i].hFeatValBtnPlus == (HWND)lParam)
// 			{
// 				logWrite(TEXT("添加按钮"));
// 				if (g_feat.name[i].iFeatValNum >= MAX_FEAT_VAL_NUM)
// 				{
// 					wsprintf(szBuffer, TEXT("抱歉,每个特征最多添加%i个特征值,如需更多请开通vip服务"), MAX_FEAT_VAL_NUM);
// 					MessageBox(NULL, szBuffer,
// 						g_szCaption, MB_ICONWARNING);
// 					Button_Enable(g_feat.name[i].hFeatValBtnPlus, FALSE);
// 					break;
// 				}
// 				j = g_feat.name[i].iFeatValNum++;
// 				g_feat.name[i].val[j].hFeatValEdt = CreateWindowEx(0, TEXT("edit"), NULL,
// 					WS_VISIBLE | WS_CHILD | WS_BORDER,
// 					0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);
// 				SendMessage(g_feat.name[i].val[j].hFeatValEdt, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
// 				if (g_feat.name[i].iFeatValNum == 2)
// 					Button_Enable(g_feat.name[i].hFeatValBtnMinus, TRUE);
// 				SendMessage(hwnd, WM_UPDATESCROLLINFO, 0, 0);
// 				InvalidateRect(hwnd, NULL, TRUE);
// 				UpdateWindow(hwnd);
// 				break;
// 			}
// 			else if (g_feat.name[i].hFeatValBtnMinus == (HWND)lParam)
// 			{
// 				logWrite(TEXT("减去按钮"));
// 				j = --g_feat.name[i].iFeatValNum;
// 				if (g_feat.name[i].iFeatValNum == MAX_FEAT_VAL_NUM - 1)
// 					Button_Enable(g_feat.name[i].hFeatValBtnPlus, TRUE);
// 				if (g_feat.name[i].iFeatValNum == 1)
// 					Button_Enable(g_feat.name[i].hFeatValBtnMinus,FALSE);
// 				if (DestroyWindow(g_feat.name[i].val[j].hFeatValEdt))
// 					g_feat.name[i].val[j].hFeatValEdt = NULL;
// 				SendMessage(hwnd, WM_UPDATESCROLLINFO, 0, 0);
// 				InvalidateRect(hwnd, NULL, TRUE);
// 				UpdateWindow(hwnd);
// 				break;
// 			}
// 		}
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

// RECT DrawFeatureWin(HDC hdc, int xBeg, int yBeg, BOOL isCal/* = FALSE*/)
// {
// 	// feat
// 	int yBase = 0, xMax = 0, yMax = 0, i, j;
// 	RECT rc{ 0 };
// 	
// 	for (i =  0; i < g_feat.iFeatNamNum; ++i)
// 	{
// 		if (!isCal) MoveWindow(g_feat.name[i].hFeatNameLbl, 22 - xBeg, yBase + 5 - yBeg, 50, 20, TRUE);
// 		if (!isCal) MoveWindow(g_feat.name[i].hFeatNameEdt, 82 - xBeg, yBase + 5 - yBeg, 71, 20, TRUE);
// 		if (!isCal) MoveWindow(g_feat.name[i].hFeatValLbl, 22 - xBeg, yBase + 35 - yBeg, 50, 20, TRUE);
// 		int xBase = 0;
// 		for (j = 0; j < g_feat.name[i].iFeatValNum; ++j)
// 		{
// 			if (!isCal) MoveWindow(g_feat.name[i].val[j].hFeatValEdt, xBase + 82 - xBeg, yBase + 35 - yBeg, 71, 20, TRUE);
// 			xBase += 80;
// 		}
// 		xBase -= 80;
// 		if (!isCal) MoveWindow(g_feat.name[i].hFeatValBtnPlus, xBase + 164 - xBeg, yBase + 35 - yBeg, 25, 20, TRUE);
// 		if (!isCal) MoveWindow(g_feat.name[i].hFeatValBtnMinus, xBase + 198 - xBeg, yBase + 35 - yBeg, 25, 20, TRUE);
// 		yBase += 60;
// 
// 		xMax = max(xBase + 198 + 20 + 10, xMax);
// 		yMax = max(yBase, yMax);
// 	}
// 	rc.right = xMax;
// 	rc.bottom = yMax;
// 	return rc;
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
		MoveWindow(g_hWinClss, 10, 20, cxClient - 20, cyClient - 30, TRUE);
		break;
	}
	default:
		break;
	}
	return oldClssGrpBoxProc(hwnd, message, wParam, lParam);
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
		//AddOneClss(g_clss, hwnd);
		Clss *c = new Clss();
		c->SetNamLblText(TEXT("类别名"));
		c->SetValLblText(TEXT("类别值"));
		ct.AddClss(c);
		break;
	}
	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);
		ct.MoveWindow(20, 5, TRUE);
// 		MoveWindow(g_clss.hClssNameLbl, 20, 5, 67, 20, TRUE);
// 		MoveWindow(g_clss.hClssNameEdt, 100, 5, 71, 20, TRUE);
// 		MoveWindow(g_clss.hClssValLbl, 20, 30, 67, 20, TRUE);
// 		MoveWindow(g_clss.hClssValEdt1, 100, 30, 71, 20, TRUE);
// 		MoveWindow(g_clss.hClssValEdt2, 180, 30, 71, 20, TRUE);
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
		MoveWindow(g_hBtnBuildDecTree, cxClient - 110, cyClient - 40, 100, 30, TRUE);
		MoveWindow(g_hWinDecTree, 10, 20, cxClient - 20, cyClient - 70, TRUE);
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
	return oldDecTreeGrpBoxProc(hwnd, message, wParam, lParam);
}


// BOOL DoCheckFeatAndClass()
// {
// 	// check feature
// 	int i, j;
// 	TCHAR szBuffer[256],szBuffer1[256],szBuffer2[256];
// 	for (i=0;i<g_feat.iFeatNamNum;++i)
// 	{
// 		Edit_GetText(g_feat.name[i].hFeatNameEdt, szBuffer1, sizeof szBuffer1);
// 		if (szBuffer1[0] == '\0')
// 		{
// 			Static_GetText(g_feat.name[i].hFeatNameLbl, szBuffer1, sizeof szBuffer1);
// 			wsprintf(szBuffer, TEXT("%s 为空,请填写数据"), szBuffer1);
// 			MessageBox(NULL, szBuffer, TEXT("数据生成器"), MB_ICONWARNING);
// 			SetFocus(g_feat.name[i].hFeatNameEdt);
// 			return FALSE;
// 		}
// 		for (j = 0; j < g_feat.name[i].iFeatValNum; ++j)
// 		{
// 			Edit_GetText(g_feat.name[i].val[j].hFeatValEdt, szBuffer2, sizeof szBuffer2);
// 			if (szBuffer2[0] == '\0')
// 			{
// 				Static_GetText(g_feat.name[i].hFeatNameLbl, szBuffer1, sizeof szBuffer1);
// 				wsprintf(szBuffer, TEXT("%s 的第%i个特征值为空,请填写数据"), szBuffer1, j+1);
// 				MessageBox(NULL, szBuffer, TEXT("数据生成器"), MB_ICONWARNING);
// 				SetFocus(g_feat.name[i].val[j].hFeatValEdt);
// 				return FALSE;
// 			}
// 		}
// 	}
// 
// 	// check class
// 	Edit_GetText(g_clss.hClssNameEdt, szBuffer1, sizeof szBuffer1);
// 	if (szBuffer1[0] == '\0')
// 	{
// 		Static_GetText(g_clss.hClssNameLbl, szBuffer2, sizeof szBuffer2);
// 		wsprintf(szBuffer, TEXT("%s 为空,请填写数据"), szBuffer2);
// 		MessageBox(NULL, szBuffer, TEXT("数据生成器"), MB_ICONWARNING);
// 		SetFocus(g_clss.hClssNameEdt);
// 		return FALSE;
// 	}
// 	Edit_GetText(g_clss.hClssValEdt1, szBuffer1, sizeof szBuffer1);
// 	if (szBuffer1[0] == '\0')
// 	{
// 		Static_GetText(g_clss.hClssValLbl, szBuffer2, sizeof szBuffer2);
// 		wsprintf(szBuffer, TEXT("%s1为空,请填写数据"), szBuffer2);
// 		MessageBox(NULL, szBuffer, TEXT("数据生成器"), MB_ICONWARNING);
// 		SetFocus(g_clss.hClssValEdt1);
// 		return FALSE;
// 	}
// 	Edit_GetText(g_clss.hClssValEdt2, szBuffer2, sizeof szBuffer2);
// 	if (szBuffer2[0] == '\0')
// 	{
// 		Static_GetText(g_clss.hClssValLbl, szBuffer1, sizeof szBuffer1);
// 		wsprintf(szBuffer, TEXT("%s2为空,请填写数据"), szBuffer1);
// 		MessageBox(NULL, szBuffer, TEXT("数据生成器"), MB_ICONWARNING);
// 		SetFocus(g_clss.hClssValEdt2);
// 		return FALSE;
// 	}
// 	return TRUE;
// }


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
		int iPaintEnd = min(logArry.size() - 1, iVerPos + ps.rcPaint.bottom / cyChar);
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
		//Edit_GetText(g_clss.hClssValEdt1, szBuffer, sizeof szBuffer);
		//ComboBox_AddString(hComboClss, szBuffer);
		//Edit_GetText(g_clss.hClssValEdt2, szBuffer, sizeof szBuffer);
		//ComboBox_AddString(hComboClss, szBuffer);
		//ComboBox_SetCurSel(hComboClss, 0);
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

HWND DoCreateTabControl(HWND hwndParent)
{
	INITCOMMONCONTROLSEX icex;
	HWND hwndTab;
	TCHAR szBuffer[256];  // Temporary buffer for strings.

	// Initialize common controls.
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_TAB_CLASSES;
	InitCommonControlsEx(&icex);

	hwndTab = CreateWindowEx(0, WC_TABCONTROL, NULL,
		TCS_FIXEDWIDTH | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		0, 0, 0, 0,
		hwndParent, NULL, g_hInst, NULL);
	
	return hwndTab;
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

// BOOL AddOneFeature(FEATURE& feat, HWND hWinFeat)
// {
// 	if (feat.iFeatNamNum >= MAX_FEAT_NUM) return FALSE;
// 	int i = feat.iFeatNamNum;
// 	TCHAR szBuffer[256];
// 	feat.iFeatNamNum++;
// 	wsprintf(szBuffer, TEXT("特征名%i"), i + 1);
// 	feat.name[i].hFeatNameLbl = CreateWindowEx(0, TEXT("static"), szBuffer,
// 		WS_VISIBLE | WS_CHILD | SS_RIGHT,
// 		0, 0, 0, 0, hWinFeat, NULL, g_hInst, NULL);
// 	SendMessage(feat.name[i].hFeatNameLbl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
// 
// 	feat.name[i].hFeatNameEdt = CreateWindowEx(0, TEXT("edit"), NULL,
// 		WS_VISIBLE | WS_CHILD | WS_BORDER,
// 		0, 0, 0, 0, hWinFeat, NULL, g_hInst, NULL);
// 	SendMessage(feat.name[i].hFeatNameEdt, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
// 
// 	wsprintf(szBuffer, TEXT("特征值"));
// 	feat.name[i].hFeatValLbl = CreateWindowEx(0, TEXT("static"), szBuffer,
// 		WS_VISIBLE | WS_CHILD | SS_RIGHT,
// 		0, 0, 0, 0, hWinFeat, NULL, g_hInst, NULL);
// 	SendMessage(feat.name[i].hFeatValLbl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
// 	
// 	feat.name[i].iFeatValNum = 1;
// 	feat.name[i].val[0].hFeatValEdt = CreateWindowEx(0, TEXT("edit"), NULL,
// 		WS_VISIBLE | WS_CHILD | WS_BORDER,
// 		0, 0, 0, 0, hWinFeat, NULL, g_hInst, NULL);
// 	SendMessage(feat.name[i].val[0].hFeatValEdt, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
// 
// 	feat.name[i].hFeatValBtnPlus = CreateWindowEx(0, TEXT("button"), TEXT("+"),
// 		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
// 		0, 0, 0, 0, hWinFeat, NULL, g_hInst, NULL);
// 	SendMessage(feat.name[i].hFeatValBtnPlus, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
// 
// 	feat.name[i].hFeatValBtnMinus = CreateWindowEx(0, TEXT("button"), TEXT("-"),
// 		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
// 		0, 0, 0, 0, hWinFeat, NULL, g_hInst, NULL);
// 	SendMessage(feat.name[i].hFeatValBtnMinus, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
// 	Button_Enable(feat.name[i].hFeatValBtnMinus, FALSE);
// 	return TRUE;
// }

// BOOL AddOneClss(CLSS& clss, HWND hWinClss)
// {
// 	TCHAR szBuffer[256];
// 	wsprintf(szBuffer, TEXT("类别名"));
// 	clss.hClssNameLbl = CreateWindowEx(0, TEXT("static"), szBuffer,
// 		WS_VISIBLE | WS_CHILD | SS_RIGHT,
// 		0, 0, 0, 0, hWinClss, NULL, g_hInst, NULL);
// 	SendMessage(clss.hClssNameLbl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
// 
// 	clss.hClssNameEdt = CreateWindowEx(0, TEXT("edit"), NULL,
// 		WS_VISIBLE | WS_CHILD | WS_BORDER,
// 		0, 0, 0, 0, hWinClss, NULL, g_hInst, NULL);
// 	SendMessage(clss.hClssNameEdt, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
// 
// 	wsprintf(szBuffer, TEXT("类别值"));
// 	clss.hClssValLbl = CreateWindowEx(0, TEXT("static"), szBuffer,
// 		WS_VISIBLE | WS_CHILD | SS_RIGHT,
// 		0, 0, 0, 0, hWinClss, NULL, g_hInst, NULL);
// 	SendMessage(clss.hClssValLbl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
// 
// 	clss.hClssValEdt1 = CreateWindowEx(0, TEXT("edit"), NULL,
// 		WS_VISIBLE | WS_CHILD | WS_BORDER,
// 		0, 0, 0, 0, hWinClss, NULL, g_hInst, NULL);
// 	SendMessage(clss.hClssValEdt1, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
// 
// 	clss.hClssValEdt2 = CreateWindowEx(0, TEXT("edit"), NULL,
// 		WS_VISIBLE | WS_CHILD | WS_BORDER,
// 		0, 0, 0, 0, hWinClss, NULL, g_hInst, NULL);
// 	SendMessage(clss.hClssValEdt2, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
// 	return TRUE;
// }