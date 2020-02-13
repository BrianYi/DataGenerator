#pragma once
#include "Header.h"
#include <vector>


class ClssVal
{
public:
	ClssVal();
	~ClssVal() { DestroyWindow(hValEdt); }
	int Flag() { return bFlag; }
	void SetFlag(int flg) { bFlag = flg; }
	RECT MoveWindow(int x, int y, BOOL bRepaint);
	int GetText(__out TCHAR* szClssVal, __in int iMaxSize);
	void SetFocus();
	BOOL AnyEmpty();
private:
	HWND hValEdt;
	int bFlag;
};

class Clss
{
public:
	Clss();
	~Clss();
	void SetNamLblText(const TCHAR *format, ...);
	void SetValLblText(const TCHAR *format, ...);
	void AddClssVal(ClssVal *clssVal);
	void DelClssVal(int vid);
	ClssVal *GetClssVal(int vid);
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
	int bFlag;
	std::vector<ClssVal*> vals;
};

class ClssTable
{
public:
	ClssTable();
	~ClssTable();

	void AddClss(Clss* clss);
	void AddClssVal(int cid, ClssVal* clssVal);
	void DelClssVal(int cid, int vid);
	std::vector<Clss*>& GetClssArry();
	Clss *GetClss(int cid);
	void SetClss(int cid, Clss* clss);
	BOOL AnyEmpty();
	int CountClss() { return fClssArry.size(); }
	RECT MoveWindow(int x, int y, BOOL bRepaint);
private:
	std::vector<Clss*> fClssArry;
};

