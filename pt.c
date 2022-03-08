#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "os.h"

//code from stackoverflow
uint64_t createMask(uint64_t a, uint64_t b)
{
    uint64_t r = 0;
    uint64_t oneSixtfourBit = 1;
    for (uint64_t i=a; i<=b; i++)
        r |= oneSixtfourBit << i;

    return r;
}
uint64_t alloc_page_frame(void);
void free_page_frame(uint64_t ppn);
void* phys_to_virt(uint64_t phys_addr);
uint64_t page_table_query(uint64_t pt, uint64_t vpn);
void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn);

uint64_t page_table_query(uint64_t pt, uint64_t vpn){
    uint64_t * node = phys_to_virt(pt);
    uint64_t a = 48;
    uint64_t b = 56;
    uint64_t bitMask;
    uint64_t index = 0;
    for (int i = 0; i < 4; i++) {
        bitMask = createMask(a,b);
        index = bitMask & vpn;
        index = index >> a;
        if ((node[index] & 1) == 0){
            return NO_MAPPING;
        } else{
            node = phys_to_virt(node[index]);
            a-=9;
            b-=9;
        }
    }
    bitMask = createMask(a,b);
    index = bitMask & vpn;
    index = index >> a;
    return node[index];
}
void cleanTree(uint64_t * addressesArray,int* indicesArray){
    for (int i = 4; i >0; i--) {
        uint64_t * node = phys_to_virt(addressesArray[i]);
        short entryExists = 0;
        for (int j = 0; j < 512; j++) {
            if (node[j]!=NO_MAPPING){
                entryExists=1;
            }
        }
        if (entryExists==0){
            free(node);
            uint64_t* parent = phys_to_virt(addressesArray[i-1]);
            parent[indicesArray[i-1]] = NO_MAPPING;
        }else{
            return;
        }

    }
}
void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn){
    uint64_t current_value = page_table_query(pt,vpn);
    if (current_value==NO_MAPPING){
        if (ppn==NO_MAPPING){
            return;
        } else{
          uint64_t* root = phys_to_virt(pt);
            int a = 48;
            int b = 56;
            for (int i = 0; i < 4; i++) {
                uint64_t bitMask = createMask(a,b);
                uint64_t index = bitMask & vpn;
                index = index >> a;
                uint64_t PPN = alloc_page_frame();
                root[index] = (PPN<<12)|1;/*PTE with valid bit*/
                a-=9;
                b-=9;
                root = phys_to_virt(root[index]);
            }
            uint64_t bitMask = createMask(a,b);
            uint64_t index = bitMask & vpn;
            index = index >> a;
            root[index] = ppn;
            return;
        }
    }else{
        if (ppn==NO_MAPPING){
            uint64_t* root = phys_to_virt(pt);
            int a = 48;
            int b = 56;
            uint64_t * addressArray = calloc(5, sizeof(uint64_t));
            addressArray[0] = pt;
            int * indices = calloc(4, sizeof(int));
            for (int i = 0; i < 4; i++) {
                uint64_t bitMask = createMask(a, b);
                uint64_t index = bitMask & vpn;
                index = index  >> a;
                a -= 9;
                b -= 9;
                uint64_t nextAddress = root[index];
                addressArray[i] = root[index];
                indices[i]=index;
                root = phys_to_virt(nextAddress);
            }
            uint64_t bitMask = createMask(a, b);
            uint64_t index = (bitMask & vpn) >> a;
            root[index]=NO_MAPPING;
            cleanTree(addressArray,indices);
            free_page_frame(root[index]);
        }
    }

}
