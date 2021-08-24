#include "obps_log_public.hpp"

// simple test to check enabling and disabling logging 
void test1()
{
    MakeScopeLog({std::cout, obps::LogLevel::INFO});

    Info("Pass");
}

int main()
{
    test1();
}