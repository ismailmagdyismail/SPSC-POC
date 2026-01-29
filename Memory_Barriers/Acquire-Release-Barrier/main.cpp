#include <iostream>
#include <thread>
#include <atomic>
#include <cassert>
#include <fstream>

static unsigned long long uiMaxValueToProduce = 100000;

void ProducerRelease(unsigned long long *p_arr, unsigned long long p_uiMaxValueToProduce, std::atomic<unsigned long long> &p_uiIndex)
{
    unsigned long long valueToProduce = 1;
    while (valueToProduce <= p_uiMaxValueToProduce)
    {
        p_arr[p_uiIndex] = valueToProduce;
        p_uiIndex.fetch_add(1, std::memory_order::memory_order_release);
        valueToProduce++;
    }
}

void ConsumerAcquire(unsigned long long *p_arr, unsigned long long p_uiMaxValueToProduce, std::atomic<unsigned long long> &p_uiIndex, unsigned long long &p_uiFinalOutput)
{
    unsigned long long totalSum = 0;
    unsigned long long uiCurrentIndex = 0;
    unsigned long long uiLastProcessedIndex = 0;
    while (uiCurrentIndex < p_uiMaxValueToProduce)
    {
        uiCurrentIndex = p_uiIndex.load(std::memory_order_acquire);
        for (unsigned long long i = uiLastProcessedIndex; i < uiCurrentIndex; ++i)
        {
            totalSum += p_arr[i];
        }
        uiLastProcessedIndex = uiCurrentIndex;
    }
    p_uiFinalOutput = totalSum;
}

unsigned long long AcquireRelease()
{
    unsigned long long *arr = new unsigned long long[uiMaxValueToProduce];
    std::atomic<unsigned long long> uiIndex = 0;
    unsigned long long uiResult = 0;
    std::thread producerThread = std::thread(ProducerRelease, arr, uiMaxValueToProduce, std::ref(uiIndex));
    std::thread consumerThread = std::thread(ConsumerAcquire, arr, uiMaxValueToProduce, std::ref(uiIndex), std::ref(uiResult));

    producerThread.join();
    consumerThread.join();

    delete[] arr;
    return uiResult;
}

unsigned long long SingleThread()
{
    return ((uiMaxValueToProduce * (uiMaxValueToProduce + 1LL)) / 2LL);
}

void TestAcquireRelease()
{
    unsigned long long uiSingleThread = SingleThread();
    for (int i = 0; i < 1000; ++i)
    {
        unsigned long long uiThreadedResult = AcquireRelease();
        assert(uiThreadedResult == uiSingleThread);
    }
    std::cout << "All Tests Passed" << std::endl;
}

int main()
{
    TestAcquireRelease();
}