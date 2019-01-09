#include <iostream>
#include <cstdlib>
#include <chrono>
#include <stdlib.h>
#include <functional>
#include <vector>
#include <thread>

#include <sched.h>
#include <pthread.h>

#include "kernels-x86_64.h"
#include "timed_run.hpp"

void set_affinity(std::thread& t, int i)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(i, &cpuset);
    int rc = pthread_setaffinity_np(t.native_handle(),
                                    sizeof(cpu_set_t), &cpuset);
    if (rc != 0)
        std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
}

int main(int argc, char ** argv)
{
    size_t nthreads = std::stoi(argv[1]);

    double time_per_run = 2.0;
    //std::vector<size_t> sizes({ 16, 32, 64, 128, 192, 256, 512, 1024, 2048,
    //                        4096, 8192, 16384, 32000, 64000, 128000, 256000});
    std::vector<size_t> sizes({100000});

    for (size_t& s : sizes) s *= 1024;

    for (size_t s : sizes)
    {
        std::vector<Buffer> buf(nthreads);
        // initialize buffers (first touch)
        for (int i = 0; i < nthreads; ++i)
        {
            std::thread t([=, &buf](){
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    buf[i] = Buffer(s);
                });
            set_affinity(t, i);
            //set_affinity(t, (i+18)%36); // allocate threads on socket 0 with mem on socket 1
            t.join();
        }

        std::function<void(size_t)> func = std::bind(&read_AVX, buf[0].get(), s, std::placeholders::_1);
        size_t loops = determine_loops(func, time_per_run);

        std::vector<std::function<void()>> tasks(nthreads);
        for (int i = 0; i < nthreads; ++i)
            tasks[i] = std::bind(&read_AVX, buf[i].get(), s, loops);

        std::vector<std::thread> workers(nthreads);
        auto t0 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < nthreads; ++i)
        {
            workers[i] = std::thread(tasks[i]);
            set_affinity(workers[i], i);
        }
            
        for (auto& t: workers)
            t.join();
        auto t1 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = t1 - t0; 
        double time = elapsed.count();

        std::cout << s/1024 << "K: "
                  << (nthreads*s*loops)/(1024*1024*1024) / time
                  << " GB/s" << " in " << time << "s" << std::endl;
    }
}
