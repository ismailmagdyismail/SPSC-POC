#include "SPSCLockFreeQueue.h"

template <typename ElementsDataType>
SPSCLockFreeQueue<ElementsDataType>::SPSCLockFreeQueue(int p_tMaxBufferSize)
{
    m_iMaxBufferSize = p_tMaxBufferSize;
    m_arrBuffer = new ElementsDataType *[m_iMaxBufferSize];
}

template <typename ElementsDataType>
SPSCLockFreeQueue<ElementsDataType>::~SPSCLockFreeQueue()
{
    while (HasElementsToRead())
    {
        int iReadPosition = GetNextReadPosition();
        delete m_arrBuffer[iReadPosition];
        m_iReadPosition.store(iReadPosition, std::memory_order_relaxed);
    }
    delete[] m_arrBuffer;
}
template <typename ElementsDataType>
void SPSCLockFreeQueue<ElementsDataType>::StopWriting()
{
    m_bIsWritingEnabled.store(false, std::memory_order_relaxed);
}

template <typename ElementsDataType>
void SPSCLockFreeQueue<ElementsDataType>::StopReading()
{
    m_bIsReadingEnabled.store(false, std::memory_order_relaxed);
}

template <typename ElementsDataType>
ElementsDataType *SPSCLockFreeQueue<ElementsDataType>::Pop()
{
    //! 1. Checking Space
    //! [Visibility & Memory order]:
    //! - Correct WritePosition to calculate space
    //! - Correct ReadPosition to calculate space
    //! - Don't Care about buffer elements actual initilization here
    while (!HasElementsToRead() && m_bIsReadingEnabled.load(std::memory_order_relaxed))
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
    if (!m_bIsReadingEnabled.load(std::memory_order_relaxed))
    {
        return nullptr;
    }

    //! 2. Getting element from an Buffer
    //! [Visibility & Memory order]:
    //! - Correct WritePosition to
    //! - Correct Buffer Elemets Addition and initilization
    //! Prevent Consuming / Reading from being moved backwards in time, due to memory read barrier; gauranteed initialzed elements
    m_iWritePosition.load(std::memory_order_acquire); //! Syncing / Handshaking (All initializtion, Writes by producer are now visible)
    int iNextReadPosition = GetNextReadPosition();    //! Since single consumer, we can guarantee this has not changed from previous read
    ElementsDataType *pElementToRead = m_arrBuffer[iNextReadPosition];

    //! 3. Updating Read Position
    //! [Visibility & Memory order]:
    //! - Don't Depend on producer at all
    //! - Don't need ordering at producer side, when he tries to see ReadPosition, just transactional ReadPosition, we don't care about actual processing of element
    m_iReadPosition.store(iNextReadPosition, std::memory_order_relaxed);

    return pElementToRead;
}

template <typename ElementsDataType>
void SPSCLockFreeQueue<ElementsDataType>::Push(ElementsDataType *p_pElement)
{
    //! 1. Checking Space
    //! [Visibility & Memory order]:
    //! - Correct ReadPosition to calculate space
    //! - Correct WritePosition to calculate space
    //! WE Don't care about what happened to array elements that are already consumed, only ReadPosition
    while (!HasSpaceToWrite() && m_bIsWritingEnabled.load(std::memory_order_relaxed))
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
    if (!m_bIsWritingEnabled.load(std::memory_order_relaxed))
    {
        return;
    }

    //! 2. adding element into buffer
    //! [Visibility & Memory order]:
    //! - We don't care about any data from consumer at this point.
    //! ==========================================================
    //! - Can safely re-access write position here, in a non transactional manner, since only 1 producer
    //! - we can gaurantee that it didn't change from previous checks
    int iNextWritePosition = GetNextWritePosition();
    m_arrBuffer[iNextWritePosition] = p_pElement;

    //! 3. Publishing WritePosition Change
    m_iWritePosition.store(iNextWritePosition, std::memory_order_release); //! to be synced with on reading from same Atomic variable by consumer
}

template <typename ElementsDataType>
inline int SPSCLockFreeQueue<ElementsDataType>::GetNextReadPosition()
{
    return GetNextPosition(m_iReadPosition.load(std::memory_order_relaxed));
}

template <typename ElementsDataType>
inline int SPSCLockFreeQueue<ElementsDataType>::GetNextWritePosition()
{
    return GetNextPosition(m_iWritePosition.load(std::memory_order_relaxed));
}

template <typename ElementsDataType>
inline int SPSCLockFreeQueue<ElementsDataType>::GetLastWrittenPosition()
{
    return m_iWritePosition.load(std::memory_order_relaxed);
}

template <typename ElementsDataType>
inline int SPSCLockFreeQueue<ElementsDataType>::GetLastReadPosition()
{
    return m_iReadPosition.load(std::memory_order_relaxed);
}

template <typename ElementsDataType>
inline bool SPSCLockFreeQueue<ElementsDataType>::DidPositionWrap(int p_iNewPosition, int p_iOldPosition)
{
    return (p_iNewPosition < p_iOldPosition);
}

template <typename ElementsDataType>
inline bool SPSCLockFreeQueue<ElementsDataType>::HasElementsToRead()
{
    return GetLastReadPosition() != GetLastWrittenPosition();
}

template <typename ElementsDataType>
inline bool SPSCLockFreeQueue<ElementsDataType>::HasSpaceToWrite()
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
inline int SPSCLockFreeQueue<ElementsDataType>::GetNextPosition(int p_uiPosition)
{
    int uiNextPosition = p_uiPosition + 1;
    if (uiNextPosition >= m_iMaxBufferSize)
    {
        return 0;
    }
    return uiNextPosition;
}