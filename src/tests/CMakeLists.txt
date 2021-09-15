################
### TESTING: ###
################

setup_testing()

# additional tests:

configuration_test(
    TEST test_logging_enabled 
    SOURCE config_test.cpp 
    FLAGS "-DLOG_ON" 
    LINK_LIBS PUBLIC obps_log
    EXPECTED "INFO Pass"
)
configuration_test(
    TEST test_logging_disabled 
    SOURCE config_test.cpp 
    LINK_LIBS PUBLIC obps_log
    EXPECTED "^?"
)

