#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>



typedef struct {
	int length;
	char *head;
} String;


String *make_string(char *contents)
{
	int length = strlen(contents) + 1;

	void *uncast = malloc(sizeof(String) + (sizeof(char)*length));
	String *string = (String*)uncast;
	string->length = length;
	string->head = ((char*)uncast)+sizeof(String);
	strcpy(string->head, contents);

	return string;
}


int main()
{
	String *string = make_string("Hello world!");
	printf("%s\n", string->head);
	free(string);

	return EXIT_SUCCESS;
}
