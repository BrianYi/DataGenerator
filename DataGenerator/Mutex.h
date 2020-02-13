#pragma once
#include <Windows.h>

class Mutex
{
public:
	Mutex();
	~Mutex();
	void Lock();
	void Unlock();
    BOOL TryLock();
private:
    void RecursiveLock();
    void RecursiveUnlock();
    BOOL RecursiveTryLock();
private:
	CRITICAL_SECTION fMutex;
	DWORD fHolderCount;
	DWORD fHolder;
};

class MutexLocker
{
public:
    MutexLocker(Mutex *inMutex);
    ~MutexLocker();
    void Lock();
    void Unlock();
private:
    Mutex *fMutex;
};
