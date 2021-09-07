#include <iostream>
#include <thread>
#include <chrono>

#include <map>

#include <gtest/gtest.h>

#include "log_queue.hpp"

using namespace obps;
using LogQueue__ = LogQueue<64>;


#ifdef DEBUG_MODE
TEST(LogQueueDeathTest, Create) {
    EXPECT_DEATH({
        LogQueue__ queue(0);
    }, "^Assertion failed: queue_size > 0"); // forbidden, assert
}
#endif

#define MSG_1 "Hello There!"


class LogQueueTest: public testing::Test {
protected:
    LogQueueTest()
      : smallest_queue(1), queue(64) {}

    void SetUp() override {
    }

    LogQueue__ smallest_queue;
    LogQueue__ queue;

    char buffer[LogQueue__::max_message_size] = {0};
    static const size_t message_len = sizeof(MSG_1);
    const char message[message_len] = MSG_1;
};


TEST_F(LogQueueTest, CreateQueueOfSize1) {
    EXPECT_EQ(smallest_queue.GetSize(), 1);    
}


TEST_F(LogQueueTest, SingleReadWrite) {
    queue.WriteFrom(message, message_len);
    EXPECT_EQ(queue.GetMessagesCount(), 1)
        << "Message count expected to be increased after WriteFrom(...).";

    queue.ReadTo(buffer);

    EXPECT_EQ(queue.GetMessagesCount(), 0)
        << "Message count expected to descend after ReadTo(...).";

    EXPECT_EQ(std::strncmp(message, buffer, message_len), 0) 
        << "Message was changed after fetching from queue! expected: \"" 
        << message << "\" result: \"" << buffer << "\"";
}


TEST_F(LogQueueTest, WriteAndReadToStream) {
    std::stringstream in_stream, out_stream;
    in_stream << message;

    queue.WriteFrom(in_stream, message_len);
    EXPECT_EQ(queue.GetMessagesCount(), 1)
        << "Message count expected to be increased after WriteFrom(...).";

    queue.ReadTo(out_stream);
    EXPECT_EQ(queue.GetMessagesCount(), 0)
        << "Message count expected to descend after ReadTo(...).";

    out_stream.read(buffer, message_len);

    EXPECT_EQ(std::strncmp(buffer, message, message_len), 0) 
        << "Message was changed after fetching from queue! expected: \"" 
        << message << "\" result: \"" << buffer << "\"";
}


TEST_F(LogQueueTest, TestFullandEmpty) {
    EXPECT_FALSE(smallest_queue.isReadAvailable());
    EXPECT_TRUE(smallest_queue.isWriteAvailable());

    smallest_queue.WriteFrom(message, message_len);

    EXPECT_TRUE(smallest_queue.isReadAvailable());
    EXPECT_FALSE(smallest_queue.isWriteAvailable());
}


TEST_F(LogQueueTest, TestIsAlive) {
    EXPECT_TRUE(queue.isAlive());
    queue.~LogQueue__();
    EXPECT_FALSE(queue.isAlive());
}


TEST_F(LogQueueTest, TestShutDownReaders) {
    EXPECT_TRUE(queue.isAlive());
    queue.ShutDown();
    EXPECT_FALSE(queue.isAlive());
}


TEST_F(LogQueueTest, TestAvailable) {
    for(int i=0; i < queue.GetSize(); ++i)
    {
        queue.WriteFrom(message, message_len);
        EXPECT_TRUE(queue.isReadAvailable());
    }
    
    EXPECT_FALSE(queue.isWriteAvailable());

    for(int i=0; i < queue.GetSize(); ++i)
    {
        EXPECT_TRUE(queue.isReadAvailable());
        queue.ReadTo(buffer);
        EXPECT_TRUE(queue.isWriteAvailable());
    }

    EXPECT_FALSE(queue.isReadAvailable());
}


TEST_F(LogQueueTest, TestAvailable2) {
    for(int i=0; i < queue.GetSize() * 3; ++i)
    {
        queue.WriteFrom(message, message_len);
        EXPECT_TRUE(queue.isReadAvailable());
        EXPECT_TRUE(queue.isWriteAvailable());
        queue.ReadTo(buffer);
        EXPECT_FALSE(queue.isReadAvailable());
        EXPECT_TRUE(queue.isWriteAvailable());
    }
}


TEST_F(LogQueueTest, TestReadWriteEmplace)
{
    struct Data
    {
        int i;
        float f;
    };

    Data data;
    
    queue.WriteEmplace<Data>(2, 0.99f);
    queue.ReadEmplace(&data);

    ASSERT_EQ(data.i, 2);
    ASSERT_FLOAT_EQ(data.f, 0.99f);
}

TEST_F(LogQueueTest, TestUserCallbacks)
{
    char src[] = "abcdefg";
    char dest[sizeof(src)]; 

    queue.WriteFrom([&src](char * queue_message_buffer, uint16_t & queue_message_size_out){
        queue_message_size_out = sizeof(src);
        std::memcpy(queue_message_buffer, src, queue_message_size_out);
    });

    queue.ReadTo([&dest](const char * queue_message_buffer, uint16_t queue_message_size){
        std::memcpy(dest, queue_message_buffer, queue_message_size);
    });

    ASSERT_STREQ(src, dest);
}


class MultiThreadTest : public testing::Test {
public:
    using thread_fun = void (MultiThreadTest::* )(LogQueue__ &);

    void writer(LogQueue__ & queue)
    {
        std::stringstream stream;
        stream << std::this_thread::get_id();
        std::string s;
        stream >> s;

        for(int j = 0; j < amount_of_messages_each_writer_sends; ++j)
        {
            queue.WriteFrom(s.c_str(), s.size()+1);
        }
    };


    void reader(LogQueue__ & queue)
    { 
        while(reader_global_count < reader_target)
        {
            queue.ReadTo([this](const char* msg, uint16_t size) {
                ++map[msg];
                ++reader_global_count; // protected by m_ReaderMutex
            });
        }
    };

protected:
    MultiThreadTest() : queue(100), big_queue(1000) {}
    
    void SetUp() override {
        //...
    }

    LogQueue__ queue, big_queue;

    void LaunchWriters(thread_fun f, LogQueue__ &queue)
    {
        for(int i=0; i < num_of_writers; ++i)
        { 
            writers.emplace_back(f, this, std::ref(queue));
        }
    }

    void LaunchReaders(thread_fun f, LogQueue__ &queue)
    {
        for(int i=0; i < num_of_readers; ++i)
        {
            readers.emplace_back(f, this, std::ref(queue));
        }
    }

    void JoinWriters()
    {
        for(auto&& writer : writers)
        {
            writer.join();
        }
    }

    void JoinReaders()
    {
        for(auto&& reader : readers)
        {
            reader.join();
        }
    }

    size_t num_of_writers = 100;
    size_t num_of_readers = num_of_writers / 4;
    size_t amount_of_messages_each_writer_sends = 10;
    size_t reader_target = num_of_writers * amount_of_messages_each_writer_sends;
    size_t reader_global_count = 0;

    std::vector<std::thread> readers;
    std::vector<std::thread> writers;
    std::map<std::string, size_t> map;
};

/* Single reader and many writers test
*/
TEST_F(MultiThreadTest, WritersTest) {

    LaunchWriters(&MultiThreadTest::writer,  big_queue);
    JoinWriters();

    EXPECT_EQ(big_queue.GetMessagesCount(), reader_target);

    big_queue.ShutDownWriters();

    std::map<std::string, size_t> map;

    for(int i=0; i < reader_target; ++i)
    {
        std::stringstream ss;
        big_queue.ReadTo([&ss](const char *msg, uint16_t size){  ss.write(msg, size);  });
        
        std::string s;
        ss >> s;
        ++map[s];
    }

    for (auto && [writer_id, amount] : map)
    {
        EXPECT_EQ(amount, amount_of_messages_each_writer_sends)
            << "Messages get lost due to a race condition";
    }
}

/* many readers, many writers test
*/
TEST_F(MultiThreadTest, ParallelTest) {
    LaunchReaders(&MultiThreadTest::reader, queue);
    LaunchWriters(&MultiThreadTest::writer, queue);

    while(reader_global_count < reader_target)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    queue.ShutDownWriters();
    queue.ShutDownReaders();

    JoinReaders();
    JoinWriters();

    EXPECT_EQ(map.size(), num_of_writers) 
        << "Map must contain only " << num_of_writers << " keys which are writers' tids.\r\n"
        << ((map.size() > num_of_writers)? "map size greater than a num of writers. Which could mean that messages' integrity was compromised!" : "");

    for (auto && [writer_id, amount] : map)
    {
        EXPECT_EQ(amount, amount_of_messages_each_writer_sends)
            << "Messages get lost due to a race condition";
    }
}
