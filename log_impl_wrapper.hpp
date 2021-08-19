#pragma once


// if logging enabled in configuration header
//  append log implementation.

#include "ObpsLogConfig.hpp"
#ifdef LOG_ON
    #include "obps_log_impl.cpp"
#endif // LOG_ON