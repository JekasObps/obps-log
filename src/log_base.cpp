#include "log_base.hpp"

namespace obps
{

template class LogBase::LogPool; // instantiate LogPool

/*
*   Singleton Builder For ThreadPool.
*/
LogBase::LogPoolSptr LogBase::GetDefaultThreadPoolInstance() 
{
    static LogPoolSptr s_Instance;
    static bool isInit = false;
    if (!isInit){
        s_Instance = std::make_shared<LogPool>();
        isInit = true;
    }
    return s_Instance;
}

/*
*   Singleton Builder For Queue.
*/
LogBase::LogQueueSptr LogBase::GetDefaultQueueInstance() 
{
    static LogQueueSptr s_Instance;
    static bool isInit = false;
    if (!isInit){
        s_Instance = std::make_shared<LogQueue_>(default_queue_size);
        isInit = true;
    }
    return s_Instance;
}


void LogBase::default_format(std::ostream& out, const std::time_t ts, const LogLevel level, const std::thread::id tid, const char* text)
{
    out << GetTimeStr("%F %T ", ts) << "[" 
        << tid << "] "
        << PrettyLevel(level) << " "
        << text << "\n";
};

void LogBase::JSON(std::ostream& out, const std::time_t ts, const LogLevel level, const std::thread::id tid, const char* text)
{
    out << "{\n" << std::left
        << "  "  << std::setw(10) << std::quoted("level") 
        << " : " << std::quoted(PrettyLevel(level)) << ",\n"
        << "  "  << std::setw(10) << std::quoted("date")
        << " : " << std::quoted(GetTimeStr("%F %T", ts)) << ",\n"
        << "  "  << std::setw(10) << std::quoted("tid")
        << " : " << tid << ",\n"
        << "  "  << std::setw(10) << std::quoted("message") 
        << " : " << std::quoted(text)
        << "\n},\n";
};

std::unique_ptr<std::ostream> LogBase::OpenFileStream(const std::string& logname)
{
    auto file = std::make_unique<std::ofstream>(MakeLogFileName(logname), std::ios::app);
    if (file->fail())
    {
        throw std::runtime_error("Failed To Open LogFile!");
    }
    return std::move(file);
}

} // namespace obps