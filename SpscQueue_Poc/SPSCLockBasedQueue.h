#pragma once

//! Threading
#include <atomic>
#include <thread>
#include <chrono>

template <typename ElementsDataType>
class SPSCLockBasedQueue
{
public:
    SPSCLockBasedQueue(int p_tMaxBufferSize);
    ~SPSCLockBasedQueue();

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
    std::mutex m_oBufferMutex;
    std::condition_variable m_oDataReadyCv;
    std::condition_variable m_oSlotAvailableCv;
    int m_iMaxBufferSize;
    alignas(CACHE_SIZE) int m_iWritePosition{-1};
    alignas(CACHE_SIZE) int m_iReadPosition{-1};
    alignas(CACHE_SIZE) bool m_bIsWritingEnabled{true};
    alignas(CACHE_SIZE) bool m_bIsReadingEnabled{true};
    ElementsDataType **m_arrBuffer;
};

#include "SPSCLockBasedQueue.ipp"