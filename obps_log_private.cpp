#include "obps_log_private.hpp"

namespace obps
{

template class LogBase::LogPool; // instantiate LogPool

///////////////////
//      Log      //
///////////////////


void Log::SendToQueue()
{

}



Log::Log(const LogSpecs& specs)
{
    switch(specs._Target.Type)
    {
        case LogSpecs::TargetType::FILENAME
        {
            auto file = OpenFileStream(specs._Target.Value.Filename);
            m_Output.swap(file);
            break;
        }
        case LogSpecs::TargetType::STREAM:
        {
            auto stream = std::make_unique<std::ostream>(specs._Target.Value.Stream->rdbuf());
            m_Output.swap(stream);
            break;
        }
    }
}

Log::~Log()
{
}


Log::LoggerThreadStatus_ Log::LogThread(std::ostream& output) 
{
    m_Queue.ReadTo([this, &output](const char * const msg, uint16_t size){
        output.write(msg, size);

#ifdef FLUSH_EVERY_MESSAGE
        output.flush(); 
#endif // FLUSH_EVERY_MESSAGE
        if (output.fail())
        {
            throw std::exception("From Writer thread: IO Failed!");
        }
    });

    //TODO: Global log never ending, need to shutdown 
    return LoggerThreadStatus_::RUNNING;
}

///////////////////
//    LogBase    //
///////////////////

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

std::unique_ptr<std::ostream> LogBase::OpenFileStream(const std::string& logname)
{
    auto file = std::make_unique<std::ofstream>(MakeLogFileName(logname), std::ios::app);
    if (file->fail())
    {
        throw std::runtime_error("Failed To Open LogFile!");
    }
    return std::move(file);
}

std::string LogBase::MakeLogFileName(const std::string& prefix_name)
{
    return prefix_name + "-" + GetTimeStr("%F") + ".log";
}

std::string LogBase::GetTimeStr(const std::string& fmt)
{
    const auto date = std::chrono::system_clock::now();
    const std::time_t t_c = std::chrono::system_clock::to_time_t(date);
    tm date_info;
    
    __localtime(&date_info, &t_c);
    
    char timestr_buffer[128];
    strftime(timestr_buffer, 128, fmt.c_str(), &date_info);

    return timestr_buffer;
}

/////////////////////
//     LogSpecs    //
/////////////////////


LogBase::LogSpecs& LogBase::LogSpecs::ThreadPool(LogPoolSptr pool)
{
    _ThreadPool = pool;
    return *this;
}

LogBase::LogSpecs& LogBase::LogSpecs::DefaultQueue(LogQueueSptr queue)
{
    _DefaultLogQueue = queue;
    return *this;
} 

} // namespace obps
