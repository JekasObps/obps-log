# ObpsLog
Simple c++ logging library written for my personal use.


# Design

Logging functionality designed to be easily turned on and off at compile time.
This feature accomplished by using macros defined in obps_log_public.hpp

Each log instance contains separate log queue and worker thread.

SCOPE_LOG
    Initializes static storage log in bound scope. 
    Must be used only inside function scope. Usage in global scope yields undefined behaviour!
    Scope log finds its application in thread functions. Thread safe.

ATTACH
    Initialize additional logger and links it to a scope logger.
    So, messages writed to a scope log will printed in additional log if they match additional log's level.
    This feature useful when you want to output different levels to different targets.

    It's convenient for them to share queue and worker, but this haven't implemented yet. 

DEBUG, INFO, WARN, ERROR
    Default logging macros for each level. You can add custom macros

GLOBAL_LOG 
    Initializes global log. Must be called in global space.
    Can be used in multiple threads, but notice that performance will be affected by single queue and worker thread.


Log messages in thread   

