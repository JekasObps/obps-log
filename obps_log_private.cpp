#include "obps_log_private.hpp"

#if defined(WIN32)
#    define __localtime(x, y) localtime_s( x, y )
#elif defined(LINUX)
#    define __localtime(x, y) localtime_r( y, x )
#endif

namespace obps
{

Log& Log::Attach(Log& other_log)
{
    if (other_log.HasAttachedLog() &&
        other_log.m_AttachedLog == this) 
    {
        throw std::logic_error("Log::Cyclic Attachment is prohibited!");
    }

    if (&other_log == this)
    {
        throw std::logic_error("Log::Self Attachment is prohibited!");
    }
    
    auto *next = this;

    while (next->m_AttachedLog)
    {
        next = next->m_AttachedLog;
    }

    next->m_AttachedLog = &other_log;
    
    return other_log;
}

bool Log::HasAttachedLog() const 
{
    return m_AttachedLog != nullptr;
}

std::unique_ptr<std::ostream> Log::OpenFileStream(const std::string& logname)
{
    auto file = std::make_unique<std::ofstream>(MakeLogFileName(logname), std::ios::app);
    if (file->fail())
    {
        throw std::runtime_error("Failed To Open LogFile!");
    }
    return std::move(file);
}

bool Log::IsRelevantLevel(LogLevel level) const
{
    return m_Level >= level;
}

void Log::WriteForward(LogLevel level, const std::string& message)
{
    if (IsRelevantLevel(level))
    {
        SendToQueue(message);
    }

    if(HasAttachedLog())
    {
        m_AttachedLog->WriteForward(level, message);
    }
}

void Log::SendToQueue(const std::string& message)
{
    assert(message.size() < m_Queue.max_message_size);
    m_Queue.Write(message.c_str(), static_cast<uint16_t>(message.size()));
}

void ExitHandler()
{
    // prevent thread to exit sponateously on main::return
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void Log::WriterFunction() 
try 
{
    const auto result = std::atexit(ExitHandler);
    if (result != 0)
    {
        std::cerr << "Failed to register exit handler! at " << __FILE__ <<  "::" << __LINE__ << std::endl;
        std::terminate();
    }
    
    while (m_Queue.isAlive())
    {
        m_Queue.ReadTo([this](const char *msg, uint16_t size){
            m_Output->write(msg, size);
  #ifdef FLUSH_EVERY_MESSAGE
            m_Output->flush(); 
  #endif // FLUSH_EVERY_MESSAGE
            if (m_Output->fail())
            {
                throw std::exception("From Writer thread: IO Failed!");
            }
        });
    }
}
catch (std::exception &e)
{
    m_WriterException = std::make_exception_ptr(e);
}

void Log::HandleWriterException()
try {
    if (m_WriterException)
    {
        std::rethrow_exception(m_WriterException);
    }
}
catch(std::exception &e)
{
    // it's considerable to use a user defined hook here.
#ifdef DEBUG_MODE
    throw std::runtime_error("DEBUG::throw instead of exit(-1)");
#else
    std::cerr << e.what() << "EXITING WITH CODE -1\n";
    std::exit(-1);
#endif // DEBUG_MODE
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

Log::Log(std::unique_ptr<std::ostream> stream, LogLevel level, 
        size_t queue_size, Formatter format, Log* attached)
  : m_Output(std::move(stream))
  , m_Level(level)
  , m_Queue(queue_size)
  , m_LogWriter(&Log::WriterFunction, this)
  , m_Format(format)
  , m_AttachedLog(attached)
{}

Log::Log(const LogSpecs& specs)
  : Log(nullptr,
        specs._Level,
        DEFAULT_QUEUE_SIZE,
        specs._Format,
        specs._Attached)
{
    switch(specs._Target.Type)
    {
        case LogSpecs::FILENAME:
        {
            auto file = OpenFileStream(specs._Target.Value.Filename);
            m_Output.swap(file);
            break;
        }
        case LogSpecs::STREAM:
        {
            auto stream = std::make_unique<std::ostream>(specs._Target.Value.Stream->rdbuf());
            m_Output.swap(stream);
            break;
        }
    }
}

Log::~Log()
{
    m_Queue.ShutDown();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    m_LogWriter.join();
    if (m_Output)
        m_Output->flush();
}

void Log::default_format(std::ostream& msg_out, MessageData mdata)
{
    msg_out << GetTimeStr("%F %T [") 
            << mdata.Tid << "] "
            << mdata.PrettyLevel << " " << mdata.Content 
            << std::endl;
};

void Log::JSON_format(std::ostream& msg_out, MessageData mdata)
{
    msg_out << "{"   << std::endl << std::left
        << "  "  << std::setw(10) << std::quoted("level") 
        << " : " << std::quoted(mdata.PrettyLevel) << "," << std::endl
        << "  "  << std::setw(10) << std::quoted("date")
        << " : " << std::quoted(GetTimeStr(mdata.DateFmt)) << "," << std::endl
        << "  "  << std::setw(10) << std::quoted("tid")
        << " : " << mdata.Tid << "," << std::endl
        << "  "  << std::setw(10) << std::quoted("message") 
        << " : " << std::quoted(mdata.Content) << std::endl 
        << "},"  << std::endl;
};

} // namespace obps
