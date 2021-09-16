#include <gtest/gtest.h>

#include "obps_log_public.hpp"

// GLOBAL_LOG({LogLevel::ERROR, std::cerr});

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using ::testing::MatchesRegex;

#include "obps_log_public.hpp"

#include <thread>

#include <sstream>
#include <iostream>

#include <filesystem>
namespace fs = std::filesystem;

#include <chrono>
using namespace std::chrono_literals;


class TestLog : public ::testing::Test
{
protected:
    std::stringstream out, err;
    std::string message, word;
    const fs::path cwd = std::filesystem::current_path();

    static void TearDownTestSuite()
    {
        OBPS_LOG_TEARDOWN();
    }
};

TEST_F(TestLog, TestSingleError)
{
    SCOPE_LOG({LogLevel::ERROR, out});

    int num = 42;
    ERROR("num == ", num, " is a prohibitted value!");

    std::this_thread::sleep_for(1ms); // make sure that thread completed work
     
    message.assign(std::istreambuf_iterator<char>(out), std::istreambuf_iterator<char>());

    EXPECT_THAT(message, MatchesRegex("^.*ERROR num == 42 is a prohibitted value!\n$"));
}


TEST_F(TestLog, TestLevels)
{
    SCOPE_LOG({LogLevel::INFO, out});

    DEBUG("some debug message!");   // ignored
    INFO("some info message!");     // ok
    WARN("some warning message!");  // ok
    ERROR("some error message!");   // ok

    std::this_thread::sleep_for(1ms); // make sure that thread completed work
     
    message.assign(std::istreambuf_iterator<char>(out), std::istreambuf_iterator<char>());

    EXPECT_THAT(message, MatchesRegex(".*INFO some info message!\n.*WARN some warning message!\n.*ERROR some error message!\n"));
}


TEST_F(TestLog, TestMultipleTargers)
{
    SCOPE_LOG({LogLevel::DEBUG, out}, 
              {LogLevel::WARN, err});

    DEBUG("some debug message!");   // out
    INFO("some info message!");     // out
    WARN("some warning message!");  // out & err
    ERROR("some error message!");   // out & err

    std::this_thread::sleep_for(1000ms); // make sure that thread completed work
     
    message.assign(std::istreambuf_iterator<char>(out), std::istreambuf_iterator<char>());

    EXPECT_THAT(message, MatchesRegex(".*DEBUG some debug message!\n.*INFO some info message!\n.*WARN some warning message!\n.*ERROR some error message!\n"));

    message.assign(std::istreambuf_iterator<char>(err), std::istreambuf_iterator<char>());

    EXPECT_THAT(message, MatchesRegex(".*WARN some warning message!\n.*ERROR some error message!\n"));
}