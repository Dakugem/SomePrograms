#include <iostream>
#include <fstream>

#define MEMSIZE 1 << 24
#define MAX_ITERATIONS 1 << 10
#define ADDR(mem, i) (mem[i] << 16 | mem[i + 1] << 8 | mem[i + 2])
#define INIT_FILE "memory_init.hex"
#define OUTPUT_FILE "result.hex"

using namespace std;

struct
{
    uint32_t pc;
    uint8_t mem[MEMSIZE];
} vm;

bool mem_init(uint8_t *memory)
{
    fstream file(INIT_FILE, ios::in | ios::binary);

    if (file.is_open())
    {
        uint8_t temp;

        for (uint32_t i = 0; i < MEMSIZE; ++i)
        {
            temp = file.get();
            if(file.eof()) break;
            memory[i] = temp;
        }
    }
    else return false;

    file.close();

    return true;
}

bool save_result(uint8_t *memory)
{
    fstream file(OUTPUT_FILE, ios::out);

    if (file.is_open())
    {
        for (uint32_t i = 0; i < MEMSIZE; i++)
        {
            file << memory[i];
        }
    }
    else return false;

    file.close();

    return true;
}

int main()
{

    vm.pc = 0;

    if (!mem_init(vm.mem)) return -1;
    for (size_t i = 0; i < MAX_ITERATIONS; i++)
    {
        vm.mem[ADDR(vm.mem, vm.pc + 3)] = vm.mem[ADDR(vm.mem, vm.pc)];
        vm.pc = ADDR(vm.mem, vm.pc + 6);
    }

    if (!save_result(vm.mem)) return -1;

    return 0;
}