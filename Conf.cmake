# main log on/off lever
option(ENABLE_LOGGING ON)

# default queue size for each log
set(DEFAULT_QUEUE_SIZE 64)

# message size have to be aligend to the word size of the system
# ! note that minimum 2 bytes of this amount will be used to store msg size data !
set(MAX_MSG_SIZE 256)


# custom user defined levels:
# ! keep in upper case for the sake of convention
set(OBPS_LOG_LEVELS 
    ERROR
    WARN
    INFO
    CHUKKA # test - target
    DEBUG
)

