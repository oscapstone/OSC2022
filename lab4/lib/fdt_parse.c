#include "lib/fdt_parse.h"

// callback type for fdt_parser
// void* (*)(fdt_node* node, fdt_property* prop)

static fdt_header header;
void fdt_parse_header(uint8_t* fdt, fdt_header* pheader){
    int i = 0;
    uint32_t *ph = (uint32_t*)pheader, *pfdt = (uint32_t *)fdt;

    for(i = 0; i < sizeof(fdt_header) / 4 ; i++){
        uint32_t tmp = pfdt[i];
        ph[i] = bswap32(tmp); 
    }
}

void fdt_parser(uint8_t* fdt, fdt_callback callback){
    uint32_t finished = 0;
    int32_t layer = -1; // To address recursive structure 
    uint32_t count = 0;
    uint32_t token = 0;
    uint32_t *pdt_struct;
    uint8_t  *pdt_strings;
    fdt_node node;
    fdt_property prop;
    uint32_t len;
    uint32_t str_offset;

    fdt_header * pheader = &header;
    fdt_parse_header(fdt, pheader);

    pdt_struct = (uint32_t *)(fdt + pheader->off_dt_struct);
    pdt_strings = fdt + pheader->off_dt_strings;

    // start parsing structure
    while(!finished){
        token = pdt_struct[count];
        count += 1;
        switch(token){
            case FDT_BEGIN_NODE:
                layer += 1;

                node.name = (char*)&pdt_struct[count];

                len = strlen(node.name);
                len = ALIGN_UP(len + 1, 4) / 4;
                count += len;

                callback(token, &node, NULL, layer);
                break;
            case FDT_END_NODE:
                callback(token, NULL, NULL, layer);
                layer--;
                break;
            case FDT_PROP:
                prop.value_len = bswap32(pdt_struct[count]);
                str_offset = bswap32(pdt_struct[count + 1]);
                prop.name = pdt_strings + str_offset;
                prop.value = (uint8_t*)&pdt_struct[count + 2];

                len = ALIGN_UP(prop.value_len, 4) / 4;
                //printf("prop.value_len: %u, prop.name: %s, len: %u\n", prop.value_len, prop.name, len);
                len += 2;

                count += len;
                callback(token, NULL, &prop, layer);
                break;
            case FDT_NOP:
                callback(token, NULL, NULL, layer);
                break;
            case FDT_END: 
                callback(token, NULL, NULL, layer);
                finished = 1;
                break;
            default:
                //printf("Parsing error!!\n");
        }
    }
}
void fdt_parse_rsvmap(uint8_t* fdt, fdt_rsvmap_callback callback){
    uint64_t *mem_rsvmap;
    uint64_t start, end;
    uint64_t count = 0;
    fdt_header * pheader = &header;
    fdt_parse_header(fdt, pheader);

    mem_rsvmap = (uint64_t*)(fdt + pheader->off_mem_rsvmap);
    do{
        start = bswap64(mem_rsvmap[count * 2]);
        end = start + bswap64(mem_rsvmap[count * 2 + 1]);
        if(end)
            callback(start, end);
        else
            break;
        count++;
    }while(1);
}
void* fdt_print_callback(uint32_t token, fdt_node* node, fdt_property* prop, int32_t layer){
    if(node != NULL){
        for(uint32_t i = 0 ; i < layer * 4 ; i++){
            printf(" ");
        }
        printf("%s {\n", node->name); 
    }else if(prop != NULL){
        for(uint32_t i = 0 ; i < layer * 4 + 4; i++){
            printf(" ");
        }
        printf("%s ;\n", prop->name); 
    }else{
        switch(token){
            case FDT_END_NODE:
                for(uint32_t i = 0 ; i < layer * 4 ; i++){
                    printf(" ");
                }
                printf("}\n"); 
                break;
            case FDT_NOP:
                break;
            case FDT_END: 
                break;
            default:
                break;
        }
    }

} 
