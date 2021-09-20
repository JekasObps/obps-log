# main log on/off lever
option(ENABLE_LOGGING ON)

# default queue size in messages for each log
set(DEFAULT_QUEUE_SIZE 64)

# for the memory alignment adviced to make it 2^(N) - 2 
set(MAX_MSG_SIZE 254) # 2^(8) - 2

# amout of memory that queue allocates for messages is: 
#   QUEUE_SIZE * (sizeof(uint16_t) + MAX_MSG_SIZE) 

# custom user defined levels:
# ! keep in upper case for the sake of convention
set(OBPS_LOG_LEVELS 
    ERROR
    WARN
    INFO
    USER_LEVEL
    DEBUG
)

