#include "shell.h"

void main(){
    
    shell_init();
    
    while(1) {
        shell_run();
    }
}