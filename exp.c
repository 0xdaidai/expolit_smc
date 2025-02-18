#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#define CMD_SMC 0x13370001
#define CMD_READPHY 0x13370002
#define DRIVER_PATH "/dev/etx_device"

typedef struct
{
    unsigned long long int smc_fid;
    unsigned long long int x1;
    unsigned long long int x2;
    unsigned long long int x3;
} SmcArgs;

typedef struct
{
    size_t addr; // 用于存储物理地址
    size_t size; // 用于存储读取的大小
    size_t recv_addr;
} Arb_read_arg;
typedef size_t UINT64 ;
typedef unsigned int UINT32;
typedef unsigned char UINT8;
typedef void *PVOID;
typedef int BOOL;
#define PAGE_SIZE 0x1000
#define PRO_KEY 0xdeadbeef
int devfd;
//print_memory(cur_address, (char *)physmem_addr, auxSize);

void print_memory(UINT64 address, char *buffer, UINT32 size)
{
    UINT32 i, j;
    unsigned char *ptr = (UINT32 *)buffer;
    for (i = 0; i < size / 4; i++)
    {
        if (i % 4 == 0)
        {
            printf("\n%p: ", (void *)(address + i * 4));
        }
        for(int j = 0;j < 4;j++)
            printf("%2x ", ptr[i+j]);
    }
    printf("\n");
}
void read_physical_memory(UINT64 address, UINT32 size, PVOID buffer, BOOL bPrint)
{
    if (size > 0)
    {

        int i = 0;
        UINT32 auxSize = size;
        UINT64 cur_address;

        while (auxSize > PAGE_SIZE)
        {
            cur_address = address + (i * PAGE_SIZE);

            void *physmem_addr = mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, devfd, cur_address);

            if (physmem_addr != MAP_FAILED)
            {
                printf("-> %08x bytes read from physical memory %p\n", PAGE_SIZE, cur_address);
                if (buffer)
                {
                    memcpy((char *)buffer + (i * PAGE_SIZE), physmem_addr, PAGE_SIZE);
                }
                if (bPrint)
                {
                    print_memory(cur_address, (char *)physmem_addr, PAGE_SIZE);
                }
                munmap(physmem_addr, PAGE_SIZE);
            }

            auxSize = auxSize - PAGE_SIZE;
            i++;
        }

        cur_address = address + (i * PAGE_SIZE);

        void *physmem_addr = mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, devfd, cur_address);

        if (physmem_addr != MAP_FAILED)
        {
            printf("-> %08x bytes read from physical memory %p\n", PAGE_SIZE, cur_address);
            if (buffer)
            {
                memcpy((char *)buffer + (i * PAGE_SIZE), physmem_addr, auxSize);
            }

            if (bPrint)
            {
                print_memory(cur_address, (char *)physmem_addr, auxSize);
            }

            munmap(physmem_addr, PAGE_SIZE);
        }
    }
}

int main()
{
    devfd = open(DRIVER_PATH, 2);
    int status = 0;
    SmcArgs smc_arg = {
        0x1000, 0x2000, 0x20, 0};
    printf("SMC_CALL\n");
    status = ioctl(devfd, CMD_SMC, &smc_arg);
    printf("status:%d\n", status);
    size_t *buf = malloc(0x3000);
    memset(buf, 0, 0x2000);
    read_physical_memory(0x3000, 0x200, buf, 1);
    return 0;
}