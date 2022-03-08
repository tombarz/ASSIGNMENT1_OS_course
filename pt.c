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
        bitMask = createMask(a,b)>>13;
        index = bitMask & vpn;
        index = index >> (a+13);
        if ((node[index] & 1) == 0){
            return NO_MAPPING;
        } else{
            node = phys_to_virt(node[index]-1);
            a-=9;
            b-=9;
        }
    }
    bitMask = createMask(a,b)>>13;
    index = bitMask & vpn;
        //index = index >> a;
    uint64_t val = node[index];
    return node[index];
}
void cleanTree(uint64_t * addressesArray,int* indicesArray){
    for (int i = 4; i >0; i--) {
        uint64_t * node = phys_to_virt(addressesArray[i]);
        short entryExists = 0;
        for (int j = 0; j < 512; j++) {
            if (node[j]!=NO_MAPPING){
                entryExists=1;
                return;
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
short isNodeEmpty(uint64_t* node){
    for (int i = 0; i < 512; i++) {
        if (node[i]!=0){
            uint64_t val = node[i];
            return 0;
        }
    }
    return 1;
}
void initNode(uint64_t* node){
    for (int i = 0; i < 512; ++i) {
        node[i] = NO_MAPPING;
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
                uint64_t bitMask = createMask(a,b)>>13;
                uint64_t index = bitMask & vpn;
                index = index >> (a+13);
                if(isNodeEmpty(root)==1){
                    initNode(root);
                }
                if (root[index] == NO_MAPPING){
                    uint64_t PPN = alloc_page_frame();
                    root[index] = (PPN<<12)|1;/*PTE with valid bit*/
                }
                a-=9;
                b-=9;
                uint64_t  value = root[index];
                root = phys_to_virt(root[index]-1);
            }
            if(isNodeEmpty(root)==1){
                initNode(root);
            }
            uint64_t bitMask = createMask(a,b)>>13;
            uint64_t index = bitMask & vpn;
            //index = index >> a;
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
                uint64_t bitMask = createMask(a, b)>>13;
                uint64_t index = bitMask & vpn;
                index = index  >> (a+13);
                a -= 9;
                b -= 9;
                uint64_t nextAddress = root[index]-1;
                addressArray[i] = nextAddress;
                indices[i]=index;
                root = phys_to_virt(nextAddress);
            }
            uint64_t bitMask = createMask(a, b)>>13;
            uint64_t index = (bitMask & vpn);
            root[index]=NO_MAPPING;
            cleanTree(addressArray,indices);
            free_page_frame(root[index]);
        }
    }

}
