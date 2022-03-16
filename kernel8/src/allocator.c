extern char __heap_base[];
extern char __heap_top[];
static char *freep = __heap_base;

char *simple_alloc(unsigned int nbytes)
{
	if ((*freep + nbytes) < *__heap_top)
	{
		char *p = freep;
		*freep += nbytes;
		return p;
	}
	else {
		return '\0';
	}
}
