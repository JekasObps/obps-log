#include "obps_log_private.hpp"

namespace obps
{

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
    switch (o_spec.path_or_stream.Type)
    {
        case LogSpecs::OutputType::PATH: 
        {
            return std::make_tuple(o_spec.level, o_spec.mod, o_spec.queue, o_spec.format,
                OpenFileStream(o_spec.path_or_stream.Value.Path));
        }
        case LogSpecs::OutputType::STREAM:
        {
            return std::make_tuple(o_spec.level, o_spec.mod, o_spec.queue, o_spec.format, 
                std::make_unique<std::ostream>(o_spec.path_or_stream.Value.Stream->rdbuf()));
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

} // namespace obps
