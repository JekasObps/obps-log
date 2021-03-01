#include "obps_log.hpp"

namespace obps
{

void ObpsLog::WriterFunction()
{
    while (m_Queue.isOpen() || m_Queue.isReadAvailable())
    {
        m_Queue.ReadTo(
            [this](const char *msg, uint16_t size) {
                m_Output->write(msg, size);
                m_Output->flush();
                if (m_Output->bad())
                {
                    throw std::runtime_error("WriterThread:: IO Fail!");
                }
        });
    }
}

std::shared_ptr<ObpsLog> ObpsLog::CreateLog(const std::string& logname, LogLevel level)
{
    std::string filename = MakeLogFileName(logname);
    auto file = std::make_unique<std::ofstream>(filename, std::ofstream::app);
    if (file->fail())
    {
        throw std::runtime_error("Failed To Open File!");
    }

    return std::shared_ptr<ObpsLog>(
        new ObpsLog(std::move(file), level)
    );
}

std::shared_ptr<ObpsLog> ObpsLog::CreateLog(std::ostream& out, LogLevel level)
{
    return std::shared_ptr<ObpsLog>(
        new ObpsLog(std::make_unique<std::ostream>(out.rdbuf()), level)
    );
}

void ObpsLog::Attach(std::shared_ptr<ObpsLog> other_log)
{
    if (!(other_log->m_AttachedLog).expired() &&
        &*other_log->m_AttachedLog.lock() == this) 
    {
        throw std::logic_error("ObpsLog::Cyclic Attachment is prohibited!");
    }

    if (&*other_log == this)
    {
        throw std::logic_error("ObpsLog::Self Attachment is prohibited!");
    }

    m_AttachedLog = other_log;
}

std::string ObpsLog::GetTimeStr(const std::string& fmt)
{
    const auto date = std::chrono::system_clock::now();
    const std::time_t t_c = std::chrono::system_clock::to_time_t(date);
    tm date_info;

    localtime_s(&date_info, &t_c);
    
    char timestr_buffer[128];
    strftime(timestr_buffer, 128, fmt.c_str(), &date_info);

    return timestr_buffer;
}

std::string ObpsLog::MakeLogFileName(const std::string& prefix_name)
{
    return prefix_name + "-" + GetTimeStr("%F") + ".log";
}

ObpsLog::ObpsLog(std::unique_ptr<std::ostream> stream, LogLevel level, size_t queue_size, Formatter format)
#ifdef LOG_ON
  : m_Output(std::move(stream))
  , m_Level(level)
  , m_Queue(queue_size)
  , m_LogWriter(&ObpsLog::WriterFunction, this)
  , m_Format(format)
#endif // LOG_ON
{}

ObpsLog::~ObpsLog()
{
#ifdef LOG_ON
    m_Queue.Close();
    m_LogWriter.join();
#endif // LOG_ON
}

void ObpsLog::default_format(
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

void ObpsLog::JSON_format(
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