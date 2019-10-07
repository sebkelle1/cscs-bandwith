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

int get_affinity(std::thread& t)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    int rc = pthread_getaffinity_np(t.native_handle(),
                                    sizeof(cpu_set_t), &cpuset);
    if (rc != 0)
        std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
    int ret;
    for (int i=0; i < 128; ++i)
        if (CPU_ISSET(i, &cpuset)) return i;

    return -1;
}

int main(int argc, char ** argv)
{
    size_t s = std::stoi(argv[1]);
    int stride = std::stoi(argv[2]);
    int nthreads = std::stoi(argv[3]);

    double time_per_run = 2.0;

    std::vector<Buffer> buf(nthreads);
    // initialize buffers (first touch)
    for (int i = 0; i < nthreads; ++i)
    {
        std::thread t([=, &buf](){
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                buf[i] = Buffer(s);
            });
        set_affinity(t, i);
        //std::cout << "thread " << i << " on core " << get_affinity(t) << std::endl;
        //set_affinity(t, (i+18)%36); // allocate threads on socket 0 with mem on socket 1
        t.join();
    }

    std::function<void(size_t)> func = std::bind(&read_stride, buf[0].get(), s, std::placeholders::_1, stride);
    size_t loops = determine_loops(func, time_per_run);

    std::vector<std::function<void()>> tasks(nthreads);
    for (int i = 0; i < nthreads; ++i)
        tasks[i] = std::bind(&read_stride, buf[i].get(), s, loops, stride);

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
              << (double(nthreads*s*loops)/stride)/(1024*1024*1024) / time
              << " GB/s" << " in " << time << "s" << std::endl;
}
