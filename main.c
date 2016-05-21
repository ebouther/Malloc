#include <stdio.h>

int main()
{
	char	*str;

	str = malloc(sizeof(10));
	for (int i = 0; i < 9; i++)
		str[i] = 'a' + i;
	str[9] = '\0';
	printf("STR: '%s'\n", str);
	return (0);
}
