#include "FeatureTable.h"

extern HINSTANCE g_hInst;

Feat::Feat(HWND hWndParent)
{
	bFlag = 0;
	//TCHAR szBuffer[64];
	//wsprintf(szBuffer, TEXT("特征名%i"), i + 1);
	hNameLbl = CreateWindowEx(0, TEXT("static"), NULL,
		WS_VISIBLE | WS_CHILD | SS_RIGHT,
		0, 0, 0, 0, hWndParent, NULL, g_hInst, NULL);
	SendMessage(hNameLbl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

	hNameEdt = CreateWindowEx(0, TEXT("edit"), NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER,
		0, 0, 0, 0, hWndParent, NULL, g_hInst, NULL);
	SendMessage(hNameEdt, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

	hValLbl = CreateWindowEx(0, TEXT("static"), NULL,
		WS_VISIBLE | WS_CHILD | SS_RIGHT,
		0, 0, 0, 0, hWndParent, NULL, g_hInst, NULL);
	SendMessage(hValLbl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

	vals.push_back(new FeatVal(hWndParent));


	// 
	hBtnPlus = CreateWindowEx(0, TEXT("button"), TEXT("+"),
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		0, 0, 0, 0, hWndParent, NULL, g_hInst, NULL);
	SendMessage(hBtnPlus, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
	SetWindowLongPtr(hBtnPlus, 0, (LONG)this);

	hBtnMinus = CreateWindowEx(0, TEXT("button"), TEXT("-"),
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		0, 0, 0, 0, hWndParent, NULL, g_hInst, NULL);
	SendMessage(hBtnMinus, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
	SetWindowLongPtr(hBtnMinus, 0, (LONG)this);
	Button_Enable(hBtnMinus, FALSE);
}

Feat::~Feat()
{
	DestroyWindow(hNameLbl);
	DestroyWindow(hNameEdt);
	DestroyWindow(hValLbl);
	DestroyWindow(hBtnPlus);
	DestroyWindow(hBtnMinus);

	for (auto it:vals)
	{
		delete it;
	}
}

void Feat::SetNamLblText(const TCHAR *format, ...)
{
	TCHAR buffer[256];
	va_list args;
	va_start(args, format);
	_vstprintf(buffer, sizeof buffer, format, args);
	va_end(args);
	Static_SetText(hNameLbl, buffer);
}

void Feat::SetValLblText(const TCHAR *format, ...)
{
	TCHAR buffer[256];
	va_list args;
	va_start(args, format);
	_vstprintf(buffer, sizeof buffer, format, args);
	va_end(args);
	Static_SetText(hValLbl, buffer);
}

void Feat::AddFeatVal(FeatVal *featVal)
{
	vals.push_back(featVal);
}

void Feat::DelFeatVal(int vid)
{
	if (vid >= vals.size()) throw;
	delete vals[vid];
	for (int i = vid+1; i < vals.size(); ++i)
		vals[i - 1] = vals[i];
	vals.pop_back();
}


FeatVal * Feat::GetFeatVal(int vid)
{
	if (vid < 0 || vid >= vals.size()) throw;
	return vals[vid];
}

int Feat::GetNamLblText(__out TCHAR* szLblTxt, __in int iMaxSize)
{
	return Static_GetText(hNameLbl, szLblTxt, iMaxSize);
}

int Feat::GetNamEdtText(__out TCHAR* szClssName, __in int iMaxSize)
{
	return Edit_GetText(hNameEdt, szClssName, iMaxSize);
}

RECT Feat::MoveWindow(int x, int y, BOOL bRepaint)
{
	// x=22,y=5
	::MoveWindow(hNameLbl, x, y, 50, 20, bRepaint);
	::MoveWindow(hNameEdt, x+60, y, 71, 20, bRepaint);
	::MoveWindow(hValLbl, x, y + 30, 50, 20, bRepaint);
	RECT rcBig = { x, y, x + 60 + 71, y + 50 };
	int xBase = 0;
	for (int j = 0; j < vals.size(); ++j)
	{
		RECT rc = vals[j]->MoveWindow(xBase + x + 60, y + 30, TRUE);
		UnionRect(&rcBig, &rcBig, &rc);
		xBase += 80;
	}
	return rcBig;
}

BOOL Feat::AnyEmpty()
{
	TCHAR szName[64], szVal[64], szNameLbl[64], szValLbl[64], szBuf[64];
	int iSize = GetNamEdtText(szName, sizeof szName);
	if (szName[0] == '\0')
		return TRUE;
	for (int j = 0; j < vals.size(); ++j)
	{
		if (vals[j]->AnyEmpty())
		{
			Static_GetText(hValLbl, szValLbl, sizeof szValLbl);
			wsprintf(szBuf, TEXT("%s 的第%i个特征值为空,请填写数据"), szNameLbl, j + 1);
			MessageBox(NULL, szBuf, TEXT("数据生成器"), MB_ICONWARNING);
			vals[j]->SetFocus();
			return TRUE;
		}
	}
	return FALSE;
}

void Feat::SetFocus()
{
	::SetFocus(hNameEdt);
}

FeatVal::FeatVal(HWND hWndParent)
{
	bFlag = 0;

	hValEdt = CreateWindowEx(0, TEXT("edit"), NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER,
		0, 0, 0, 0, hWndParent, NULL, g_hInst, NULL);
	SendMessage(hValEdt, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
}


RECT FeatVal::MoveWindow(int x, int y, BOOL bRepaint)
{
	// x=82,y=35,
	::MoveWindow(hValEdt, x, y, 71, 20, bRepaint);
	return RECT{ x,y,x+71,y+20 };
}

int FeatVal::GetText(__out TCHAR* szFeatVal, __in int iMaxSize)
{
	return Edit_GetText(hValEdt, szFeatVal, iMaxSize);
}

void FeatVal::SetFocus()
{
	::SetFocus(hValEdt);
}

BOOL FeatVal::AnyEmpty()
{
	TCHAR szVal[64];
	int iSize = GetText(szVal, sizeof szVal);
	if (szVal[0] == '\0')
		return TRUE;
	return FALSE;
}

FeatureTable::FeatureTable()
{
}


FeatureTable::~FeatureTable()
{
	for (auto it:fFeatArry)
	{
		delete it;
	}
}

void FeatureTable::AddFeat(Feat* feat)
{
	fFeatArry.push_back(feat);
}

void FeatureTable::AddFeatVal(int fid, FeatVal* featVal)
{
	if (fid > fFeatArry.size()) throw;
	fFeatArry[fid]->AddFeatVal(featVal);
}

void FeatureTable::DelFeatVal(int fid, int vid)
{
	if (fid < 0 || fid >= fFeatArry.size()) throw;
	if (vid < 0 || vid >= fFeatArry[fid]->CountVal()) throw;
	fFeatArry[fid]->DelFeatVal(vid);
}

std::vector<Feat*>& FeatureTable::GetFeatArry()
{
	return fFeatArry;
}

Feat* FeatureTable::GetFeat(int fid)
{
	if (fid < 0 || fid >= fFeatArry.size()) throw;
	return fFeatArry[fid];
}

void FeatureTable::SetFeature(int fid, Feat* feat)
{
	if (fFeatArry.size() >= fid) throw;
	delete fFeatArry[fid];
	fFeatArry[fid] = feat;
}

RECT FeatureTable::MoveWindow(int x, int y, BOOL bRepaint)
{
	// feat
	int yBase = 0, xMax = 0, yMax = 0, i, j;
	RECT rcBig{ x,y,x,y };

	for (i = 0; i < fFeatArry.size(); ++i)
	{
		RECT rc = fFeatArry[i]->MoveWindow(x, y - yBase, bRepaint);
		UnionRect(&rcBig, &rcBig, &rc);
		yBase += 60;
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

// 		xMax = max(xBase + 198 + 20 + 10, xMax);
// 		yMax = max(yBase, yMax);
	}
	return rcBig;
}

BOOL FeatureTable::AnyEmpty()
{
	// check feature
	TCHAR szBuf[64], szLblTxt[64];
	for (int i = 0; i < fFeatArry.size(); ++i)
	{
		if (fFeatArry[i]->AnyEmpty())
		{
			fFeatArry[i]->GetNamLblText(szLblTxt, sizeof szLblTxt);
			wsprintf(szBuf, TEXT("%s 为空,请填写数据"), szLblTxt);
			MessageBox(NULL, szBuf, TEXT("数据生成器"), MB_ICONWARNING);
			fFeatArry[i]->SetFocus();
			return TRUE;
		}
	}




// 	int i, j;
// 	TCHAR szBuffer[256], szBuffer1[256], szBuffer2[256];
// 	for (i = 0; i < g_feat.iFeatNamNum; ++i)
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
// 				wsprintf(szBuffer, TEXT("%s 的第%i个特征值为空,请填写数据"), szBuffer1, j + 1);
// 				MessageBox(NULL, szBuffer, TEXT("数据生成器"), MB_ICONWARNING);
// 				SetFocus(g_feat.name[i].val[j].hFeatValEdt);
// 				return FALSE;
// 			}
// 		}
// 	}

	
	return FALSE;
}

