#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include <windowsx.h>
#include <cmath>
#include <processthreadsapi.h>
#include <tchar.h>

#define Assert(condition)    {                              \
                                                                \
        if (!(condition))                                       \
        {                                                       \
            TCHAR s[512];                            \
            s[511] = 0;                          \
            _snwprintf_s (s,sizeof(s), _T("_Assert: %s, %d"),_T(__FILE__), __LINE__ ); \
            _tprintf(_T("%s\n"), s); \
        }   }

#define AssertV(condition,errNo)    {                                   \
        if (!(condition))                                                   \
        {                                                                   \
            TCHAR s[512];                                        \
            s[511] = 0;                                      \
            _snwprintf_s ( s,sizeof(s), _T("_AssertV: %s, %d (%d)"),_T(__FILE__), __LINE__, errNo );    \
            _tprintf(_T("%s\n"), s); \
        }   }