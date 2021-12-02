#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// defined constants
#define SUCCESS 0
#define NOT_VALID -1
#define NOT_ACCESSIBLE -2
#define VALID_MASK 0b01
#define ACCESS_MASK 0b10
#define PFN_SHIFT 12

// globl variables for MMU
unsigned int *page_table = NULL;
unsigned int vpn_mask = 0;
unsigned int shift = 0;
unsigned int offset_mask = 0;

// function declaration
void alloc_page_table(int address_space_bits, int page_bytes);
void init_page_table(int address_space_bits, int page_bytes);
void init_mmu_variables(int address_space_bits, int page_bytes);
int mmu_address_translation(unsigned int virtual_address, unsigned int *physical_address);

/* This function allocates memory space for page_table, which is a global variable. 
   First, calculate the size of the page table.
   Then, allocate dynamic memory for the page table by malloc() with the size of the page table * the size of PTE.
   Third, assign the return value of malloc() into page_table.
   Finally, initialize the allocated memory with zero.
*/
void alloc_page_table(int address_space_bits, int page_bytes)
{
    /* Please implement your own code below */


    int pte_size = 4;
    int page_table_size = pow(2, address_space_bits - (int)log2(page_bytes));
    page_table = (int*)malloc(page_table_size * pte_size);
    memset(page_table, 0, page_table_size * pte_size);

}

/* This function inserts initial PTEs into the page_table.
   It will fill only half of the page table.
   VPN n is mapped to PFN n*2.
   When the index of the page table can be divided by 4, the PTE becomes inaccessible.
   Please do not modify this function.
*/
void init_page_table(int address_space_bits, int page_bytes)
{
    unsigned int i;
    int page_table_size = pow(2, address_space_bits - (int)log2(page_bytes));

    /* fill the page table only half */
    for (i = 0; i < page_table_size / 2; i++)
    {
        unsigned int *pte = page_table + i;
        *pte = (i * 2) << PFN_SHIFT;
        if (i % 4 == 0)
            *pte = *pte | VALID_MASK; // make this pte as valid and inaccessible
        else
            *pte = *pte | VALID_MASK | ACCESS_MASK; // make this pte as valid and accessible
    }
}

/* This function initializes the global variables for the mmu_address_translation() function. 
   For each meaning, please refer to Page 13 of the lecture slide of Chapter 18.
   Please do not modify this function.
*/
void init_mmu_variables(int address_space_bits, int page_bytes)
{
    vpn_mask = (0xffffffff >> (int)log2(page_bytes)) << (int)log2(page_bytes);
    shift = (int)log2(page_bytes);
    offset_mask = 0xffffffff >> (sizeof(offset_mask) * 8 - (int)log2(page_bytes));
}

/* This function performs address translation from virtual to physical.
   To implement this function, please refer to Page 13 of the lecture slide of Chapter 18.
   If successful, the function copies the translated address into the physical_address variable and returns SUCCESS.
   The function returns NOT_VALID if the PTE is not valid.
   The function returns NOT_ACCESSIBLE if the PTE is not accessible.
*/
int mmu_address_translation(unsigned int virtual_address, unsigned int *physical_address)
{
    /* Please implement your own code below */

    unsigned int vpn;
    unsigned int pfn;
    int valid;
    int access;
    char* ptbr;
    int* pteAddr;
    int pte;
    int offset;

    

    vpn = virtual_address & vpn_mask >> shift;
    pteAddr = ptbr + (vpn * sizeof(int));
    pte = *pteAddr;

    pfn = pte & 0xfffff000;

    if(pte & 1 == 0){
        return NOT_VALID;
    } else if(pte & 3 == 1){
        return NOT_ACCESSIBLE;
    } else {
        offset = virtual_address & offset_mask;
        physical_address = (pfn << shift) | offset; 
    }
    




    // printf(" (vpn:%u, pfn: %u, valid: %d, access: %d) ", vpn, pfn, valid, access);

    return SUCCESS;
}

// Please do not modify the main() function.
int main(int argc, char *argv[])
{
    printf("Welcome to Software-managed MMU\n");

    if (argc != 3)
    {
        printf("Usage: ./mmu [address_space_size_in_bits] [page_size_in_bytes]\n");
        exit(1);
    }

    int address_space_bits = atoi(argv[1]);
    int page_bytes = atoi(argv[2]);

    if (address_space_bits < 1 || address_space_bits > 32)
    {
        printf("address_space_bits shoud be between 1 and 32\n");
        exit(1);
    }

    if (page_bytes < 1 || page_bytes > 4096)
    {
        printf("page_bytes shoud be between 1 and 4096\n");
        exit(1);
    }

    alloc_page_table(address_space_bits, page_bytes);

    if (page_table == NULL)
    {
        printf("Please allocate the page table with malloc()\n");
        exit(1);
    }

    init_page_table(address_space_bits, page_bytes);
    init_mmu_variables(address_space_bits, page_bytes);

    while (1)
    {
        unsigned int value;
        printf("Input a virtual address of hexadecimal value without \"0x\" (-1 to exit): ");
        scanf("%x", &value);

        if (value == -1)
            break;

        printf("Virtual address: %#x", value);

        unsigned int physical_address = 0;
        int result = mmu_address_translation(value, &physical_address);
        if (result == NOT_VALID)
        {
            printf(" -> Segmentation Fault.\n");
        }
        else if (result == NOT_ACCESSIBLE)
        {
            printf(" -> Protection Fault.\n");
        }
        else if (result == SUCCESS)
        {
            printf(" -> Physical address: %#x\n", physical_address);
        }
        else
        {
            printf(" -> Unknown error.\n");
        }
    }

    if (page_table != NULL)
        free(page_table);

    return 0;
}