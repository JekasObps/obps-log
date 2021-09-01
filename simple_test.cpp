#include "obps_log_public.hpp"


// simple test to check enabling and disabling logging 
void test1()
{
    SCOPE_LOG(INFO, std::cout);

    USER_LEVEL("---"); // will not get logged
    INFO("Pass");      // ok
}

int main()
{
    test1();

    OBPS_LOG_DESTROY();
}