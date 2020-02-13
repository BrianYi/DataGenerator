#include "ClssTable.h"

extern HWND g_hWinClss;
extern HINSTANCE g_hInst;

ClssTable::ClssTable()
{
}


ClssTable::~ClssTable()
{
	for (auto it : fClssArry)
	{
		delete it;
	}
}

void ClssTable::AddClss(Clss* clss)
{
	fClssArry.push_back(clss);
}

void ClssTable::AddClssVal(int cid, ClssVal* clssVal)
{
	if (cid > fClssArry.size()) throw;
	fClssArry[cid]->AddClssVal(clssVal);
}

void ClssTable::DelClssVal(int cid, int vid)
{
	if (cid < 0 || cid >= fClssArry.size()) throw;
	if (vid < 0 || vid >= fClssArry[cid]->CountVal()) throw;
	fClssArry[cid]->DelClssVal(vid);
}

std::vector<Clss*>& ClssTable::GetClssArry()
{
	return fClssArry;
}

Clss * ClssTable::GetClss(int cid)
{
	if (cid < 0 || cid >= fClssArry.size()) throw;
	return fClssArry[cid];
}

void ClssTable::SetClss(int cid, Clss* clss)
{
	if (fClssArry.size() >= cid) throw;
	delete fClssArry[cid];
	fClssArry[cid] = clss;
}

BOOL ClssTable::AnyEmpty()
{
	// check class
	TCHAR szBuf[64],szLblTxt[64];
	for (int i = 0; i < fClssArry.size(); ++i)
	{
		if (fClssArry[i]->AnyEmpty())
		{
			fClssArry[i]->GetNamLblText(szLblTxt, sizeof szLblTxt);
			wsprintf(szBuf, TEXT("%s 为空,请填写数据"), szLblTxt);
			MessageBox(NULL, szBuf, TEXT("数据生成器"), MB_ICONWARNING);
			fClssArry[i]->SetFocus();
			return TRUE;
		}
	}



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
	return FALSE;
}

RECT ClssTable::MoveWindow(int x, int y, BOOL bRepaint)
{
	// x=20, y=5
	RECT rcBig{ x,y,x,y };
	for (int i = 0; i < fClssArry.size(); ++i)
	{
		RECT rc=fClssArry[i]->MoveWindow(x, y, bRepaint);
		UnionRect(&rcBig, &rcBig, &rc);
	}
	return rcBig;
}

Clss::Clss()
{
	bFlag = 0;

	TCHAR szBuffer[256];
	hNameLbl = CreateWindowEx(0, TEXT("static"), NULL,
		WS_VISIBLE | WS_CHILD | SS_RIGHT,
		0, 0, 0, 0, g_hWinClss, NULL, g_hInst, NULL);
	SendMessage(hNameLbl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

	hNameEdt = CreateWindowEx(0, TEXT("edit"), NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER,
		0, 0, 0, 0, g_hWinClss, NULL, g_hInst, NULL);
	SendMessage(hNameEdt, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

	hValLbl = CreateWindowEx(0, TEXT("static"), NULL,
		WS_VISIBLE | WS_CHILD | SS_RIGHT,
		0, 0, 0, 0, g_hWinClss, NULL, g_hInst, NULL);
	SendMessage(hValLbl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

	vals.push_back(new ClssVal());
	vals.push_back(new ClssVal());
}

Clss::~Clss()
{
	DestroyWindow(hNameLbl);
	DestroyWindow(hNameEdt);
	DestroyWindow(hValLbl);
	for (auto it:vals)
	{
		delete it;
	}
}

void Clss::SetNamLblText(const TCHAR *format, ...)
{
	TCHAR buffer[256];
	va_list args;
	va_start(args, format);
	_vstprintf(buffer, sizeof buffer, format, args);
	va_end(args);
	Static_SetText(hNameLbl, buffer);
}

void Clss::SetValLblText(const TCHAR *format, ...)
{
	TCHAR buffer[256];
	va_list args;
	va_start(args, format);
	_vstprintf(buffer, sizeof buffer, format, args);
	va_end(args);
	Static_SetText(hValLbl, buffer);
}

void Clss::AddClssVal(ClssVal *clssVal)
{
	vals.push_back(clssVal);
}

void Clss::DelClssVal(int vid)
{
	if (vid >= vals.size()) throw;
	delete vals[vid];
	for (int i = vid + 1; i < vals.size(); ++i)
		vals[i - 1] = vals[i];
	vals.pop_back();
}

ClssVal * Clss::GetClssVal(int vid)
{
	if (vid < 0 || vid >= vals.size()) throw;
	return vals[vid];
}

int Clss::GetNamLblText(__out TCHAR* szLblTxt, __in int iMaxSize)
{
	return Static_GetText(hNameLbl, szLblTxt, iMaxSize);
}

int Clss::GetNamEdtText(__out TCHAR* szClssName, __in int iMaxSize)
{
	return Edit_GetText(hNameEdt, szClssName, iMaxSize);
}

RECT Clss::MoveWindow(int x, int y, BOOL bRepaint)
{
	RECT rcBig{ x,y,x,y };
	// 
	::MoveWindow(hNameLbl, x, y, 67, 20, bRepaint);
	::MoveWindow(hNameEdt, x + 80, y, 71, 20, bRepaint);
	::MoveWindow(hValLbl, x, y + 25, 67, 20, bRepaint);
	int xBase = 0;
	for (int j = 0; j < vals.size(); ++j)
	{
		RECT rc=vals[j]->MoveWindow(x + 80 + xBase, y + 25, bRepaint);
		UnionRect(&rcBig, &rcBig, &rc);
		xBase += 80;
	}
	return rcBig;
// 	MoveWindow(g_clss.hClssNameLbl, 20, 5, 67, 20, TRUE);
// 	MoveWindow(g_clss.hClssNameEdt, 100, 5, 71, 20, TRUE);
// 	MoveWindow(g_clss.hClssValLbl, 20, 30, 67, 20, TRUE);
// 	MoveWindow(g_clss.hClssValEdt1, 100, 30, 71, 20, TRUE);
// 	MoveWindow(g_clss.hClssValEdt2, 180, 30, 71, 20, TRUE);
}

BOOL Clss::AnyEmpty()
{
	TCHAR szName[64],szVal[64],szNameLbl[64],szValLbl[64],szBuf[64];
	int iSize = GetNamEdtText(szName, sizeof szName);
	if (szName[0] == '\0')
		return TRUE;
	for (int j=0;j<vals.size();++j)
	{
		if (vals[j]->AnyEmpty())
		{
			Static_GetText(hValLbl, szValLbl, sizeof szValLbl);
			wsprintf(szBuf, TEXT("%s 的第%i个类别值为空,请填写数据"), szNameLbl, j+1);
			MessageBox(NULL, szBuf, TEXT("数据生成器"), MB_ICONWARNING);
			vals[j]->SetFocus();
			return TRUE;
		}
	}
	return FALSE;
}

void Clss::SetFocus()
{
	::SetFocus(hNameEdt);
}

ClssVal::ClssVal()
{
	bFlag = 0;

	hValEdt = CreateWindowEx(0, TEXT("edit"), NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER,
		0, 0, 0, 0, g_hWinClss, NULL, g_hInst, NULL);
	SendMessage(hValEdt, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
}

RECT ClssVal::MoveWindow(int x, int y, BOOL bRepaint)
{
	RECT rc{ x,y,71,20 };
	::MoveWindow(hValEdt, x, y, 71, 20, bRepaint);
	return rc;
}

int ClssVal::GetText(__out TCHAR* szClssVal, __in int iMaxSize)
{
	return Edit_GetText(hValEdt, szClssVal, iMaxSize);
}

void ClssVal::SetFocus()
{
	::SetFocus(hValEdt);
}

BOOL ClssVal::AnyEmpty()
{
	TCHAR szVal[64];
	int iSize = GetText(szVal, sizeof szVal);
	if (szVal[0] == '\0')
		return TRUE;
	return FALSE;
}
