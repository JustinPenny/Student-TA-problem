CC=gcc
CFLAGS=-l.

student_ta_solution: student_ta_solution.o
	$(CC) -o student_ta_solution student_ta_solution.o -lpthread

clean:
	rm student_ta_solution.o student_ta_solution
