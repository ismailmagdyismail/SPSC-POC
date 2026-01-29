#include <iostream>
#include <thread>
#include <atomic>
#include <cassert>
#include <fstream>

static unsigned long long uiMaxValueToProduce = 10000000;
static unsigned int testsCount = 1;

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

void ConsumerAcquire2(unsigned long long *p_arr, unsigned long long p_uiMaxValueToProduce, std::atomic<unsigned long long> &p_uiIndex, unsigned long long &p_uiFinalOutput)
{
    unsigned long long totalSum = 0;
    unsigned long long uiCurrentIndex = 0;
    unsigned long long uiLastProcessedIndex = 0;
    while (uiCurrentIndex < p_uiMaxValueToProduce)
    {
        while ((uiCurrentIndex = p_uiIndex.load(std::memory_order_acquire)) <= uiLastProcessedIndex)
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(10));
        }
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

unsigned long long AcquireRelease2()
{
    unsigned long long *arr = new unsigned long long[uiMaxValueToProduce];
    std::atomic<unsigned long long> uiIndex = 0;
    unsigned long long uiResult = 0;
    std::thread producerThread = std::thread(ProducerRelease, arr, uiMaxValueToProduce, std::ref(uiIndex));
    std::thread consumerThread = std::thread(ConsumerAcquire2, arr, uiMaxValueToProduce, std::ref(uiIndex), std::ref(uiResult));

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
    for (unsigned int i = 0; i < testsCount; ++i)
    {
        unsigned long long uiThreadedResult = AcquireRelease();
        assert(uiThreadedResult == uiSingleThread);
    }
    std::cout << "All Acquire-Release Tests Passed" << std::endl;
}

void TestAcquireRelease2()
{
    unsigned long long uiSingleThread = SingleThread();
    for (unsigned int i = 0; i < testsCount; ++i)
    {
        unsigned long long uiThreadedResult = AcquireRelease2();
        assert(uiThreadedResult == uiSingleThread);
    }
    std::cout << "All Acquire-Release2 Tests Passed" << std::endl;
}

void ProducerMutex(unsigned long long *p_arr, unsigned long long p_uiMaxValueToProduce, unsigned long long &p_uiIndex, std::mutex &p_oMutex)
{
    unsigned long long valueToProduce = 1;
    while (valueToProduce <= p_uiMaxValueToProduce)
    {
        p_arr[p_uiIndex] = valueToProduce;
        {
            std::lock_guard<std::mutex> oLock(p_oMutex);
            p_uiIndex++;
        }
        valueToProduce++;
    }
}

void ConsumerMutex(unsigned long long *p_arr, unsigned long long p_uiMaxValueToProduce, unsigned long long &p_uiIndex, std::mutex &p_oMutex, unsigned long long &p_uiFinalOutput)
{
    unsigned long long totalSum = 0;
    unsigned long long uiCurrentIndex = 0;
    unsigned long long uiLastProcessedIndex = 0;
    while (uiCurrentIndex < p_uiMaxValueToProduce)
    {
        {
            std::lock_guard<std::mutex> oLock(p_oMutex);
            uiCurrentIndex = p_uiIndex;
        }
        for (unsigned long long i = uiLastProcessedIndex; i < uiCurrentIndex; ++i)
        {
            totalSum += p_arr[i];
        }
        uiLastProcessedIndex = uiCurrentIndex;
    }
    p_uiFinalOutput = totalSum;
}

unsigned long long MutexProducerConsumer()
{
    unsigned long long *arr = new unsigned long long[uiMaxValueToProduce];
    unsigned long long uiIndex = 0;
    unsigned long long uiResult = 0;
    std::mutex oMutex;
    std::thread producerThread = std::thread(ProducerMutex, arr, uiMaxValueToProduce, std::ref(uiIndex), std::ref(oMutex));
    std::thread consumerThread = std::thread(ConsumerMutex, arr, uiMaxValueToProduce, std::ref(uiIndex), std::ref(oMutex), std::ref(uiResult));

    producerThread.join();
    consumerThread.join();

    delete[] arr;
    return uiResult;
}

void TestMutex()
{
    unsigned long long uiSingleThread = SingleThread();
    for (unsigned int i = 0; i < testsCount; ++i)
    {
        unsigned long long uiThreadedResult = MutexProducerConsumer();
        assert(uiThreadedResult == uiSingleThread);
    }
    std::cout << "All Mutex Tests Passed" << std::endl;
}

//! GPT-Generated
template <typename Func>
auto TimeFunction(Func &&f, const char *name)
{
    using clock = std::chrono::steady_clock;

    auto start = clock::now();
    auto result = f();
    auto end = clock::now();

    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << name << ":\n"
              << "  " << ns << " ns\n"
              << "  " << us << " us\n"
              << "  " << ms << " ms\n";

    return result;
}

int main()
{
    std::thread t1 = std::thread(
        []()
        {
            TestAcquireRelease();
        });

    std::thread t2 = std::thread(
        []()
        {
            TestMutex();
        });

    std::thread t3 = std::thread(
        []()
        {
            TestAcquireRelease2();
        });

    t1.join();
    t2.join();
    t3.join();

    TimeFunction(AcquireRelease, "AcquireRelease");
    TimeFunction(AcquireRelease2, "AcquireRelease2");
    TimeFunction(MutexProducerConsumer, "MutexProducerConsumer");
}