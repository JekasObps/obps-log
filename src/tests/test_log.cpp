#include <gtest/gtest.h>

#include "obps_log_public.hpp"

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
    /////
    // Streams required to have a static lifetime duration !!! 
    //      stream that had been used by log and destroyed before log shutdown
    //      results in undefined behaviour!
    /////
    static std::stringstream out, err;

    std::string message, word;
    const fs::path cwd = std::filesystem::current_path();
    const fs::path logdir = cwd / "logs";
    const fs::path logname = "logfile";
    const fs::path expected_log_path = logdir / obps::make_log_filename(logname.string());

    void SetUp() override
    {
        // cleaning up streams before every test case 
        out.clear();
        err.clear();
    }

    static void TearDownTestSuite()
    {
        // teardown log, this happens globally and can't be reverted, 
        //  so only single test suite is allowed!
        OBPS_LOG_TEARDOWN();
    }
};

// initialization of static streams
std::stringstream TestLog::out, TestLog::err;


TEST_F(TestLog, TestSingleError)
{
    SCOPE_LOG({LogLevel::ERROR, out});

    int num = 42;
    ERROR("num == ", num, " is a prohibitted value!");

    std::this_thread::sleep_for(10ms); // make sure that thread completed work
     
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

    std::this_thread::sleep_for(10ms); // make sure that thread completed work
     
    message.assign(std::istreambuf_iterator<char>(out), std::istreambuf_iterator<char>());

    EXPECT_THAT(message, MatchesRegex(".*INFO some info message!\n.*WARN some warning message!\n.*ERROR some error message!\n"));
}


TEST_F(TestLog, TestMultipleTargers)
{
    auto q_uid = obps::LogRegistry::GenerateQueueUid();

    SCOPE_LOG({LogLevel::DEBUG, out}, // creates queue with default_queue_size and unique id: q_<hex>
              {LogLevel::WARN, err, 10, q_uid}); // example of creating queue with custom size and manual queue_uid generation for later use.

    DEBUG("some debug message!");   // out
    INFO("some info message!");     // out
    WARN("some warning message!");  // out & err
    ERROR("some error message!");   // out & err

    std::this_thread::sleep_for(10ms); // make sure that thread completed work
     
    message.assign(std::istreambuf_iterator<char>(out), std::istreambuf_iterator<char>());

    EXPECT_THAT(message, MatchesRegex(".*DEBUG some debug message!\n.*INFO some info message!\n.*WARN some warning message!\n.*ERROR some error message!\n"));

    message.assign(std::istreambuf_iterator<char>(err), std::istreambuf_iterator<char>());

    EXPECT_THAT(message, MatchesRegex(".*WARN some warning message!\n.*ERROR some error message!\n"));
}


TEST_F(TestLog, TestFileTarget)
{
    fs::create_directory(logdir);  // prepare directory on user side
    fs::remove(expected_log_path); // clean test

    SCOPE_LOG({LogLevel::DEBUG, logdir / logname});

    ASSERT_TRUE(fs::exists(expected_log_path));

    DEBUG_SYNC("some debug message!");   // out

    std::this_thread::sleep_for(10ms); // make sure that thread completed work
     
    std::fstream log_file_in(expected_log_path);
    
    message.assign(std::istreambuf_iterator<char>(log_file_in), std::istreambuf_iterator<char>());

    EXPECT_THAT(message, MatchesRegex(".*DEBUG some debug message!\n"));
}