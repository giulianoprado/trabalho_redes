all: dependencies
	gcc -o dosapp \include/src/dosapp.c -lpthread \include/obj/messages.o \include/obj/contacts.o
dependencies:
	gcc -c \include/src/contacts.c -o \include/obj/contacts.o
	gcc -c \include/src/messages.c -o \include/obj/messages.o
runapp:
	./dosapp 7000

make cleanobj:
	rm \include/obj/*.o