int hex_to_int(char *p, int len) {
    int val = 0;
    int temp;
    for (int i = 0; i < len; i++) {
        temp = *(p + i);
        if (temp >= 'A') {
            temp = temp - 'A' + 10;
        } else
            temp -= '0';
        val *= 16;
        val += temp;
    }
    return val;
}