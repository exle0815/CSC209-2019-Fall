# The first Makefile rule is the one that is evaluated when make is run 
# with no arguments. 
all : simpletest mytest

# The target in this rule is the executable file name. The executable file 
# depends on the three object files which are compiled separately using the 
# rules below.  The action line in this rule links the three object files to 
# produce the executable file.

simpletest : simpletest.o smalloc.o testhelpers.o
	gcc -Wall -g -o simpletest simpletest.o smalloc.o testhelpers.o

mytest : mytest.o smalloc.o testhelpers.o
	gcc -Wall -g -o mytest mytest.o smalloc.o testhelpers.o

# The following three rules compile a single .c file into an object file.  
# We could collape these into one pattern rule.

simpletest.o : simpletest.c smalloc.h
	gcc -Wall -g -c simpletest.c

mytest.o : mytest.c smalloc.h
	gcc -Wall -g -c mytest.c

smalloc.o : smalloc.c smalloc.h
	gcc -Wall -g -c smalloc.c

testhelpers.o : testhelpers.c smalloc.h
	gcc -Wall -g -c testhelpers.c

# The clean target is a phony target because there is no file called clean
# It is a handy rule to remove the executabel and object files. For example,
# you might run "make clean" before git add and commit.

clean : 
	rm simpletest mytest *.o
