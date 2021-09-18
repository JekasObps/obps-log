#include "obps_log_private.hpp"

namespace obps
{

using namespace std::chrono_literals;

Log::Log(LogSpecs&& specs) : m_Pool(specs.GetLogPool())
{
    for (auto&& o_spec : specs.GetOutputSpecs())
    {
        AddOutput(o_spec);
    }
}

Log::~Log()
{}

void Log::AddOutput(const LogSpecs::OutputSpecs& o_spec)
{
    auto&& output = m_Outputs.emplace_back(CreateOutput(o_spec));

    m_Pool->RunTask<LogQueueSptr, std::shared_ptr<std::ostream>>(
        &Log::LogThread, 
        std::get<2>(output), // LogQueueSptr
        std::get<4>(output) // std::shared_ptr<std::ostream>
    );
}

Log::Output Log::CreateOutput(const LogBase::LogSpecs::OutputSpecs& o_spec)
{
    const auto& target = o_spec.path_or_stream;
    if (target.isPath())
    {
        return std::make_tuple(o_spec.level, o_spec.mod, o_spec.queue, o_spec.format,
            OpenFileStream(target.getPath()));
    }
    else
    {
        return std::make_tuple(o_spec.level, o_spec.mod, o_spec.queue, o_spec.format, 
            std::make_shared<std::ostream>(target.getStream()->rdbuf()));
    }
}

Log::LoggerThreadStatus Log::LogThread(LogQueueSptr queue, std::shared_ptr<std::ostream> output) 
{
    // Constructing and writing to the stream inside syncronizing decorator
    auto && status = queue->ReadTo([&output] (const char * const buffer, size_t size){
        MessageData message;
        LogQueue::Construct<MessageData>(&message, buffer);

        message.Format(*output, message.TimeStamp, message.Level, message.Tid, message.Text);
        if (message.Sync)
        {
            output->flush();
        }
    });
        
    if (status == LogQueue::OperationStatus::SHUTDOWN)
    {
        output->flush();
        return LoggerThreadStatus::FINISHED;
    }

    if (output->fail())
    {
        return LoggerThreadStatus::ABORTED;
    }
    return LoggerThreadStatus::RUNNING;
}

} // namespace obps
