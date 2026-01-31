#include <iostream>
#include <cassert>

#include "SPSCLockFreeQueue.h"

void ProducerThread(SPSCLockFreeQueue<long long> *p_pSPSCQueue, int p_iMaxElement)
{
    for (int i = 1; i <= p_iMaxElement; ++i)
    {
        p_pSPSCQueue->Push(new long long(static_cast<long long>(i)));
    }
}

void ConsumerThread(SPSCLockFreeQueue<long long> *p_pSPSCQueue, long long *p_llTotal, long long p_llExpected)
{
    long long total = 0;
    while (p_llExpected != total)
    {
        long long *val = p_pSPSCQueue->Pop();
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

void RunSummationTC(int p_iMaxElement)
{
    const long long expectedSum = (p_iMaxElement * (p_iMaxElement + 1)) / 2LL;

    SPSCLockFreeQueue<long long> oSPSCQueue(2);
    long long total = 0;
    std::thread producer = std::thread(ProducerThread, &oSPSCQueue, p_iMaxElement);
    std::thread consumer = std::thread(ConsumerThread, &oSPSCQueue, &total, expectedSum);

    producer.join();
    consumer.join();

    std::cout << "Expected Sum = " << expectedSum << std::endl;
    std::cout << "Actual Sum = " << total << std::endl;
    assert(total == expectedSum);
    std::cout << "Success" << std::endl;
}

int main()
{
    std::vector<int> TCs = {10, 100, 212, 1000, 2000};
    int iTCsCount = TCs.size();
    for (int i = 0; i < iTCsCount; ++i)
    {
        RunSummationTC(TCs[i]);
    }
}