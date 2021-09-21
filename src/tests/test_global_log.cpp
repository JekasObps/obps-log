#include "gtest/gtest.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using ::testing::MatchesRegex;

#include "obps_log_public.hpp"

#include <chrono>
using namespace std::chrono_literals;

#include <thread>
#include <vector>

class TestGlobalLog : public ::testing::Test
{
protected:
    /////
    // Streams required to have a static lifetime duration !!! 
    //      stream that had been used by log and destroyed before log shutdown
    //      results in undefined behaviour!
    /////
    static std::stringstream out, err;

public:   
    // let's pretend that GLOBAL_LOG is actually global
    // this way Global Log will be constructed every Test from scratch, 
    // spawning additional thread 
    // and each target will be initialized with fresh queue 
    GLOBAL_LOG({LogLevel::INFO, out}, {LogLevel::ERROR, err});
protected:
    std::string message, word;

    void SetUp() override
    {
        std::this_thread::sleep_for(100ms);
        out.str("");
        out.clear();
        err.str("");
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
std::stringstream TestGlobalLog::out, TestGlobalLog::err;


TEST_F(TestGlobalLog, TestAPI)
{
    G_DEBUG("Nothing going to be printed!");
    G_WARN("Hot stuff!");
    G_INFO_SYNC("Syncing info...");

    G_ERROR("Alert! ", std::hex, 2319);
    G_MUTE(LogLevel::ERROR);
    G_ERROR("--- IGNORED ---");

    G_UNMUTE(LogLevel::WARN); // nothing will happen, it's fine to pass levels that not being muted
    G_UNMUTE(LogLevel::INFO, LogLevel::ERROR); // will only unmute error

    // make sure that thread completed work
    std::this_thread::sleep_for(100ms);

    message.assign((std::istreambuf_iterator<char>(err)), std::istreambuf_iterator<char>());
    EXPECT_THAT(message, MatchesRegex("^.*ERROR Alert! 90f\n$"));

    message.assign((std::istreambuf_iterator<char>(out)), std::istreambuf_iterator<char>());
    EXPECT_THAT(message, MatchesRegex(
        "^.*WARN Hot stuff!\n"
        ".*INFO Syncing info\\.\\.\\.\n"
        ".*ERROR Alert! 90f\n$")
    );
}


TEST_F(TestGlobalLog, TestFromThreads)
{
    const size_t threads_count = 10;
    {
        auto thread_func = [this, &threads_count](){
            G_ERROR("from ", threads_count, " threads");
            G_INFO("from ", threads_count, " threads");
        };   
        
        std::vector<std::jthread> threads(threads_count);
        for(int i = 0; i < threads_count; ++i)
        {
            threads.emplace_back(thread_func);
        }
    }
    std::this_thread::sleep_for(100ms); // make sure that thread completed work

    message.assign((std::istreambuf_iterator<char>(err)), std::istreambuf_iterator<char>());
    
    std::stringstream regex("^");
    for(int i = 0; i < threads_count; ++i)
    {
        regex << std::format(".*ERROR from {} threads\n", threads_count); 
    }
    regex << "$";
    
    std::string match_string((std::istreambuf_iterator<char>(regex)), std::istreambuf_iterator<char>());

    EXPECT_THAT(message, MatchesRegex(match_string));
}