#include "obps_log_private.hpp"

namespace obps
{

// Constructs Log instance from specialization object
Log::Log(LogSpecs&& specs) : m_Pool(specs.GetLogPool())
{
    for (auto&& o_spec : specs.GetOutputSpecs())
    {
        AddOutput(o_spec);
    }
}

// Creates output target(file or stream) and spowns a logThread that will write to this target
void Log::AddOutput(const LogSpecs::OutputSpecs& o_spec)
{
    // important to store and then reference output when Running Task.
    auto&& output = m_Outputs.emplace_back(CreateOutput(o_spec));

    m_Pool->RunTask<LogQueueSptr, OstreamSptr>(
        &Log::LogThread, 
        std::get<LogQueueSptr>(output),
        std::get<OstreamSptr>(output)
    );
}

// Stores levels that will be ignored by this Log instance
// Not Thread safe:
//  designed to be called in the same thread where Log has been initialized
void Log::Mute(const std::unordered_set<LogLevel>& mute_levels)
{
    m_MutedLevels = mute_levels;
}

// Unmutes some levels, reanabling them for output
// Not Thread safe: same as Mute
void Log::Unmute(const std::set<LogLevel>& unmute_levels)
{
    for (const auto& level : unmute_levels)
    {
        m_MutedLevels.erase(level);
    }
}

// helps to convert from OutputSpecs to an actual Output to be stored in a Log instance
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

// thread function that runs in separate thread per each instance of a Log class
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
