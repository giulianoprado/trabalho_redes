all: dependencies
	gcc -o main_menu \include/src/menu.c

dependencies:
	gcc -c \include/src/contacts.c -o \include/obj/contacts.o
rmexec:
	rm main_menu
rmobj:
	rm *.o
