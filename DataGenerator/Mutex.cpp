#include "Mutex.h"
#include "Header.h"


Mutex::Mutex()
{
	::InitializeCriticalSection(&fMutex);
	fHolder = 0;
	fHolderCount = 0;
}


Mutex::~Mutex()
{
	::DeleteCriticalSection(&fMutex);
}


void Mutex::Lock()
{
	this->RecursiveLock();
}

void Mutex::Unlock()
{
    this->RecursiveUnlock();
}

void Mutex::RecursiveLock()
{
	if (::GetCurrentThreadId() == fHolder)
	{
		fHolderCount++;
		return;
	}
	::EnterCriticalSection(&fMutex);
    Assert(fHolder == 0);
	fHolder = GetCurrentThreadId();
	fHolderCount++;
    Assert(fHolderCount == 1);
}

void Mutex::RecursiveUnlock()
{
    if (GetCurrentThreadId() != fHolder)
        return;

    Assert(fHolderCount > 0);
    fHolderCount--;
    if (fHolderCount == 0) 
    {
        fHolder = 0;
        ::LeaveCriticalSection(&fMutex);
    }
}

BOOL Mutex::RecursiveTryLock()
{
    if (GetCurrentThreadId() == fHolder) 
    {
        fHolderCount++;
        return true;
    }

    BOOL theErr = ::TryEnterCriticalSection(&fMutex);
    if (!theErr)
        return theErr;

    Assert(fHolder == 0);
    fHolder = GetCurrentThreadId();
    fHolderCount++;
    Assert(fHolderCount == 1);
    return true;
}

BOOL Mutex::TryLock()
{
    return RecursiveTryLock();
}

MutexLocker::MutexLocker(Mutex *inMutex):
    fMutex(inMutex)
{
    if (fMutex)
        fMutex->Lock();
}

MutexLocker::~MutexLocker()
{
    if (fMutex)
        fMutex->Unlock();
}

void MutexLocker::Lock()
{
    if (fMutex)
        fMutex->Lock();
}

void MutexLocker::Unlock()
{
    if (fMutex)
        fMutex->Unlock();
}
