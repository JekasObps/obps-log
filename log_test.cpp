#include "obps_log.hpp"

#include <iostream>

using Log = obps::Log;
using LogLevel = obps::LogLevel;
using LogSpecs = obps::Log::LogSpecs;

/*
* ----------------- Benchmarking ------------------
*/
#include <chrono>
#define BENCHMARK(expr) do {                                    \
    std::cerr << "Running Benchmark: " << __FUNCTION__ << "...\n";\
    using hrez_clock = std::chrono::high_resolution_clock;      \
    auto start = hrez_clock::now();                             \
    {expr};                                                     \
    auto end = hrez_clock::now();                               \
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);\
    std::cerr << __FUNCTION__ << " : " << elapsed.count() << "ms\n";\
} while(0)

//
//-------------------------------------------------
//

bool TestStdOut()
{
    Log::LogSpecs specs;
    specs
        .Target(std::cerr)
        .Level (obps::LogLevel::WARN)
        .Format(Log::default_format);

    auto logger = Log(specs);
    logger.Write(obps::LogLevel::WARN, "[TestStdOut] Important Log Message");
    return true;
}

bool TestNotOpened()
{
    try
    {
        auto bad_logger = Log({"//", LogLevel::WARN});
    }
    catch ( std::runtime_error& e)
    {
        auto good_logger = Log({std::cout, obps::LogLevel::INFO});
        good_logger.Write(obps::LogLevel::INFO, "[TestNotOpened]::PASS");
    }
    return true;
}

bool TestMultiple()
{
    auto logger = Log({std::cout, obps::LogLevel::WARN});
    auto secondary = Log({std::cerr, obps::LogLevel::WARN});

    logger.Attach(secondary);

    logger.Write(obps::LogLevel::WARN, "[TestMultiple] Writing Two Targets!");
    return true;
}

bool TestSelfAttachment()
{
    auto logger = Log({std::cout, obps::LogLevel::DEBUG});

    try
    {
        logger.Attach(logger);
    }
    catch(std::logic_error& e)
    {
        logger.Write(obps::LogLevel::DEBUG, "[TestSelfAttachment]::PASS");
        return true;
    }

    logger.Write(obps::LogLevel::DEBUG, "[TestSelfAttachment]::FAIL");
    return false;
}

bool TestCyclicAttachment()
{
    auto logger = Log({std::cout, obps::LogLevel::DEBUG});
    auto secondary = Log({std::cerr, obps::LogLevel::DEBUG});

    logger.Attach(secondary);

    try
    {
        secondary.Attach(logger);
    }
    catch(std::logic_error& e)
    {
        secondary.Write(obps::LogLevel::DEBUG, "[TestCyclicAttachment]::PASS");
        return true;
    }

    secondary.Write(obps::LogLevel::DEBUG, "[TestCyclicAttachment]::FAIL");
    return false;
}

bool TestConcurrent()
{
    auto logger = Log({"concurrent", obps::LogLevel::WARN});
    std::thread ts[16];
    for (auto& t : ts)
    {
        t = std::thread([&logger]{
            for (int i = 0; i < 100; ++i)
            {
                logger.Write(obps::LogLevel::ERROR, "[TestConcurrent] i = ", i);
            }
        });
    }

    for (auto& t : ts)
    {
        t.join();
    }
    return true;
}

bool TestSorting()
{ BENCHMARK(
        auto logA = Log({"logA", obps::LogLevel::DEBUG});
        auto logB = Log({"logB", obps::LogLevel::INFO});
        auto logC = Log({"logC", obps::LogLevel::WARN});
        auto logD = Log({"logD", obps::LogLevel::ERROR});

        logA
        . Attach(logB)
        . Attach(logC)
        . Attach(logD);

        logA.Write(obps::LogLevel::DEBUG, "MESSAGE 1\n");
        logA.Write(obps::LogLevel::INFO,  "MESSAGE 2\n");
        logA.Write(obps::LogLevel::WARN,  "MESSAGE 3\n");
        logA.Write(obps::LogLevel::ERROR, "MESSAGE 4\n");
    );
    return true;
}

#include <chrono>
bool TestThreadException()
{
    const char* filename = "out1.log";
    std::ofstream out(filename);

    out.exceptions (std::ofstream::badbit | std::ofstream::failbit);
    auto logger = Log({out, obps::LogLevel::DEBUG});
    logger.Write(obps::LogLevel::DEBUG, "Ok!");
    out.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    try 
    { // catching error in debug mode
        logger.Write(obps::LogLevel::DEBUG, "Error!");
    }
    catch (std::exception &e)
    {
        std::cerr << "Expected: (" << e.what() << ")\n";
        return true;
    }
    
    return false;
}

int main()
{
    BENCHMARK(
        TestStdOut();
        TestNotOpened();
        TestMultiple();
        TestSelfAttachment();
        TestCyclicAttachment();
        TestConcurrent();
        TestSorting();
        TestThreadException();
    );
}
