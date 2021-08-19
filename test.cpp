#include "obps_log.hpp"


bool test1()
{
    MakeScopeLog({std::cout, obps::LogLevel::INFO});
    Attach({std::cerr, obps::LogLevel::ERROR});
    Attach({std::cout, obps::LogLevel::DEBUG});

    // std::cout << EXP(__LINE__, suffix) << std::endl;

    Error("ERROR!");
    Debug("Debug");


    return true;
}

int main()
{

    test1();
}