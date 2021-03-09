#include "obps_log.hpp"

#include <iostream>

using Log = obps::Log;
bool TestStdOut()
{
    auto logger = Log::Create(std::cerr, obps::LogLevel::WARN);
    logger->Write(obps::LogLevel::WARN, "[TestStdOut] Important Log Message");
    return true;
}

bool TestNotOpened()
{
    try 
    {
        auto bad_logger = Log::Create("//", obps::LogLevel::WARN);
    }
    catch ( std::runtime_error& e)
    {
        auto good_logger = Log::Create(std::cout, obps::LogLevel::INFO);
        good_logger->Write(obps::LogLevel::INFO, "[TestNotOpened]::PASS");
    }
    return true;
}

bool TestMultiple()
{
    auto logger = Log::Create(std::cout, obps::LogLevel::WARN);
    auto secondary = Log::Create(std::cerr, obps::LogLevel::WARN);

    logger->Attach(secondary);

    logger->Write(obps::LogLevel::WARN, "[TestMultiple] Writing Two Targets!");
    return true;
}

bool TestSelfAttachment()
{
    auto logger = Log::Create(std::cout, obps::LogLevel::DEBUG);

    try
    {
        logger->Attach(logger);
    }
    catch(std::logic_error& e)
    {
        logger->Write(obps::LogLevel::DEBUG, "[TestSelfAttachment]::PASS");
        return true;
    }

    logger->Write(obps::LogLevel::DEBUG, "[TestSelfAttachment]::FAIL");
    return false;
}

bool TestCyclicAttachment()
{
    auto logger = Log::Create(std::cout, obps::LogLevel::DEBUG);
    auto secondary = Log::Create(std::cerr, obps::LogLevel::DEBUG);

    logger->Attach(secondary);

    try
    {
        secondary->Attach(logger);
    }
    catch(std::logic_error& e)
    {
        secondary->Write(obps::LogLevel::DEBUG, "[TestCyclicAttachment]::PASS");
        return true;
    }

    secondary->Write(obps::LogLevel::DEBUG, "[TestCyclicAttachment]::FAIL");
    return false;
}

bool TestConcurrent()
{
    auto logger = Log::Create("concurrent", obps::LogLevel::WARN);
    std::thread ts[16];
    for (auto& t : ts)
    {
        t = std::thread([logger]{
            for (int i = 0; i < 100; ++i)
            {
                logger->Write(obps::LogLevel::ERROR, "[TestConcurrent] i = ", i);
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
