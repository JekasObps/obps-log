#include <gtest/gtest.h>

#include "obps_log_public.hpp"

GLOBAL_LOG({std::cerr, LogLevel::ERROR});


class TestSomething : public testing::Test
{

};

TEST_F(TestSomething, TestA)
{
    
}



