#pragma once

//! Threading
#include <atomic>
#include <thread>
#include <chrono>

template <typename ElementsDataType>
class SPSCLockFreeQueue
{
public:
    SPSCLockFreeQueue(int p_tMaxBufferSize);
    ~SPSCLockFreeQueue();

    void StopWriting();
    void StopReading();

    void Push(ElementsDataType *p_pElement);
    ElementsDataType *Pop();

private:
    inline int GetNextPosition(int p_uiPosition);
    inline int GetNextReadPosition();
    inline int GetNextWritePosition();
    inline int GetLastWrittenPosition();
    inline int GetLastReadPosition();
    inline bool DidPositionWrap(int p_iNewPosition, int p_iOldPosition);
    inline bool HasElementsToRead();
    inline bool HasSpaceToWrite();

    static const unsigned int CACHE_SIZE = 64;
    int m_iMaxBufferSize;
    alignas(CACHE_SIZE) std::atomic<int> m_iWritePosition{-1};
    alignas(CACHE_SIZE) std::atomic<int> m_iReadPosition{-1};
    alignas(CACHE_SIZE) std::atomic<bool> m_bIsWritingEnabled{true};
    alignas(CACHE_SIZE) std::atomic<bool> m_bIsReadingEnabled{true};
    ElementsDataType **m_arrBuffer;
};

#include "SPSCLockFreeQueue.ipp"