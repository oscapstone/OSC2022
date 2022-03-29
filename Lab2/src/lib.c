#include "../include/lib.h"


int Strncmp(char *a, char *b, int len){
	for(int i=0;i<len;i++){
		if(a[i] != b[i])
			return 1; 
	}
	return 0;
}

unsigned int hex_to_dec(char size[], int n){
	int ans = 0;
	int i=0;

	while(i<n){
		ans *= 16;

		char c = size[i];

		if(c>='0'&&c<='9')
			ans += c-'0';
		else if (c>='a'&&c<='f')
            ans += c-'a'+10;
        else if (c>='A'&&c<='F')
            ans += c-'A'+10;

        i++;
	}

	return ans;
}

int Strlen(char *str){
	int ans = 0;
	while(str[ans] != '\0')
		ans++;
	return ans;
}