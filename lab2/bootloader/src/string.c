void itoa(int value,char *s) {
    int idx = 0;
    if (value < 0) {
        value *= -1;
        s[idx++] = '-';
    }
    char tmp[10];
    int tidx = 0;
    do {
        tmp[tidx++] = '0' + value % 10;
        value /= 10;
    } while (value != 0 && tidx < 11);
    // reverse tmp
    int i;
    for (i = tidx - 1; i >= 0; i--) {
        s[idx++] = tmp[i];
    }
    s[idx] = '\0';
}

void i16toa(int value, char *s, int digits) {
    int idx = 0;
    char tmp[11];
    int tidx = 0;
    do {
        if(value % 16<10)
          tmp[tidx++] = '0' + value % 16;
        else
          tmp[tidx++] = 'A' + value % 16 - 10;
        value /= 16;
    } while (value != 0 && tidx < 11);
    // reverse tmp
    int i;
    for(int j=0;j<digits-tidx;j++) s[idx++]='0';
    for (i = tidx - 1; i >= 0; i--) {
        s[idx++] = tmp[i];
    }
    s[idx] = '\0';
}

int strcmp(char *string1, char *string2){
    while(*string1 && *string2){
        if(*string1!=*string2) return 0;
        else{
            string1++;
            string2++;
        }
    }
    if(*string1){
        if((*string1>='a' && *string1<='z') || (*string1>='A' && *string1<='Z') || (*string1>='0' && *string1<='9'))
            return 0;
        else
            return 1;
    }
    if(*string2){
        if((*string2>='a' && *string2<='z') || (*string2>='A' || *string2<='Z') || (*string2>=0 && *string2<=9))
            return 0;
        else
            return 1;
    }
    return 1;
}