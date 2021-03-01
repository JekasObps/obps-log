#include "obps_log.hpp"

#include <iostream>

using Log = obps::ObpsLog;
bool TestStdOut()
{
    auto logger = Log::CreateLog(std::cerr, obps::LogLevel::WARN);
    logger->Log(obps::LogLevel::WARN, "[TestStdOut] Important Log Message");
    return true;
}

bool TestNotOpened()
{
    try 
    {
        auto bad_logger = Log::CreateLog("|", obps::LogLevel::WARN);
    }
    catch ( std::runtime_error& e)
    {
        auto good_logger = Log::CreateLog(std::cout, obps::LogLevel::INFO);
        good_logger->Log(obps::LogLevel::INFO, "[TestNotOpened]::PASS");
    }
    return true;
}

bool TestMultiple()
{
    auto logger = Log::CreateLog(std::cout, obps::LogLevel::WARN);
    auto secondary = Log::CreateLog(std::cerr, obps::LogLevel::WARN);

    logger->Attach(secondary);

    logger->Log(obps::LogLevel::WARN, "[TestMultiple] Writing Two Targets!");
    return true;
}

bool TestSelfAttachment()
{
    auto logger = Log::CreateLog(std::cout, obps::LogLevel::DEBUG);

    try
    {
        logger->Attach(logger);
    }
    catch(std::logic_error& e)
    {
        logger->Log(obps::LogLevel::DEBUG, "[TestSelfAttachment]::PASS");
        return true;
    }

    logger->Log(obps::LogLevel::DEBUG, "[TestSelfAttachment]::FAIL");
    return false;
}

bool TestCyclicAttachment()
{
    auto logger = Log::CreateLog(std::cout, obps::LogLevel::DEBUG);
    auto secondary = Log::CreateLog(std::cerr, obps::LogLevel::DEBUG);

    logger->Attach(secondary);

    try
    {
        secondary->Attach(logger);
    }
    catch(std::logic_error& e)
    {
        secondary->Log(obps::LogLevel::DEBUG, "[TestCyclicAttachment]::PASS");
        return true;
    }

    secondary->Log(obps::LogLevel::DEBUG, "[TestCyclicAttachment]::FAIL");
    return false;
}

bool TestConcurrent()
{
    auto logger = Log::CreateLog("concurrent", obps::LogLevel::WARN);
    std::thread ts[16];
    for (auto& t : ts)
    {
        t = std::thread([logger]{
            for (int i = 0; i < 100; ++i)
            {
                logger->Log(obps::LogLevel::ERROR, "[TestConcurrent] i = ", i);
            }
        });
    }

    for (auto& t : ts)
    {
        t.join();
    }
    return true;
}

int main()
{
    TestStdOut();
    TestNotOpened();
    TestMultiple();
    TestSelfAttachment();
    TestCyclicAttachment();
    TestConcurrent();
}
