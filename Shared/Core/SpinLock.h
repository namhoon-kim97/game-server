#pragma once
#include <atomic>
#include <intrin.h>

// 짧은 임계 구역 전용 스핀락 (ex: MemoryPool 프리리스트)
// 락 경합 시 sleep 없이 CPU를 돌리므로 긴 작업에는 사용 금지
class SpinLock
{
public:
    void Lock()
    {
        while (_flag.test_and_set(std::memory_order_acquire))
            _mm_pause(); // 스핀 대기 중 CPU 자원 양보 (HyperThreading 최적화)
    }

    void Unlock()
    {
        _flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag _flag = ATOMIC_FLAG_INIT;
};
