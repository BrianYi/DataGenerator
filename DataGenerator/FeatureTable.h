#pragma once
#include "Header.h"
#include <vector>

class FeatVal
{
public:
	FeatVal(HWND hWndParent);
	~FeatVal() { DestroyWindow(hValEdt); }
	int Flag() { return bFlag; }
	void SetFlag(int flg) { bFlag = flg; }
	RECT MoveWindow(int x, int y, BOOL bRepaint);
	int GetText(__out TCHAR* szFeatVal, __in int iMaxSize);
	void SetFocus();
	BOOL AnyEmpty();
private:
	HWND hValEdt;
	int bFlag;
};

class Feat
{
public:
	Feat(HWND hWndParent);
	~Feat();
	void SetNamLblText(const TCHAR *format,...);
	void SetValLblText(const TCHAR *format,...);
	void AddFeatVal(FeatVal *featVal);
	void DelFeatVal(int vid);
	std::vector<FeatVal*>& GetFeatValArry();
	FeatVal *GetFeatVal(int vid);
	FeatVal *BackFeatVal();
	FeatVal *FrontFeatVal();
	int CountVal() { return vals.size(); }
	int Flag() { return bFlag; }
	void SetFlag(int flg) { bFlag = flg; }
	int GetNamLblText(__out TCHAR* szLblTxt, __in int iMaxSize);
	int GetNamEdtText(__out TCHAR* szClssName, __in int iMaxSize);
	RECT MoveWindow(int x, int y, BOOL bRepaint);
	BOOL AnyEmpty();
	void SetFocus();
private:
	HWND hNameLbl;
	HWND hNameEdt;
	HWND hValLbl;
	HWND hBtnPlus, hBtnMinus;
	int bFlag;
	std::vector<FeatVal*> vals;
};

class FeatureTable
{
public:
	FeatureTable();
	~FeatureTable();
	void AddFeat(Feat* feat);
	void AddFeatVal(int fid, FeatVal* featVal);
	void DelFeatVal(int fid, int vid);
	std::vector<Feat*>& GetFeatArry();
	Feat* GetFeat(int fid);
	Feat* BackFeat();
	Feat* FrontFeat();
	void SetFeature(int fid, Feat* feat);
	int CountFeature() { return fFeatArry.size(); }
	RECT MoveWindow(int x, int y, BOOL bRepaint);
	BOOL AnyEmpty();
private:
	std::vector<Feat*> fFeatArry;
};

