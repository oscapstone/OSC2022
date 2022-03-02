int strcmp(char *s1, char *s2) {
    int value;

    s1--, s2--;
    do {
        s1++, s2++;
        if (*s1 == *s2) {
            value = 0;
        } else if (*s1 < *s2) {
            value = -1;
            break;
        } else {
            value = 1;
            break;
        }
    } while (*s1 != 0 && *s2 != 0);
    return value;
}