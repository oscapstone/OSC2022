unsigned long int atoi(const char *str) {
    unsigned long int ret = 0;
    while (*str) {
        ret = ret * 10 + (*str - '0');
        str++;
    }
    return ret;
}

int log2(int x) {
    int ret = 0;
    while (x >>= 1)
        ret++;
    return ret;
}

int pow2(int x) {
    return (1 << x);
}