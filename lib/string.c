int strcmp (const char* str1, const char* str2) {
    for(int i = 0; ; i++) {
        if(str1[i] != str2[i]) {
            return 0;
        }
        else if(str1[i] == '\0' && str2[i] == '\0') {
            return 1;
        }
    }
}

int startwith(const char* str1, const char* str2) {
    for(int i = 0; ; i++) {
        if(str2[i] == '\0') {
            return 1;
        }
        else if(str1[i] != str2[i]) {
            return 0;
        }
    }
}

int strlen(const char* str1) {
    int len = 0;
    while(1) {
        if(str1[len] == '\0') {
            break;
        }
        
        len++;
    }
    return len;
}