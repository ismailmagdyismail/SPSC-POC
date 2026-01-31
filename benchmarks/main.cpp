#include <iostream>
#include <cassert>
#include <thread>

#include "SPSCLockFreeQueue.h"

void ProducerThread(SPSCLockFreeQueue<long long> *p_pSPSCQueue, int p_iMaxElement)
{
    for (int i = 1; i <= p_iMaxElement; ++i)
    {
        // Simulate heavy CPU task
        long long dummy = 0;
        for (int j = 0; j < 10000; ++j)
        {
            dummy += j * j;
            dummy %= 1000000;
        }
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
            // Simulate heavy CPU task
            long long dummy = 0;
            for (int j = 0; j < 10000; ++j)
            {
                dummy += j * j;
                dummy %= 1000000;
            }
            total += *val;
            delete val;
        }
    }
    *p_llTotal = total;
}

void RunSummationTC(int p_iMaxElement, int p_iMaxQueueSize)
{
    const long long expectedSum = (p_iMaxElement * (p_iMaxElement + 1)) / 2LL;

    SPSCLockFreeQueue<long long> oSPSCQueue(p_iMaxQueueSize);
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


#include <benchmark/benchmark.h>
void BM_SPSCConsumption(benchmark::State& state)
{
    int iMaxElementsToSumTo = static_cast<int>(state.range(0));
    int iMaxQueueSize = static_cast<int>(state.range(1));
    const long long expectedSum = (iMaxElementsToSumTo * (iMaxElementsToSumTo + 1)) / 2LL;
    for (auto _ : state)
    {
        state.PauseTiming();
        SPSCLockFreeQueue<long long> oSPSCQueue(iMaxQueueSize);
        long long total = 0;
        std::thread producer = std::thread(ProducerThread, &oSPSCQueue, iMaxElementsToSumTo);
        std::thread consumer = std::thread(ConsumerThread, &oSPSCQueue, &total, expectedSum);
        state.ResumeTiming();
        producer.join();
        consumer.join();
        benchmark::DoNotOptimize(total);
    }
}


#include <mutex>
#include <queue>

void BM_MutexConsumption(benchmark::State& state)
{
    int iMaxElementsToSumTo = static_cast<int>(state.range(0));
    int iMaxQueueSize = static_cast<int>(state.range(1));
    const long long expectedSum = (iMaxElementsToSumTo * (iMaxElementsToSumTo + 1)) / 2LL;
    for (auto _ : state)
    {
        long long total = 0;
        state.PauseTiming();
        std::mutex g_mutex;
        std::queue<long long*> g_queue;
        std::thread producer = std::thread([&](){
            for (int i = 1; i <= iMaxElementsToSumTo; ++i)
            {
                long long dummy = 0;
                for (int j = 0; j < 10000; ++j)
                {
                    dummy += j * j;
                    dummy %= 1000000;
                }
                std::lock_guard<std::mutex> lock(g_mutex);
                g_queue.push(new long long(static_cast<long long>(i)));
            }
        });
        std::thread consumer = std::thread([&](){
            long long total = 0;
            while (total != expectedSum)
            {
                long long *val = nullptr;
                {
                    std::lock_guard<std::mutex> lock(g_mutex);
                    if (g_queue.empty())
                    {
                        continue;
                    }
                    val = g_queue.front();
                    g_queue.pop();
                }
                if(!val)
                {
                    continue;
                }
                long long dummy = 0;
                for (int j = 0; j < 10000; ++j)
                {
                    dummy += j * j;
                    dummy %= 1000000;
                }
                total += *val;
                delete val;
            }
        });
        state.ResumeTiming();
        producer.join();
        consumer.join();
        benchmark::DoNotOptimize(total);
    }
}


BENCHMARK(BM_SPSCConsumption)->Ranges({{100, 10000}, {10, 1000}});
BENCHMARK(BM_MutexConsumption)->Ranges({{100, 10000}, {10, 1000}});
BENCHMARK_MAIN();


// int main()
// {
//     std::vector<std::pair<int, int>> TCs = {{10, 2}, {100, 100}, {212, 2}, {1000, 2000}, {2000, 50000}, {10, 9}};
//     // std::vector<std::pair<int, int>> TCs = {{2, 1}}; // Will throw, Cannot Create SPSCLFQueue with size <= 1 .
//     int iTCsCount = TCs.size();
//     for (int i = 0; i < iTCsCount; ++i)
//     {
//         int iMaxElementsToSumTo = TCs[i].first;
//         int iMaxQueueSize = TCs[i].second;
//         RunSummationTC(iMaxElementsToSumTo, iMaxQueueSize);
//     }
// }