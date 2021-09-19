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

    m_Pool->RunTask<LogQueueSptr, OstreamSptr>(
        &Log::LogThread, 
        std::get<2>(output), // LogQueueSptr
        std::get<4>(output) // OstreamSptr
    );
}

Log::Output Log::CreateOutput(const LogBase::LogSpecs::OutputSpecs& o_spec)
{
    const auto& target = o_spec.Target;
    if (target.isPath())
    {
        return std::make_tuple(o_spec.Level, o_spec.Mod, o_spec.Queue, o_spec.Format,
            OpenFileStream(target.getPath()));
    }
    else
    {
        return std::make_tuple(o_spec.Level, o_spec.Mod, o_spec.Queue, o_spec.Format, 
            std::make_shared<std::ostream>(target.getStream()->rdbuf()));
    }
}

Log::LoggerThreadStatus Log::LogThread(LogQueueSptr queue, OstreamSptr output) 
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
