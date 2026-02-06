#include <iostream>
#include <cassert>

#include "WaitFreeQueue.h"
#include "SPSCLockBasedQueue.h"

template <template <typename> class QueueType>
void ProducerThread(QueueType<long long> *p_pWaitFreeQueue, int p_iMaxElement)
{
    for (int i = 1; i <= p_iMaxElement; ++i)
    {
        p_pWaitFreeQueue->Push(new long long(static_cast<long long>(i)));
    }
}

template <template <typename> class QueueType>
void ConsumerThread(QueueType<long long> *p_pWaitFreeQueue, long long *p_llTotal, long long p_llExpected)
{
    long long total = 0;
    while (p_llExpected > total)
    {
        long long *val = p_pWaitFreeQueue->Pop();
        if (val)
        {
            total += *val;
        }
        else
        {
            *p_llTotal = total;
            break;
        }
    }
    *p_llTotal = total;
}

template <template <typename> class QueueType>
void RunSummationTC(int p_iMaxElement, int p_iMaxQueueSize)
{
    const long long expectedSum = (p_iMaxElement * (p_iMaxElement + 1)) / 2LL;

    QueueType<long long> oWaitFreeQueue(p_iMaxQueueSize);
    long long total = 0;
    std::thread producer(ProducerThread<QueueType>, &oWaitFreeQueue, p_iMaxElement);
    std::thread consumer(ConsumerThread<QueueType>, &oWaitFreeQueue, &total, expectedSum);

    producer.join();
    consumer.join();

    std::cout << "Expected Sum = " << expectedSum << std::endl;
    std::cout << "Actual Sum = " << total << std::endl;
    assert(total == expectedSum);
    std::cout << "Success" << std::endl;
}

int main()
{
    std::vector<std::pair<int, int>> TCs = {{10, 2}, {100, 100}, {212, 2}, {1000, 2000}, {2000, 50000}, {10, 9}};
    int iTCsCount = TCs.size();
    for (int i = 0; i < iTCsCount; ++i)
    {
        int iMaxElementsToSumTo = TCs[i].first;
        int iMaxQueueSize = TCs[i].second;
        RunSummationTC<WaitFreeQueue>(iMaxElementsToSumTo, iMaxQueueSize);
    }
    for (int i = 0; i < iTCsCount; ++i)
    {
        int iMaxElementsToSumTo = TCs[i].first;
        int iMaxQueueSize = TCs[i].second;
        RunSummationTC<SPSCLockBasedQueue>(iMaxElementsToSumTo, iMaxQueueSize);
    }
}