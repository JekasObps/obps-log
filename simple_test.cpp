#include "obps_log_public.hpp"

constexpr bool str_equal(const char * fun1, const char * fun2)
{
    for ( ;*fun1 != '\0' || *fun2 != '\0'; ++fun1, ++fun2)
    {
        if (*fun1 != *fun2)
            return false;
    }

    if (*fun1 != *fun2)
    {
        return false;
    }
    else
    {
        return true;
    }
}


// simple test to check enabling and disabling logging 
void test1()
{
    SCOPE_LOG(INFO, std::cout);

    CHUKKA("Pass"); 
    // bug! programm never ends if no target catches message.
    // UPDATE: this is not the case, bacause message was not inserted at all
    //          trouble caused by DEADLOCK when main and thread try to do atexit cleanup.
    //          
    // https://stackoverflow.com/questions/10915233/stdthreadjoin-hangs-if-called-after-main-exits-when-using-vs2012-rc 
}

int main()
{
    test1();
}