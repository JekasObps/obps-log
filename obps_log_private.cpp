#include "obps_log_private.hpp"

namespace obps
{

template class LogBase::LogPool; // instantiate LogPool


Log& Log::Attach(Log& other_log)
{
    assert(&other_log != this && "prevent self attachment!!!");
    
    auto *next = this;

    while (next->m_AttachedLog)
    {
        next = next->m_AttachedLog;
    }

    next->m_AttachedLog = &other_log;
    
    return other_log;
}

bool Log::HasAttachedLog() const noexcept
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

bool Log::IsRelevantLevel(LogLevel level) const noexcept
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


Log::LoggerThreadStatus_ Log::LogThread() 
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

    return LoggerThreadStatus_::RUNNING;
}

Log::LoggerThreadStatus_ Log::LogExceptHandler(const std::exception& e) 
{
    std::cerr << "Exception has been thrown from LogThread!\n" << e.what() << std::endl;
    return LoggerThreadStatus_::FAILED;
}


Log::Log(std::unique_ptr<std::ostream> stream, LogLevel level, 
        size_t queue_size, Formatter format, Log* attached, LogPoolSptr pool)
  : m_Output(std::move(stream))
  , m_Level(level)
  , m_Queue(queue_size)
  , m_Format(format)
  , m_AttachedLog(attached)
  , m_Pool(pool)
{
    m_Pool->RunTask<Log*>
        (&Log::LogExceptHandler, &Log::LogThread, this);
}

Log::Log(const LogSpecs& specs) 
  : Log(nullptr,
    specs._Level,
    DEFAULT_QUEUE_SIZE,
    specs._Format,
    static_cast<Log*>(specs._Attached),
    specs._ThreadPool)
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

    if (m_Output)
        m_Output->flush();
}

void LogBase::default_format(std::ostream& msg_out, MessageData mdata)
{
    msg_out << GetTimeStr("%F %T [") 
            << mdata.Tid << "] "
            << mdata.PrettyLevel << " " << mdata.Content 
            << std::endl;
};

void LogBase::JSON_format(std::ostream& msg_out, MessageData mdata)
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
