#include "kernel/shell.h"

char *miku_ascii = "⠀⠀⠀⠀⠀⠀⢀⡤⣢⠟⢁⣴⣾⡿⠋⢉⠱⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⠑⠒⠦⢄⣀⣴⠟⢡⣠⣼⣿⡿⢳⣄⡀⠀⠀\r\n⠀⠀⠀⠀⠀⢀⣾⡿⠃⣠⣿⣿⠿⠂⠀⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢲⡿⠋⢰⣾⣿⣿⡟⠀⠀⠈⠙⢆⠀\r\n⠀⠀⠀⠀⠀⡜⠻⣷⣾⣿⠟⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⣿⣁⣰⢸⣿⢻⠟⢀⠀⠀⠀⠀⠀⠁\r\n⠀⠀⠀⠀⠰⠀⠀⢙⡿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⣿⣿⣿⣿⣯⡀⠀⢃⠀⠀⠀⠀⠀\r\n⠀⠀⠀⠀⠀⠀⢠⠎⠀⠀⠀⠀⠀⠀⣼⠀⢀⠀⠀⠀⠀⠀⢠⣷⡀⠀⠀⠀⠀⡀⠄⠀⠀⠀⠀⢻⣿⣿⣿⣧⠑⠀⣢⡄⠀⠀⠀⠀\r\n⠀⠀⠀⠀⠀⡰⠃⢀⠄⠀⠀⠀⠀⣼⡿⡆⢸⠀⠀⠀⠀⠀⠈⣿⢷⡄⠀⠀⠀⠱⡀⠰⡀⠀⠀⠈⢿⣿⣿⣿⣧⠀⢸⣧⠀⠀⠀⠀\r\n⠀⠀⠀⠀⠀⠡⢢⠋⠀⠀⠀⠀⣼⡟⠀⣇⢸⡆⠀⠀⠀⡄⠀⢿⠀⢳⡄⠀⠀⠀⢳⠀⢳⠀⠀⠀⠈⣿⣿⣿⣿⣷⣘⡟⠀⠀⠀⠀\r\n⠀⠀⠀⠀⠀⢀⠇⠀⠀⠀⠀⣸⡿⢤⠤⠸⡸⣷⠀⠀⠀⢱⠀⣾⡤⠤⢿⡤⢀⡀⠀⢧⠘⡆⠀⠀⠀⢸⡟⠻⢿⠟⣿⣷⡄⠀⠀⠀\r\n⠀⠀⠀⠀⠀⡞⠀⠀⠀⠀⢰⡿⢠⠇⠀⠀⢳⣿⢇⠀⠀⠈⡇⣿⡇⠀⠀⠻⣄⠀⠀⠘⡆⡇⠀⠀⠀⠀⣇⢀⡏⠀⣿⡿⣄⠀⠀⠀\r\n⠀⠀⠀⠀⢰⠁⠀⠀⠀⠀⣿⠁⣄⣀⣀⡀⠈⢿⡜⡄⠀⠀⢹⣿⡇⠐⢄⣀⠘⢧⡀⠀⠹⣿⠀⠀⠀⠀⢸⣿⣷⣶⣿⡇⢹⡇⠀⠀\r\n⠀⠀⠀⠀⠾⠀⠀⠀⠀⢸⣧⣾⠟⢉⣽⣿⣦⠈⢷⡘⣆⠀⠸⡟⣷⣶⠟⠛⢻⣷⣦⣀⠀⢻⠀⠀⠀⠀⢸⣏⣩⣼⣿⡇⠈⣷⠀⠀\r\n⠀⠀⠀⠃⠀⠀⠀⠀⠀⣿⡿⠁⠀⣠⣾⣿⣿⠀⠈⢿⠺⡆⠀⣧⢸⠀⠀⢀⣹⣿⣿⣿⣷⣼⣤⠀⠀⠀⢸⣿⣿⣿⣿⠀⠀⣿⠀⠀\r\n⠀⠀⣠⠄⣀⠀⠀⠀⢠⣿⡇⠀⠀⢻⢻⣟⢿⠀⠀⠈⠣⠈⠓⠾⠀⠀⠀⣿⣿⢿⣿⣿⠘⡇⡞⠀⠀⢠⣾⣿⣿⣿⡏⠀⠀⢹⠀⠀\r\n⠀⠀⠛⠀⣿⠀⠀⠀⢸⣿⣿⡀⠀⠈⠃⠐⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸⣄⣐⣠⠏⢠⣿⠁⠀⠀⢸⣿⣿⣿⣿⠀⠀⠀⢸⠀⠀\r\n⠀⠀⠀⠀⢹⡆⠰⡀⢸⡟⠩⣷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⠃⠀⠀⠀⢸⣿⣿⣿⠟⠀⠀⠀⠘⠀⠀\r\n⠀⠀⠀⠀⢎⣿⡀⢱⢞⣁⣀⡿⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⠞⡏⡼⠀⠀⠀⣾⣿⠋⠁⠀⠀⠀⠀⠀⠀⠀\r\n⠀⠀⠀⠀⠈⠿⠻⡇⠀⠀⠒⠢⢵⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⣽⠁⠀⠀⢠⡿⢹⠀⠀⠀⠀⠀⠀⠀⠀⠀\r\n⡀⠀⠀⠀⠀⠀⠀⡟⣦⡀⠀⠀⠀⠈⠓⢄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣴⢿⡇⠀⠀⡄⣸⣇⣼⣀⣀⣀⠀⠀⠀⠀⠀⠀\r\n⡀⠀⠀⠀⠀⠀⢰⠇⣿⢸⣦⡀⠀⠀⠀⠀⠈⠲⣄⡀⠀⠀⠀⠀⠀⣀⡤⠒⢉⡴⠃⣸⠀⠀⢰⣿⣿⣿⠃⡤⠊⠁⠉⠑⢄⠀⠀⠀\r\n⡇⠀⠀⠀⠀⠀⢸⠀⣿⣾⣿⢿⠲⣄⠀⠀⠀⠀⠘⠟⣦⣤⣴⡒⠉⢀⡠⠖⠉⠀⣠⠃⠀⣠⣿⣿⡿⠁⠊⠀⠀⠀⠀⠀⠀⠀⠀⠀\r\n⠀⠀⠀⠀⠀⠀⢸⠀⣿⠛⢿⠈⢢⠏⠀⠀⠀⠀⠀⣰⣏⣀⣿⠗⠊⠁⠀⠀⣠⣾⠃⢀⡴⠿⠛⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\r\n⠀⠀⠀⠀⠀⠀⢸⢀⠇⠀⠈⢠⠃⠀⠀⠀⠀⠀⢰⠟⠁⠀⢹⢇⠀⣀⠴⠊⡱⠥⠔⠋⠀⠀⢰⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\r\n⠀⠀⠀⠀⠀⠀⢸⡟⠀⢀⡴⠁⠀⠀⠀⠀⠀⢠⡟⠀⠀⣰⢿⡘⣾⡅⠀⠀⠀⠀⢀⠄⠀⢠⠏⢀⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\r\n⠀⠀⠀⠀⠀⠀⢸⠀⣰⣿⠀⠀⠀⠀⠀⠀⢠⣿⠃⢀⡾⡇⠘⠻⡿⢷⡀⠀⠀⠒⠁⠀⢠⠏⢀⠏⣸⠃⢻⠏⠀⠀⠀⠀⠀⠀⠀⠀\r\n⠀⠀⠀⠀⠀⠀⣧⣾⣹⣿⠀⠀⠀⠀⠀⢠⠏⢉⠀⡞⣰⡇⠀⣴⣥⠞⢷⠀⠀⠀⠀⣠⠎⠀⠸⣶⠋⣠⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀";
char buf[0x2000];

void print_irq_count(void){
    printf("irq_count: ");
    for(uint32_t i = 0 ; i < END_OF_LIST ; i++){
        printf("%l   ", irq_count[i]); 
    }
    printf("\r\n");
}

void setTimeout_callback(void* data){
    uint64_t t = get_jiffies();
    printf("Elapsed time after booting: %l.%l\r\n", t / HZ, t % HZ);
    print_irq_count();
    printf("Your message: %s\r\n",(char*) data);
}


void simple_shell(){
    unsigned int i;
    char ch, *token;
    printf(miku_ascii);
    printf("\r\n\r\n");
    while(1){
        i = 0;
        buf[0] = '\0';
        printf("# ");
        // read command
        while(1){
            ch = (char)getchar();
            if(ch == '\r'){
                printf("\r\n");
                buf[i] = '\0';
                break;
            }else if(ch != '\n'){
                putchar(ch);
                buf[i] = ch;        
                i++;
            }
        }

        token = strtok(buf, DELIM); 
        if(token == NULL) continue;
        // match comman
        if(strcmp(token, "help") == 0){
            printf("help        : print this help menu\r\n" \
                   "hello       : print Hello World!\r\n" \
                   "info        : print hardware infomation\r\n" \
                   "ls          : list files\r\n" \
                   "cat         : cat files\r\n" \
                   "laod        : load user program\r\n" \
                   "time        : print time after booting\r\n" \
                   "reboot      : reboot the device\r\n" \
                   "setTimeout  : set a N seconds timer task\r\n" \
                   "              setTimeout <str> <sec>\r\n" \
                   "irq_count   : list irq count\r\n"
				   "alloc_pages : get pages\r\n" \
                   "              alloc_pages <order>\r\n" \
				   "free_pages  : free pages\r\n" \
                   "              free_pages <addr>\r\n" \
				   "kmalloc     : get an object\r\n" \
                   "              kmalloc <size>\r\n" \
				   "kfree       : free an object\r\n" \
                   "              kfree <addr>\r\n" \

				   "page_info   : get buddy system's statistics\r\n" \

            );

        }else if(strcmp(token, "hello") == 0){
            printf("Hello World!\r\n");
        }else if(strcmp(token, "info") == 0){
            uint32_t tmp[2];
            MBox_get_board_revision(tmp);
            printf("Board revision: 0x%x\r\n", tmp[0]);

            MBox_get_arm_memory(tmp);
            printf("Memory base:    0x%x\r\n", tmp[0]);
            printf("Memory size:    0x%x\r\n", tmp[1]);
        }else if(strcmp(token, "reboot") == 0){
            printf("Start rebooting...\r\n");
            reboot(100); 
            while(1);
        }else if(strcmp(token, "ls") == 0){
            initrdfs_ls();
        }else if(strcmp(token, "cat") == 0){
            initrdfs_cat();
        }else if(strcmp(token, "load") == 0){
            initrdfs_loadfile("test.img",(uint8_t*) 0x100000);
            asm volatile("mov x0, 0x0\n\t" 
                         "msr spsr_el1, x0\n\t"
                         "mov x0, #0x100000\n\t"
                         "msr elr_el1, x0\n\t"
                         "eret\n\t"
            );
        }else if(strcmp(token, "time") == 0){
            uint64_t j = get_jiffies();
            printf("Elapsed time after booting: %l.%l\r\n", j / HZ, j % HZ);
        }else if(strcmp(token, "setTimeout") == 0){
            uint64_t j = 0, len = 0, timeout = 0;
            char *data = NULL;
            
            token = strtok(NULL, DELIM);
            if(token == NULL) continue;
            len = strlen(token);
            data = (char*)simple_malloc(len + 1);
            strcpy(data, token);

            token = strtok(NULL, DELIM);
            if(token == NULL) continue;
            timeout = atoul(token);

            j = get_jiffies();
            add_timer(setTimeout_callback, data, timeout * 1000);
            printf("Elapsed time after booting: %l.%l\r\n", j / HZ, j % HZ);
            print_irq_count();
            printf("Timer will trigger after %l seconds\r\n", timeout);
        }else if(strcmp(token, "irq_count") == 0){
            print_irq_count();    
		}else if(strcmp(token, "alloc_pages") == 0){
			uint32_t order;
			void* p;

			token = strtok(NULL, DELIM);
            if(token == NULL) continue;
            order = atoul(token);
			p = alloc_pages(order);
			printf("Allocate pages at %l\r\n", p);
        }else if(strcmp(token, "free_pages") == 0){
			void* p;
			struct page* page;

			token = strtok(NULL, DELIM);
            if(token == NULL) continue;
            p = (void*)atoul(token);
			page = pfn_to_page(addr_to_pfn(p));
			
			free_pages(p, get_page_order(page));
			printf("Free pages\r\n");
        }else if(strcmp(token, "kmalloc") == 0){
			uint32_t size;
			void* p;

			token = strtok(NULL, DELIM);
            if(token == NULL) continue;
            size = atoul(token);
			p = kmalloc(size);
			printf("Allocate an object at %l\r\n", p);
        }else if(strcmp(token, "kfree") == 0){
			void* p;

			token = strtok(NULL, DELIM);
            if(token == NULL) continue;
            p = (void*)atoul(token);
			
			kfree(p);
			printf("Free an object\r\n");
        }else if(strcmp(token, "page_info") == 0){
			print_buddy_statistics();
        }
    }
}
