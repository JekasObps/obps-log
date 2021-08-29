#define OBPS_LOG_LEVELS ERROR, WARN, INFO, DEBUG

#define CASE(C) case LogLevel::C : return #C;

#define _PRETTY_LEVEL(arg, ...) CASE(arg) _СASE_1(__VA_ARGS__)
#define _СASE_1(arg, ...) CASE(arg) _СASE_2(__VA_ARGS__)
#define _СASE_2(arg, ...) CASE(arg) _СASE_3(__VA_ARGS__)
#define _СASE_3(arg, ...) CASE(arg) _СASE_4(__VA_ARGS__)
#define _СASE_4(arg, ...) CASE(arg) _СASE_5(__VA_ARGS__)
#define _СASE_5(arg, ...) CASE(arg) _СASE_6(__VA_ARGS__)

#define PRETTY_OBPS_LOG_LEVELS(...) _PRETTY_LEVEL(__VA_ARGS__)



PRETTY_OBPS_LOG_LEVELS(OBPS_LOG_LEVELS)