#include <iostream>
#include <vector>
#include <iterator>

typedef unsigned long size_t;

void read_stride(double* src, size_t sz, size_t iterations, int stride)
{
    size_t nbytes = sizeof(double) * sz;
    stride *= 8;

    //while (iterations--)
    //{
        // rax: loop counter
        // %1 : loop bound
        // %2 : stride
        __asm__ __volatile__
                ("xor %%rax, %%rax\n\t"
                ".LL:\n\t"
                "movsd (%0, %%rax), %%xmm0\n\t"
                "addsd %%xmm0, %%xmm0\n\t"
                "movsd %%xmm0, (%0, %%rax)\n\t"
                "addq %%rbx, %%rax\n\t"
                //"imulq %%rbx, %%rax\n\t"
                "cmp %%rax, %1\n\t"
                "jne .LL\n\t"
                :
                :"r"(src), "r"(nbytes), "b"(stride)
                :"%rax", "%rcx", "cc"
                );
    //}

}

int main(int argc, char** argv)
{
    size_t sz = 64;
    
    //double* buf = new double[sz];
    //buf[1] = 1;

    std::vector<double> data(sz, 1);
    double* buf = data.data();

    int stride = std::stoi(argv[1]);
    std::cout << "stride " << stride << std::endl;

    read_stride(buf, sz, 1, stride);

    std::copy(std::begin(data), std::end(data), std::ostream_iterator<double>(std::cout, "\n"));
}
