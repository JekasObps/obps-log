#include "obps_log_private.hpp"

namespace obps
{

template class LogBase::LogPool; // instantiate LogPool

///////////////////
//      Log      //
///////////////////

Log::Log(const LogSpecs& specs) : m_Pool(specs._ThreadPool)
{
    for (auto&& o_spec : specs._Outputs)
    {
        AddOutput(o_spec);
    }
}

Log::~Log()
{}

void Log::AddOutput(const LogSpecs::OutputSpec & o_spec)
{
    auto&& output = m_Outputs.emplace_back(CreateOutput(o_spec));

    m_Pool->RunTask<LogQueueSptr, std::shared_ptr<std::ostream>>(&Log::LogThread, std::get<2>(output), std::get<4>(output));
}

Log::Output Log::CreateOutput(const LogBase::LogSpecs::OutputSpec & o_spec)
{
    switch (o_spec.file_or_stream.Type)
    {
        case LogSpecs::OutputType::FILENAME: 
        {
            return std::make_tuple(o_spec.level, o_spec.mod, o_spec.queue, o_spec.format,
                OpenFileStream(o_spec.file_or_stream.Value.Filename));
        }
        case LogSpecs::OutputType::STREAM:
        {
            return std::make_tuple(o_spec.level, o_spec.mod, o_spec.queue, o_spec.format, 
                std::make_unique<std::ostream>(o_spec.file_or_stream.Value.Stream->rdbuf()));
        }
        default:
            throw std::logic_error("FileOrStream::UnknownType");
    };
}

Log::LoggerThreadStatus_ Log::LogThread(LogQueueSptr queue, std::shared_ptr<std::ostream> output) 
{
    // Constructing and writing to the stream inside syncronizing decorator
    auto && status = queue->ReadTo([&output] (const char * const buffer, size_t size){
        MessageData message;
        LogQueue_::Construct<MessageData>(&message, buffer);
        message.Format(*output, message.TimeStamp, message.Level, message.Tid, message.Text);
    });
        
    if (status == LogQueue_::OperationStatus::SHUTDOWN)
    {
        output->flush();
        return LoggerThreadStatus_::FINISHED;
    }

#ifdef FLUSH_EVERY_MESSAGE
    output->flush(); 
#endif // FLUSH_EVERY_MESSAGE

    if (output->fail())
    {
        return LoggerThreadStatus_::ABORTED;
    }
    return LoggerThreadStatus_::RUNNING;
}

///////////////////
//    LogBase    //
///////////////////

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

std::string MakeLogFileName(const std::string& prefix_name)
{
    return prefix_name + "-" + GetTimeStr("%F", get_timestamp()) + ".log";
}

std::string GetTimeStr(const char* fmt, const std::time_t stamp) noexcept
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
