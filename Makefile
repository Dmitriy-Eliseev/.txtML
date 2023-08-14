CC = gcc
#CC = clang
#CC = tcc

all:
	$(CC) txtml.c txtml_tags.c txtml_tags_lib.c tinyexpr.c -lm -O3 -o txtml	 
