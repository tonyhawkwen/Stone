#include "AsyncQueue.h"
#include "Times.h"
#include <thread>
#include <iostream>
//g++ -o aqueue async_queue.cpp ../Public/Times.cpp -I../Data -I../Public -lpthread -latomic --std=c++11
int total = 1000000;
int main(){
    Stone::AsyncQueue<int> queue;
    Stone::Tick tick;
    std::thread t1([&](){
        for(int i = 0; i < total; ++i)
        {
            while(!queue.BlockingPut(10))
                ;
        }
    });
    std::thread t2([&](){                                     
        for(int i = 0; i < total; ++i)                     
        {
            while(!queue.BlockingPut(10))
			;
        }
    });
    std::thread t3([&](){
        for(int i = 0; i < total; ++i)
        {
            while(!queue.BlockingPut(10))
			;
        }
    });

    std::thread t6([&](){
        for(int i = 0; i < 3*total; ++i)
        {
            int num = 0;
            while(!queue.SingleGet(num))
                ;
        }
    });

    t1.join();
    t2.join();
    t3.join();
    t6.join();
    //t7.join();
    int64_t time = tick.Elapsed();
    std::cout << time << std::endl;
    return 0;
}
