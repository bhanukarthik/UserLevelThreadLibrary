#include<stdio.h>
#include<stdlib.h>

int main()
{
	char * a = malloc(sizeof(char ));
	if( a ==NULL)
	printf("a is null");
	else
	printf("a is not null");

}
