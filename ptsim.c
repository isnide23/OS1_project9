#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MEM_SIZE 16384  // MUST equal PAGE_SIZE * PAGE_COUNT
#define PAGE_SIZE 256  // MUST equal 2^PAGE_SHIFT
#define PAGE_COUNT 64
#define PAGE_SHIFT 8  // Shift page number this much

// Simulated RAM
unsigned char mem[MEM_SIZE];


// Convert a page,offset into an address
int get_address(int page, int offset)
{
    return (page << PAGE_SHIFT) | offset;
}

// Initialize RAM
void initialize_mem(void)
{
    // TODO
    memset(mem, 0, MEM_SIZE);
    mem[0] = 1;
}

// Allocate a physical page
// Returns the number of the page, or 0xff if no more pages available
unsigned char get_page(void)
{
    int i = 0;
    while (i < 64 && mem[i] == 1) i++;
    {
        if (i == 64) {
            return 0xff;
        }

        mem[i] = 1;
        return i;
    }
}

// Get the page table for a given process
unsigned char get_page_table(int process_number)
{
    return mem[process_number + 64];
}

// allocate memory for a new process
void set_page_table_entry(int page_table, int vpage, int page) {
    int pt_addr = get_address(page_table, vpage);

    mem[pt_addr] = page;
}

// Allocate pages for a new process
// This includes the new process page table and page_count data pages.
void new_process(int proc_num, int page_count)
{

    int page_table = get_page();

    // TODO error check ^^
    mem[64 + proc_num] = page_table;

    for (int i = 0; i < page_count; i++) {
        int new_page = get_page();
        
        int pt_addr = get_address(page_table, i);
        mem[pt_addr] = new_page;
    }
}

void deallocate_page(int page_number) {
    mem[page_number] = 0;
}

void kill_process(int process_number) {
    unsigned char page_table = get_page_table(process_number);

    int pt_addr = get_address(page_table, 0);

    for (int i = 0; i < 64; i++) {
        if (mem[pt_addr + i] != 0) {
            deallocate_page(mem[pt_addr + i]);
        }
    }

    deallocate_page(page_table);
}

int get_physical_address (int process_number, int virtual_address) {
    int virtual_page = virtual_address >> 8;
    int offset = virtual_address & 255;

    int page_table = get_page_table(process_number);
    int pt_addr = get_address(page_table, virtual_page);
    int physical_page = mem[pt_addr];

    int physical_page_address = get_address(physical_page, offset);

    return physical_page_address;
}


// LoadValue(proc_num, virt_addr):
//     phys_addr = GetPhysicalAddr(proc_num, virt_addr)
//     value = mem[phys_addr]
void load_value(int process_number,int virtual_addr) {
    int physical_address = get_physical_address(process_number, virtual_addr);

    int value = mem[physical_address];

    printf("Load proc %d: %d => %d, value=%d\n",
    process_number, virtual_addr, physical_address, value);
}

void store_value(int process_number,int virtual_addr, unsigned char value) {
    int physical_address = get_physical_address(process_number, virtual_addr);

    mem[physical_address] = value;

    printf("Store proc %d: %d => %d, value=%d\n",
    process_number, virtual_addr, physical_address, value);
}



// Print the free page map
void print_page_free_map(void)
{
    printf("--- PAGE FREE MAP ---\n");

    for (int i = 0; i < 64; i++) {
        int addr = get_address(0, i);

        printf("%c", mem[addr] == 0? '.': '#');

        if ((i + 1) % 16 == 0)
            putchar('\n');
    }
}

// Print the address map from virtual pages to physical
void print_page_table(int proc_num)
{
    printf("--- PROCESS %d PAGE TABLE ---\n", proc_num);

    // Get the page table for this process
    int page_table = get_page_table(proc_num);

    // Loop through, printing out used pointers
    for (int i = 0; i < PAGE_COUNT; i++) {
        int addr = get_address(page_table, i);

        int page = mem[addr];

        if (page != 0) {
            printf("%02x -> %02x\n", i, page);
        }
    }
}

// Main -- process command line
int main(int argc, char *argv[])
{
    assert(PAGE_COUNT * PAGE_SIZE == MEM_SIZE);

    if (argc == 1) {
        fprintf(stderr, "usage: ptsim commands\n");
        return 1;
    }
    
    initialize_mem();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "np") == 0) {
            int proc_num = atoi(argv[++i]);
            int pages = atoi(argv[++i]);
            new_process(proc_num, pages);
        }
        else if (strcmp(argv[i], "pfm") == 0) {
            print_page_free_map();
        }
        else if (strcmp(argv[i], "ppt") == 0) {
            int proc_num = atoi(argv[++i]);
            print_page_table(proc_num);
        }
        else if (strcmp(argv[i], "kp") == 0) {
            int proc_num = atoi(argv[++i]);
            kill_process(proc_num);
        }
        else if (strcmp(argv[i], "lb") == 0) {
            int proc_num = atoi(argv[++i]);
            int virt_addr = atoi(argv[++i]);
            load_value(proc_num, virt_addr);
        }
        else if (strcmp(argv[i], "sb") == 0) {
            int proc_num = atoi(argv[++i]);
            int virt_addr = atoi(argv[++i]);
            int value = atoi(argv[++i]);
            store_value(proc_num, virt_addr, value);
        }
    }
}
