#include "obps_log.hpp"

#if defined(WIN32)
#    define __localtime(x, y) localtime_s( x, y )
#elif defined(LINUX)
#    define __localtime(x, y) localtime_r( y, x )
#endif

namespace obps
{

#ifdef LOG_ON
void Log::WriterFunction()
{
    while (m_Queue.isOpen() || m_Queue.isReadAvailable())
    {
        m_Queue.ReadTo(
            [this](const char *msg, uint16_t size) {
                m_Output->write(msg, size);
#ifdef AUTO_FLUSHING
                m_Output->flush(); 
#endif // AUTO_FLUSHING
                //      providing user a freedom to decide whenever to do flush.
                //      (user can also do so by sending std::flush)
                if (m_Output->bad())
                {
                    throw std::runtime_error("WriterThread:: IO Fail!");
                }
        });
    }
}
#endif // LOG_ON

std::shared_ptr<Log> Log::Create(const std::string& logname, LogLevel level)
{
#ifdef LOG_ON
    std::string filename = MakeLogFileName(logname);
    auto file = std::make_unique<std::ofstream>(filename, std::ofstream::app);
    if (file->fail())
    {
        throw std::runtime_error("Failed To Open File!");
    }

    return std::shared_ptr<Log>(
        new Log(std::move(file), level)
    );
#else 
    return nullptr;
#endif // LOG_ON
}

std::shared_ptr<Log> Log::Create(std::ostream& out, LogLevel level)
{
#ifdef LOG_ON
    return std::shared_ptr<Log>(
        new Log(std::make_unique<std::ostream>(out.rdbuf()), level)
    );
#else
    return nullptr;
#endif // LOG_ON
}

void Log::Attach(std::shared_ptr<Log> other_log)
{
#ifdef LOG_ON
    if (!(other_log->m_AttachedLog).expired() &&
        &*other_log->m_AttachedLog.lock() == this) 
    {
        throw std::logic_error("Log::Cyclic Attachment is prohibited!");
    }

    if (&*other_log == this)
    {
        throw std::logic_error("Log::Self Attachment is prohibited!");
    }

    m_AttachedLog = other_log;
#endif // LOG_ON
}

std::string Log::GetTimeStr(const std::string& fmt)
{
    const auto date = std::chrono::system_clock::now();
    const std::time_t t_c = std::chrono::system_clock::to_time_t(date);
    tm date_info;

    __localtime(&date_info, &t_c);
    
    char timestr_buffer[128];
    strftime(timestr_buffer, 128, fmt.c_str(), &date_info);

    return timestr_buffer;
}

std::string Log::MakeLogFileName(const std::string& prefix_name)
{
    return prefix_name + "-" + GetTimeStr("%F") + ".log";
}

Log::Log(std::unique_ptr<std::ostream> stream, LogLevel level, size_t queue_size, Formatter format)
#ifdef LOG_ON
  : m_Output(std::move(stream))
  , m_Level(level)
  , m_Queue(queue_size)
  , m_LogWriter(&Log::WriterFunction, this)
  , m_Format(format)
#endif // LOG_ON
{}

Log::~Log()
{
#ifdef LOG_ON
    m_Queue.Close();
    m_LogWriter.join();
#endif // LOG_ON
}

void Log::default_format(
        std::ostream&       msg,
        const std::string&  content,
        const std::string&  date_fmt,
        const std::string&  pretty_level,
        std::thread::id     tid
    )
{
    msg << GetTimeStr("%F %T [") << tid << "] " << pretty_level << " " << content 
        << std::endl;
};

void Log::JSON_format(
        std::ostream&       msg,
        const std::string&  content,
        const std::string&  date_fmt,
        const std::string&  pretty_level,
        std::thread::id     tid
    )
{
    msg << "{"   << std::endl << std::left
        << "  "  << std::setw(10) << std::quoted("level") 
        << " : " << std::quoted(pretty_level) << "," << std::endl
        << "  "  << std::setw(10) << std::quoted("date")
        << " : " << std::quoted(GetTimeStr(date_fmt)) << "," << std::endl
        << "  "  << std::setw(10) << std::quoted("tid")
        << " : " << tid << "," << std::endl
        << "  "  << std::setw(10) << std::quoted("message") 
        << " : " << std::quoted(content) << std::endl 
        << "},"  << std::endl;
};

} // namespace obps
