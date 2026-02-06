#include <thread>
#include <chrono>

template <typename ElementsType>
WaitFreeQueue<ElementsType>::WaitFreeQueue(unsigned int p_tMaxBufferSize)
    : m_uiMaxBufferSize(p_tMaxBufferSize)
{
    m_pBuffer = new ElementsType *[p_tMaxBufferSize];
}

template <typename ElementsType>
WaitFreeQueue<ElementsType>::~WaitFreeQueue()
{
    //! TODO: delete
}

template <typename ElementsType>
void WaitFreeQueue<ElementsType>::Push(ElementsType *p_pElement)
{
    bool bIsWritingEnabled{true};
    while ((m_uiSize.load(std::memory_order_relaxed) == m_uiMaxBufferSize) && (bIsWritingEnabled = m_bIsWritingEnabled.load(std::memory_order_relaxed)))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (!bIsWritingEnabled)
    {
        return;
    }
    m_pBuffer[m_uiWritePosition] = p_pElement;
    UpdateIndex(m_uiWritePosition);
    m_uiSize.fetch_add(1, std::memory_order_release);
}

template <typename ElementsType>
ElementsType *WaitFreeQueue<ElementsType>::Pop()
{
    bool bIsReadingEnabled{true};
    while ((m_uiSize.load(std::memory_order_acquire) == 0) && (bIsReadingEnabled = m_bIsReadingEnabled.load(std::memory_order_relaxed)))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (!bIsReadingEnabled)
    {
        return nullptr;
    }
    ElementsType *pElement = m_pBuffer[m_uiReadPosition];
    UpdateIndex(m_uiReadPosition);
    m_uiSize.fetch_sub(1, std::memory_order_relaxed); //! No need for two-way barriers
    return pElement;
}

template <typename ElementsType>
void WaitFreeQueue<ElementsType>::StopWriting()
{
    m_bIsWritingEnabled.store(false, std::memory_order_relaxed);
}

template <typename ElementsType>
void WaitFreeQueue<ElementsType>::StopReading()
{
    m_bIsReadingEnabled.store(false, std::memory_order_relaxed);
}

template <typename ElementsType>
void WaitFreeQueue<ElementsType>::UpdateIndex(unsigned int &p_iPosition)
{
    unsigned int uiNextPosition = p_iPosition + 1;
    if (uiNextPosition >= m_uiMaxBufferSize)
    {
        p_iPosition = 0;
    }
    else
    {
        ++p_iPosition;
    }
}