
#include <memory>
#include <string>
#include <chrono>
#include <functional>

template <class Callable>
size_t determine_loops(Callable const& func, double target_duration)
{
    // measure number of loops required
    size_t loops = 1;
    double elapsed = 0;
    do {
        loops *= 2;
        auto t0 = std::chrono::high_resolution_clock::now();
        func(loops);
        auto t1 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = t1 - t0;
        elapsed = duration.count();
    } while (elapsed < 0.1);
    loops *= target_duration/elapsed;

    return loops;
}

template <class Callable>
double timed_run(Callable const& func)
{
    auto t0 = std::chrono::high_resolution_clock::now();
    func();
    auto t1 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = t1 - t0;
    return duration.count();
}

class Buffer
{
public:
    Buffer() : data_size(0) {}

    explicit Buffer(size_t sz_) : data_size(sz_)
    {
        {
            void* buf;
            if (posix_memalign(&buf, 32, data_size)) {
                std::cout << "alloc failed\n"; exit(1);
            } 
            data_.reset((char*)buf);
        }

        init();
    }

    Buffer(Buffer&& rhs) : data_(std::move(rhs.data_)), data_size(rhs.data_size) {}

    Buffer& operator=(Buffer&& rhs) {
        data_ = std::move(rhs.data_);
        data_size = rhs.data_size;
        return *this;
    }

    void* get() const { return data_.get(); }

private:

    void init()
    {
        for (size_t i = 0; i < data_size/8; i++)
            ((size_t*)data_.get())[i] = 1;
    }

    std::unique_ptr<char> data_;
    size_t data_size;
};

void verify(void* buf)
{
    size_t *ibuf = (size_t*)buf;
    std::cout << "verify " << ibuf[0] << " " << ibuf[1] << " " << ibuf[2] << " " << ibuf[3] << std::endl;
}

