
#ifndef TXTML_TAGS_H
#define TXTML_TAGS_H

#include "txtml_tags_lib.h"

//date and time
char* get_date(char* str, char** attrs);
char* get_time(char* str, char** attrs);
char* get_datetime(char* str, char** attrs);

//alignment
char* right(char* str, char** attrs);
char* center(char* str, char** attrs);

//headers
char* h1(char* str, char** attrs);
char* h2(char* str, char** attrs);
char* h3(char* str, char** attrs);
char* h4(char* str, char** attrs);

//text formatting
char* doc_width(char* str, char** attrs);
char* def_width(char* str, char** attrs);
char* separator(char* str, char** attrs);
char* p(char* str, char** attrs);
char* get_framed_text(char* str, char** attrs);
char* get_list(char* str, char** attrs);
char* get_lines(char* str, char** attrs);

//calculations and visualization
char* calc(char* str, char** attrs);
char* get_table(char* str, char** attrs);
char* get_histogram(char* str, char** attrs);

//files
char* insert(char* str, char** attrs);


#endif /*TXTML_TAGS_H*/