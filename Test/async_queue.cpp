#include "AsyncQueue.h"
#include "Times.h"
#include <thread>
#include <iostream>
//g++ -o aqueue async_queue.cpp ../Public/Times.cpp -I../Data -I../Public -lpthread -latomic --std=c++11
int total = 100000;
int main(){
    Stone::AsyncQueue<int> queue;
    Stone::Tick tick;
    std::thread t1([&](){
        for(int i = 0; i < total; ++i)
        {
            while(!queue.BlockingPut(10))
                std::cout << "t1" << std::endl;
        }
    });
    std::thread t2([&](){                                     
        for(int i = 0; i < total; ++i)                     
        {
            while(!queue.BlockingPut(10))
               std::cout << "t2" << std::endl;
        }
    });
    std::thread t3([&](){
        for(int i = 0; i < total; ++i)
        {
            while(!queue.BlockingPut(10))
               std::cout << "t3" << std::endl ;
        }
    });
    std::thread t4([&](){
        for(int i = 0; i < total; ++i)
        {
            while(!queue.BlockingPut(10))
                std::cout << "t4" << std::endl;
        }
    });

   std::thread t5([&](){
        for(int i = 0; i < total; ++i)
        {
            int num = 0;
            while(!queue.BlockingPut(num))
                std::cout << "t5" << std::endl;
        }
    });
    std::thread t6([&](){
        for(int i = 0; i < 5*total; ++i)
        {
            int num = 0;
            while(!queue.SingleGet(num))
            {
		if(i%10000 == 0)
                    std::cout << "t6:" << i << std::endl;
            }
                ;
        }
    });

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    //t7.join();
    int64_t time = tick.Elapsed();
    std::cout << time << std::endl;
    return 0;
}
