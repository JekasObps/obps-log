#include <iostream>
#include <thread>
#include <chrono>

#include "obps_log.hpp"
#include "log_queue.hpp"

////////////////////////////// test driver /////////////////////////////////////
using namespace obps;

bool create_queue()
{
    // LogQueue<0> queue(0); // forbidden, static_assert
    // LogQueue<1> queue(0); // forbidden, assert
    LogQueue<1, 500> queue(1);    // minimal size queue case

    return true;
}

bool single_read_write()
{
    LogQueue<1, 0> queue(1);

    char msg, result;

    msg = 'A';

    queue.Write(&msg, 1);
    queue.Read(&result);

    return !strncmp(&msg, &result, 1);
}

bool write_to_file()
{
    std::thread reader;

    LogQueue<16, 0> queue(10);
    const char *msg[3] = {"ABCabcXYZxyz000_", "000111222333444_", "!@#$%^&*_!@#$%^_"};
    
    reader = std::thread { 
        [&queue]{ 
        std::ofstream out("test.log");
        while (queue.isOpen() || queue.isReadAvailable())
        {
            queue.ReadToFile(out); out << std::endl;
        }
    }};

    for(int i = 0; i < 100; ++i)
        for (auto m : msg) queue.Write(m, 16); 

    queue.Close();

    reader.join();
    return true;
}

bool test_queue()
{
    LogQueue<256, 0> queue(10000);

    char in[256], out[256];

    std::thread t1([&](){
        for (int i = 0; i < 10; ++i)
        {
            queue.Read(out);
            std::cerr << out << '\n';
        }
    });

    std::thread t2([&](){
        for (int i = 0; i < 10; ++i)
        {
            int b = sprintf(in, " t2 no: %d ", i);
            std::memset(in + b, 'B', 256 - b); in[255] = 0;
            queue.Write(in, 256);
            std::this_thread::yield();
        }
    });

    for (int i = 0; i < 10; ++i)
    {
        int b = sprintf(in, " t1 no: %d ", i);
        std::memset(in + b, 'A', 256 - b); in[255] = 0;
        queue.Write(in, 256);
        std::this_thread::yield();
    }

    t2.join();
    t1.join();
    return true;
}

struct test_data 
{
    const char* name; 
    bool(*func)(); 
};

#define TEST(f) test_data{#f, f}

int main()
{
    for ( auto [name, func] : { 
        TEST(create_queue),  
        TEST(single_read_write),
        TEST(write_to_file),
        TEST(test_queue)
    }) 
    {
        std::cerr << name;
        (!func()) ? std::cerr <<  ":: FAILED! " : std::cerr << ":: PASSED! ";
        std::cerr << std::endl;
    }
}