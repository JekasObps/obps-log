################
### TESTING: ###
################

setup_testing()

# additional tests:

configuration_test(test_logging_enabled simple_test.cpp "-DLOG_ON" "PRIVATE obps_log" "INFO Pass")
configuration_test(test_logging_disabled simple_test.cpp "" "PRIVATE obps_log" "^?")

