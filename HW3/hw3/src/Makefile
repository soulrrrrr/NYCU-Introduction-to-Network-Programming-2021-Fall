OUT = ../test/hw3
FILE_NAME = hw3
CFLAGS = -Wall
CC = gcc

all: ${FILE_NAME}.*
	@if [ "$(suffix $<)" = ".cpp" ]; then\
		g++ ${CFLAGS} $< -o ${OUT};\
	elif [ "$(suffix $<)" = ".c" ]; then\
		gcc ${CFLAGS} $< -o ${OUT};\
	else\
		echo "There is no hw3.cpp and hw3.c file in the src/ folder.";\
    fi

clean:
	rm hw3