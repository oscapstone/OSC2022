int str_cmp(char *s, char *t)
{
	for ( ; *s == *t; s++, t++) {
		if (*s == '\0')
			return 0;
	}
	return *s - *t;
}

void str_clear(char *s)
{
	while (*s != '\0') {
		*s++ = '\0';
	}
}
