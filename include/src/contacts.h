typedef char name_string;
typedef char ip_string;

typedef struct element{
	ip_string ip[17];
	name_string name[64];
	struct element *previous;
	struct element *next;
}element;

typedef struct infoList{
	element *begin;
	element *end;
	int size;
}infoList;

typedef element contact;