#include "fdt.h"
#include <stdio.h>



void fdt_parse(uint32_t *addr, fdt_struct_t* tree, void* (*mf)(size_t)) {

    tree->header = addr;


    tree->rsvmap = (fdt_reserve_entry_t*)((char*)tree->header + brl(tree->header->off_mem_rsvmap));
    tree->strblk_addr = (void*)((char*)tree->header + brl(tree->header->off_dt_strings));
    INIT_LIST_HEAD(&tree->node_head);
    INIT_LIST_HEAD(&tree->list);


    //* parse dtb

    uint32_t* ptr = (uint32_t*)((char*)tree->header + brl(tree->header->off_dt_struct)); 
    
    struct list_head* node_head = &tree->node_head;

    fdt_node_t* chnode_ptr = NULL;
    fdt_node_t* cur_node_ptr = NULL;
    fdt_prop_accessor_t* cur_prop_ptr = NULL;

    for(;;) {

        parsing:

        if(*ptr == brl(FDT_NOP)){
            ptr++;
            goto parsing;
        }
            

        if(*ptr == brl(FDT_BEGIN_NODE)){
            ptr++;
            goto create_node;
        }

        if(*ptr == brl(FDT_PROP)){
            ptr++;
            goto parse_prop;
        }

        if(*ptr == brl(FDT_END_NODE)){
            ptr++;
            goto end_node;
        }

        if(*ptr == brl(FDT_END)){
            goto end;
        }

        goto parsing;

        create_node:
            chnode_ptr = (fdt_node_t*) mf(sizeof(fdt_node_t));
            fdt_init_node(chnode_ptr);
            chnode_ptr->addr = ptr;
            if(*ptr != '\0'){
                chnode_ptr->node_name = ptr;
            } else
                chnode_ptr->node_name = "/";
            if(list_empty(node_head)) {
                list_add_tail(&chnode_ptr->list, node_head);
                chnode_ptr->parent_node = NULL;
                cur_node_ptr = chnode_ptr;
            } else {
                list_add_tail(&chnode_ptr->list, &cur_node_ptr->chnode_head);
                chnode_ptr->parent_node = cur_node_ptr;
                cur_node_ptr = chnode_ptr;
            }

            ptr = align_l(ptr, _strlen_null(chnode_ptr->node_name));
            goto parsing;


        parse_prop:
            cur_prop_ptr = (fdt_prop_accessor_t*) mf(sizeof(fdt_prop_accessor_t));
            fdt_init_prop(cur_prop_ptr, ptr);
            list_add_tail(&cur_prop_ptr->list, &cur_node_ptr->prop_head);
            ptr = (char*)ptr + sizeof(fdt_prop_t);
            ptr = align_l(ptr, brl(cur_prop_ptr->prop->len));

            goto parsing;
            


        end_node:
            if(cur_node_ptr->parent_node)
                cur_node_ptr = cur_node_ptr->parent_node;
            else
                goto parsing;

        end:
            break;


    }
}





void fdt_init_node(fdt_node_t* node) {

    node->addr = 0;
    node->node_name = NULL;
    node->parent_node = NULL;
    INIT_LIST_HEAD(&node->prop_head);
    INIT_LIST_HEAD(&node->chnode_head);
    INIT_LIST_HEAD(&node->list);

}

void fdt_init_prop(fdt_prop_accessor_t* prop_obj, void* prop_addr) {

    prop_obj->prop = prop_addr;
    INIT_LIST_HEAD(&prop_obj->list);

}


uint32_t brl(uint32_t netlong){

    uint32_t res = 0;

    res |= ((netlong & 0x000000ff) << 24) | ((netlong & 0xff000000) >> 24);
    res |= (((netlong & 0x00ff0000) >> 8)) | ((netlong & 0x0000ff00) << 8);

    return res;

}


uint32_t* align_l(uint32_t* addr, uint32_t len) {

    len = (len + 0x3) & (~0x3); 
    // len >>= 2;
    return (uint32_t*)((char*)addr + len);

}


uint32_t _strlen_null(char* str) {


    char* cur = str;
    uint32_t i = 0;
    while(*cur++ != '\0') i++;

    return (i+1);

}