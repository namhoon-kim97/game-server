#pragma once
#include <shared_mutex>

// 읽기/쓰기 락 — 읽기는 다수 동시 허용, 쓰기는 단독 점유
// 읽기 빈도가 압도적으로 높은 곳에 사용 (ex: SessionManager 세션 맵)
class RWLock
{
public:
    void ReadLock()    { _mutex.lock_shared(); }
    void ReadUnlock()  { _mutex.unlock_shared(); }

    void WriteLock()   { _mutex.lock(); }
    void WriteUnlock() { _mutex.unlock(); }

private:
    std::shared_mutex _mutex;
};
