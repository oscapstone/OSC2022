#include "dtb.h"
#include "mini_uart.h"
#include "utils.h"
#include "string.h"
#include "cpio.h"

void fdt_traverse(void (*f)()){
    register unsigned long dtb_reg asm ("x15");
    writes_uart("Loaded dtb address: ");
    writehex_uart(dtb_reg,1);
    fdt_header* header = (fdt_header*)dtb_reg;
    
    if(big2little(header->magic) != FDT_HEADER_MAGIC){
        writes_uart("Header magic failed\r\n");
        return;
    }
        
    uint32_t* struct_start = (uint32_t*)((char*)header+ big2little(header->off_dt_struct));
    //uint32_t* string_start = (uint32_t*)((char*)header+ big2little(header->off_dt_strings));
    fdt_prop* prop;
    char *name;
    // writehex_uart(struct_start);
    // writes_uart("\r\n");
    // writehex_uart(big2little(header->totalsize));
    // writes_uart("\r\n");
    

    while(1){
        
        uint32_t tok = big2little(*struct_start);
        switch (tok)
        {
        case FDT_BEGIN_NODE:
            //writes_uart("Begin node\r\n");
            
            name = (char*)(struct_start+1);
            // followed node name

            // writes_n_uart(name,strlen(name));
            // writes_uart("\r\n");
            // writehex_uart(strlen(name));
            // writes_uart("\r\n");
            uint32_t name_size = strlen(name);

            struct_start+=align_up(name_size+1,4)/sizeof(uint32_t);
            // name have zero padding, aligned up to multiple of 4.

            break;
        case FDT_END_NODE:
            // writes_uart("End node\r\n");
            break;
        case FDT_PROPERTY:
            // // writes_uart("PROP\r\n");
            prop = (fdt_prop*)(struct_start+1);
            // // followed  property information

            name = ((char*)header+big2little(header->off_dt_strings)+ big2little(prop->nameoff));
            
            // writes_n_uart(name,big2little(prop->len));
            // writes_uart("\r\n");
            // writes_uart(name);
            // writes_uart("\r\n");
            // writehex_uart(big2little(prop->len));
            // writes_uart("\r\n");
            f(prop,name,big2little(prop->len));

            struct_start+=(sizeof(fdt_prop)+align_up(big2little(prop->len),4))/sizeof(uint32_t); 
            // add size of property name length and name.
            // property name have zero padding, up to multiple of 4.
            break;
        case FDT_END:
            //writes_uart("End\r\n");
            return;
            break;
        case FDT_NOP:
            //writes_uart("NOP\r\n");
            //struct_start+= 1;
            break;
        default:
            //struct_start+= 1;
            //writehex_uart(tok);
            break;
        }
        struct_start+=1;
    }

}