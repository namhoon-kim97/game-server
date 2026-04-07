#pragma once
#include "SpinLock.h"
#include "RWLock.h"

// RAII 락 가드 — 스코프 종료 시 자동 Unlock (예외 상황에서도 안전)

template<typename T>
class LockGuard
{
public:
    explicit LockGuard(T& lock) : _lock(lock) { _lock.Lock(); }
    ~LockGuard() { _lock.Unlock(); }

    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;

private:
    T& _lock;
};

// RWLock 읽기 전용 가드
class ReadLockGuard
{
public:
    explicit ReadLockGuard(RWLock& lock) : _lock(lock) { _lock.ReadLock(); }
    ~ReadLockGuard() { _lock.ReadUnlock(); }

    ReadLockGuard(const ReadLockGuard&) = delete;
    ReadLockGuard& operator=(const ReadLockGuard&) = delete;

private:
    RWLock& _lock;
};

// RWLock 쓰기 전용 가드
class WriteLockGuard
{
public:
    explicit WriteLockGuard(RWLock& lock) : _lock(lock) { _lock.WriteLock(); }
    ~WriteLockGuard() { _lock.WriteUnlock(); }

    WriteLockGuard(const WriteLockGuard&) = delete;
    WriteLockGuard& operator=(const WriteLockGuard&) = delete;

private:
    RWLock& _lock;
};
