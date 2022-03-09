#include "../include/lib.h"

int Strncmp(char *a, char *b, int len){
	for(int i=0;i<len;i++){
		if(a[i] != b[i])
			return 1; 
	}
	return 0;
}