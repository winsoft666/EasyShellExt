#include "pch.h"
#include "CriticalSection.h"

CriticalSection::CriticalSection() {
    InitializeCriticalSection(&mCS);
}

CriticalSection::~CriticalSection() {
    DeleteCriticalSection(&mCS);
}

void CriticalSection::Enter() {
    EnterCriticalSection(&mCS);
}

void CriticalSection::Leave() {
    LeaveCriticalSection(&mCS);
}

CriticalSectionGuard::CriticalSectionGuard(CriticalSection* cs) {
    cs_ = cs;
    if (cs_) {
        cs_->Enter();
    }
}

CriticalSectionGuard::~CriticalSectionGuard() {
    if (cs_) {
        cs_->Leave();
    }
    cs_ = NULL;
}
