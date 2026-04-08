#pragma once
#include "SpinLock.h"
#include "LockGuard.h"
#include <cstdlib>
#include <array>

static constexpr size_t POOL_SIZE_CLASSES[] = { 32, 64, 128, 256, 512, 1024, 2048, 4096 };
static constexpr int    NUM_SIZE_CLASSES    = 8;
static constexpr size_t MAX_POOL_SIZE       = 4096;

// 반환된 블록을 연결 리스트로 관리하는 노드
struct FreeNode
{
    FreeNode* next;
};

// 특정 크기 클래스의 프리리스트 (SpinLock으로 스레드 안전 보장)
class MemoryBucket
{
public:
    void* Pop()
    {
        LockGuard<SpinLock> guard(_lock);
        if (_head == nullptr)
            return nullptr;

        FreeNode* node = _head;
        _head = _head->next;
        return node;
    }

    void Push(void* ptr)
    {
        LockGuard<SpinLock> guard(_lock);
        FreeNode* node = static_cast<FreeNode*>(ptr);
        node->next = _head;
        _head = node;
    }

private:
    FreeNode* _head = nullptr;
    SpinLock  _lock;
};

// 크기 클래스별 버킷을 관리하는 메모리 풀
class MemoryPool
{
public:
    static void* Allocate(size_t size)
    {
        int idx = GetBucketIndex(size);
        if (idx < 0)
            return std::malloc(size);

        void* ptr = _buckets[idx].Pop();
        if (ptr == nullptr)
            ptr = std::malloc(POOL_SIZE_CLASSES[idx]);

        return ptr;
    }

    static void Release(void* ptr, size_t size)
    {
        if (ptr == nullptr)
            return;

        int idx = GetBucketIndex(size);
        if (idx < 0)
        {
            std::free(ptr);
            return;
        }

        _buckets[idx].Push(ptr);
    }

private:
    // 요청 크기 이상인 가장 작은 버킷 인덱스 반환, 초과 시 -1
    static int GetBucketIndex(size_t size)
    {
        for (int i = 0; i < NUM_SIZE_CLASSES; i++)
        {
            if (size <= POOL_SIZE_CLASSES[i])
                return i;
        }
        return -1;
    }

    static std::array<MemoryBucket, NUM_SIZE_CLASSES> _buckets;
};

inline std::array<MemoryBucket, NUM_SIZE_CLASSES> MemoryPool::_buckets{};

// 전역 new/delete 오버라이드 — MemoryPool.h include 시 프로젝트 전체 적용
inline void* operator new(size_t size)   { return MemoryPool::Allocate(size); }
inline void* operator new[](size_t size) { return MemoryPool::Allocate(size); }

inline void operator delete(void* ptr, size_t size) noexcept   { MemoryPool::Release(ptr, size); }
inline void operator delete[](void* ptr, size_t size) noexcept { MemoryPool::Release(ptr, size); }
