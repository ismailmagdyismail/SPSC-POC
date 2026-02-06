#pragma once

#include <atomic>

template <typename ElementsType>
class WaitFreeQueue
{
public:
    WaitFreeQueue(unsigned int p_tMaxBufferSize);
    ~WaitFreeQueue();

    void StopWriting();
    void StopReading();

    void Push(ElementsType *p_pElement);
    ElementsType *Pop();

private:
    void UpdateIndex(unsigned int &p_iPosition);

    ElementsType **m_pBuffer;
    static const unsigned int CACHE_LINE_SIZE = 64;

    alignas(WaitFreeQueue::CACHE_LINE_SIZE) unsigned int m_uiMaxBufferSize;
    alignas(WaitFreeQueue::CACHE_LINE_SIZE) std::atomic<unsigned int> m_uiSize{0};
    alignas(WaitFreeQueue::CACHE_LINE_SIZE) unsigned int m_uiWritePosition{0};
    alignas(WaitFreeQueue::CACHE_LINE_SIZE) unsigned int m_uiReadPosition{0};
    alignas(WaitFreeQueue::CACHE_LINE_SIZE) std::atomic<bool> m_bIsWritingEnabled{true};
    alignas(WaitFreeQueue::CACHE_LINE_SIZE) std::atomic<bool> m_bIsReadingEnabled{true};
};

#include "WaitFreeQueue.ipp"