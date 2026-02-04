#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

/*
NOTE: cost vaires from ARM to x86
//! ARM atomics and locks has a higher cost than x86
TODO: use google benchmark instead
*/

std::mutex gMutex;
int iCounter = 0;
std::atomic<int> atomicCounter = 0;
int MAX_SUM = 100000000;
bool bIsStarted = false;

//! RACE_Condition, I know shut up!.
//! I am testing Cach-ping pong / Cachlines invalidation / false sharing (or actual sharing in this case).
//! To measure cost of the theoritcal optimal data sharing cost. (before locks and atomics).

void SequentialAtomic()
{
    while (!bIsStarted)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    while (atomicCounter < MAX_SUM)
    {
        atomicCounter++;
    }
}

void RelaxedAtomic()
{
    while (!bIsStarted)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    while (atomicCounter.load(std::memory_order_relaxed) < MAX_SUM)
    {
        atomicCounter.fetch_add(1, std::memory_order_relaxed);
    }
}

void MutexBased()
{
    while (!bIsStarted)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    while (iCounter < MAX_SUM)
    {
        std::lock_guard<std::mutex> oLock{gMutex};
        ++iCounter;
    }
}

void NonAtomic()
{
    while (!bIsStarted)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    while (iCounter < MAX_SUM)
    {
        ++iCounter;
    }
}

int main(int argc, char **argv)
{
    int iNumberOfThreads{0};
    int iTCToUse = 1;
    if (argc != 3)
    {
        std::cerr << "[Usage]: ./main.exe \n\
        <Number_Of_Threads>\n\
        <Benchmark_To_Use>\n\
        \t 1-Atomic-Sequential\n\
        \t 2-Atomic-Relaxed\n\
        \t 3-Mutex\n\
        \t 4-Non-Atomic\n\
        ";
        return 1;
    }
    try
    {
        iNumberOfThreads = std::stoi(argv[1]);
        iTCToUse = std::stoi(argv[2]);
    }
    catch (std::exception &p_eException)
    {
        std::cerr << p_eException.what();
        return 1;
    }

    std::function<void(void)> benchmarkHandler;
    std::string benchmarkName;
    switch (iTCToUse)
    {
    case 1:
        benchmarkHandler = []()
        { SequentialAtomic(); };
        benchmarkName = "Atomic-Sequential";
        break;
    case 2:
        benchmarkHandler = []()
        { RelaxedAtomic(); };
        benchmarkName = "Atomic-Relaxed";
        break;
    case 3:
        benchmarkHandler = []()
        { MutexBased(); };
        benchmarkName = "Mutex";
        break;
    case 4:
        benchmarkHandler = []()
        { NonAtomic(); };
        benchmarkName = "Non-Atomic";
        break;

    default:
        std::cerr << "Invalid benchmarking name " << std::endl;
        return 0;
        break;
    }

    std::cerr << "Current Machine's Available Threads = " << std::thread::hardware_concurrency() << std::endl;
    std::cerr << "Number of threads to Use in Benchmarking = " << iNumberOfThreads << std::endl;
    std::cerr << "Bencmarking Type = " << benchmarkName << std::endl;

    std::vector<std::thread> vecThreads;
    for (int i = 0; i < iNumberOfThreads; ++i)
    {
        vecThreads.push_back(std::thread(benchmarkHandler));
    }
    bIsStarted = true;
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iNumberOfThreads; ++i)
    {
        if (vecThreads[i].joinable())
        {
            vecThreads[i].join();
        }
    }
    bIsStarted = false;
    auto end = std::chrono::steady_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cerr << "TIME" << ":\n"
              << "  " << ns << " ns\n"
              << "  " << us << " us\n"
              << "  " << ms << " ms\n";

    std::cerr << "Non-Atomic-Counter " << iCounter << std::endl;
    std::cerr << "Atomic-Counter " << atomicCounter << std::endl;
}