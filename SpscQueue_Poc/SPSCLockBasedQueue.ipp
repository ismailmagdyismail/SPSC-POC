#include "SPSCLockBasedQueue.h"

template <typename ElementsDataType>
SPSCLockBasedQueue<ElementsDataType>::SPSCLockBasedQueue(int p_tMaxBufferSize)
{
    if (p_tMaxBufferSize <= 1)
    {
        throw std::runtime_error("Cannot Create a SPSCLockBasedQueue with size <= 1");
    }
    m_iMaxBufferSize = p_tMaxBufferSize;
    m_arrBuffer = new ElementsDataType *[m_iMaxBufferSize];
}

template <typename ElementsDataType>
SPSCLockBasedQueue<ElementsDataType>::~SPSCLockBasedQueue()
{
    while (HasElementsToRead())
    {
        int iReadPosition = GetNextReadPosition();
        delete m_arrBuffer[iReadPosition];
        m_iReadPosition = iReadPosition;
    }
    delete[] m_arrBuffer;
}
template <typename ElementsDataType>
void SPSCLockBasedQueue<ElementsDataType>::StopWriting()
{
    std::lock_guard<std::mutex> oLock{m_oBufferMutex};
    m_bIsWritingEnabled = false;
}

template <typename ElementsDataType>
void SPSCLockBasedQueue<ElementsDataType>::StopReading()
{
    std::lock_guard<std::mutex> oLock{m_oBufferMutex};
    m_bIsReadingEnabled = false;
}

template <typename ElementsDataType>
ElementsDataType *SPSCLockBasedQueue<ElementsDataType>::Pop()
{
    ElementsDataType *pElement{nullptr};
    {
        std::unique_lock<std::mutex> oLock{m_oBufferMutex};
        m_oDataReadyCv.wait(oLock, [this]()
                            { return HasElementsToRead() || !m_bIsReadingEnabled; });
        if (!m_bIsReadingEnabled)
        {
            return nullptr;
        }
        int iNextReadIndex = GetNextReadPosition();
        pElement = m_arrBuffer[iNextReadIndex];
        m_iReadPosition = iNextReadIndex;
    }
    m_oSlotAvailableCv.notify_one();
    return pElement;
}

template <typename ElementsDataType>
void SPSCLockBasedQueue<ElementsDataType>::Push(ElementsDataType *p_pElement)
{
    {
        std::unique_lock<std::mutex> oLock{m_oBufferMutex};
        m_oSlotAvailableCv.wait(oLock, [this]()
                                { return HasSpaceToWrite() || !m_bIsWritingEnabled; });
        if (!m_bIsWritingEnabled)
        {
            return;
        }
        int iNextWritePosition = GetNextWritePosition();
        m_arrBuffer[iNextWritePosition] = p_pElement;
        m_iWritePosition = iNextWritePosition;
    }
    m_oDataReadyCv.notify_one();
}

template <typename ElementsDataType>
inline int SPSCLockBasedQueue<ElementsDataType>::GetNextReadPosition()
{
    return GetNextPosition(m_iReadPosition);
}

template <typename ElementsDataType>
inline int SPSCLockBasedQueue<ElementsDataType>::GetNextWritePosition()
{
    return GetNextPosition(m_iWritePosition);
}

template <typename ElementsDataType>
inline int SPSCLockBasedQueue<ElementsDataType>::GetLastWrittenPosition()
{
    return m_iWritePosition;
}

template <typename ElementsDataType>
inline int SPSCLockBasedQueue<ElementsDataType>::GetLastReadPosition()
{
    return m_iReadPosition;
}

template <typename ElementsDataType>
inline bool SPSCLockBasedQueue<ElementsDataType>::DidPositionWrap(int p_iNewPosition, int p_iOldPosition)
{
    return (p_iNewPosition < p_iOldPosition);
}

template <typename ElementsDataType>
inline bool SPSCLockBasedQueue<ElementsDataType>::HasElementsToRead()
{
    return GetLastReadPosition() != GetLastWrittenPosition();
}

template <typename ElementsDataType>
inline bool SPSCLockBasedQueue<ElementsDataType>::HasSpaceToWrite()
{
    int iNextWritePosition = GetNextWritePosition();
    int iLastReadPosition = GetLastReadPosition();
    bool bDidWriteWrapOver = DidPositionWrap(iNextWritePosition, GetLastWrittenPosition());
    if (bDidWriteWrapOver)
    {
        return iNextWritePosition < iLastReadPosition;
    }
    return iNextWritePosition > iLastReadPosition;
}

template <typename ElementsDataType>
inline int SPSCLockBasedQueue<ElementsDataType>::GetNextPosition(int p_uiPosition)
{
    int uiNextPosition = p_uiPosition + 1;
    if (uiNextPosition >= m_iMaxBufferSize)
    {
        return 0;
    }
    return uiNextPosition;
}