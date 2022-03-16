#include <stdio.h>
#include <stdlib.h>
typedef union
{
	char c;
	int a;
	int b;
}Demo;
int main()
{
	Demo d;
	d.c = 'H';
	d.a = 10;
	d.b = 66;
	printf("最開始時變量所占的字節長度為: %d\n", sizeof(d)/4);
	printf("賦值後的三個值分別為：\n");
	printf("%c\t%d\t%d\n", d.c, d.a, d.b);
    char c = 66;
    printf("%c\n",c);

	return 0;
	
}