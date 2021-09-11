#include "obps_log_public.hpp"

#include <chrono>
#include <thread>

// GLOBAL_LOG(ERROR, std::cerr);

// simple test to check enabling and disabling logging 
void test1()
{
    SCOPE_LOG({LogLevel::INFO, std::cerr});

    USER_LEVEL("---"); // will not get logged
    INFO("Pass");      // ok
}

int main()
{
    SCOPE_LOG({LogLevel::INFO, std::cout});
    OBPS_LOG_INIT();
    INFO("Cass");      // ok
    test1();
    test1();
    test1();
    test1();
    INFO("Cass");      // ok


    std::this_thread::sleep_for(std::chrono::milliseconds(350));
}