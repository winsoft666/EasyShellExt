#pragma once

class CriticalSection {
   protected:
    CRITICAL_SECTION mCS;

   public:
    CriticalSection();
    virtual ~CriticalSection();

    void Enter();
    void Leave();
};

class CriticalSectionGuard {
   protected:
    CriticalSection* cs_;

   public:
    CriticalSectionGuard(CriticalSection* cs);
    ~CriticalSectionGuard();
};
