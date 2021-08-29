#include "obps_log_public.hpp"

// simple test to check enabling and disabling logging 
void test1()
{
    SCOPE_LOG(INFO, std::cout);

    CHUKKA("Pass"); // bug! programm never ends if no target catches message.
}

int main()
{
    test1();
}