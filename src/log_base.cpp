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

/*
*   Singleton Builder For LogRegistry.
*/
LogBase::LogRegistrySptr LogBase::GetLogRegistry()
{
    static LogRegistrySptr s_Instance;
    static bool isInit = false;
    if (!isInit){
        s_Instance = std::make_shared<LogRegistry>();
        isInit = true;
    }
    return s_Instance;
}


void LogBase::default_format(std::ostream& out, const std::time_t ts, const LogLevel level, const std::thread::id tid, const char* text)
{
    out << get_time_string("%F %T ", ts) << "[" 
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
        << " : " << std::quoted(get_time_string("%F %T", ts)) << ",\n"
        << "  "  << std::setw(10) << std::quoted("tid")
        << " : " << tid << ",\n"
        << "  "  << std::setw(10) << std::quoted("message") 
        << " : " << std::quoted(text)
        << "\n},\n";
};

std::unique_ptr<std::ostream> LogBase::OpenFileStream(fs::path log_path)
{
    auto&& log_name = log_path.filename();
    log_path.replace_filename(make_log_filename(log_name.string()));
    auto file = std::make_unique<std::ofstream>(log_path, std::ios::app);
    if (file->fail())
    {
        throw std::runtime_error("Failed To Open LogFile!");
    }
    return std::move(file);
}

std::string make_log_filename(const std::string& prefix_name)
{
    return prefix_name + "-" + get_time_string("%F", get_timestamp()) + ".log";
}

std::string get_time_string(const char* fmt, const std::time_t stamp) noexcept
{
    tm date_info;
    
    __localtime(&date_info, &stamp);
    
    char timestr_buffer[128];
    strftime(timestr_buffer, 128, fmt, &date_info);

    return timestr_buffer;
}

const std::time_t get_timestamp() noexcept
{
    const auto date = std::chrono::system_clock::now();
    return std::chrono::system_clock::to_time_t(date);
}

} // namespace obps