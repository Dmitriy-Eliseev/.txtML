#include "txtml_tags.h"
#include "tinyexpr.h"
/***************************************************************************
* Date and Time
***************************************************************************/
char* get_date(char* str, char** attrs)
{
    char* date = calloc(11, sizeof(char));
    is_memory_allocated(date);
    time_t current_time = time(NULL);
    struct tm *local_time = localtime(&current_time);
    sprintf(date, "%02d.%02d.%d", local_time->tm_mday, local_time->tm_mon + 1, local_time->tm_year + 1900);
    return date;
}

char* get_time(char* str, char** attrs)
{
    char* t = calloc(9, sizeof(char));
    is_memory_allocated(t);
    time_t current_time = time(NULL);
    struct tm *local_time = localtime(&current_time);
    sprintf(t, "%02d:%02d:%02d", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
    return t;
}

char* get_datetime(char* str, char** attrs)
{
    char* date = get_date(NULL, NULL);
    char* t = get_time(NULL, NULL);
    char* datetime = calloc(strlen(date) + strlen(t) + 3, sizeof(char));
    is_memory_allocated(datetime);
    sprintf(datetime, "%s %s", date, t);
    free(t);
    free(date);
    return datetime;
}


/***************************************************************************
* Text Alignment
***************************************************************************/
char* right(char* str, char** attrs)
{
    char** t = calloc(1, sizeof(char*));
    is_memory_allocated(t);
    char* result = get_aligned_text(str, t);
    free(t);
    return result;
}

char* center(char* str, char** attrs)
{
    return get_aligned_text(str, NULL);
}


/***************************************************************************
* Headers
***************************************************************************/
char* h1(char* str, char** attrs)
{
    return header(str, 1, attrs);
}

char* h2(char* str, char** attrs)
{
    return header(str, 2, attrs);
}

char* h3(char* str, char** attrs)
{
    return header(str, 3, attrs);
}

char* h4(char* str, char** attrs)
{
    char* h = calloc(strlen(str) * 2 + 2, sizeof(char));
    is_memory_allocated(h);
    char sep_sym = '-';
    if (attrs != NULL) {
        if (attrs[0] != NULL) {
            sep_sym = attrs[0][0];
        }
    }
    char* s = get_str_from_sym(sep_sym, strlen(str));
    sprintf(h, "%s\n%s", str, s);
    free(s);
    return h;
}


/***************************************************************************
* Text Formatting
***************************************************************************/
char* doc_width(char* str, char** attrs)
{
    uint8_t width = DOC_WIDTH;
    if (attrs != NULL) {
        if (attrs[0] != NULL){
            if (is_num(attrs[0])) width = atoi(attrs[0]);
            set_doc_width(width);
        }
    }
    char* r = calloc(2, sizeof(char));
    strcpy(r, "");
    return r;
}

char* separator(char* str, char** attrs)
{
    char sep_symbol = '-';
    if (attrs != NULL) {
        if (attrs[0] != NULL) sep_symbol = attrs[0][0];
    }
    return get_str_from_sym(sep_symbol, DOC_WIDTH);
}

char* p(char* str, char** attrs)
{
    char* tmp_str = NULL;
    if (attrs != NULL) {
        set_doc_width(DOC_WIDTH - 2);
        tmp_str = right(str, NULL);
        set_doc_width(DOC_WIDTH + 2);
    } else tmp_str = strdup(str);
    char** lines = split('\n', tmp_str);
    uint16_t lines_count = get_elements_count('\n', tmp_str);
    uint32_t len = strlen(tmp_str) + lines_count * 2 + 3;
    char* pr = calloc(len, sizeof(char));
    is_memory_allocated(p);
    for (uint16_t i = 0; i < lines_count; i++) {
        if (attrs == NULL) {
            strcat(pr, "  ");
            strcat(pr, lines[i]);
        } else {
            strcat(pr, lines[i]);
            strcat(pr, "  ");
        }
        strcat(pr, "\n");
        free(lines[i]);
    }
    free(tmp_str);
    free(lines);
    strcat(pr, "\n");
    return pr;
}

char* get_framed_text(char* str, char** attrs)
{
    char** lines = split('\n', str);
    uint16_t lines_count = get_elements_count('\n', str);
    uint16_t max_line = get_max_len(lines, lines_count);
    uint32_t len = (max_line + 11) * (lines_count + 2);
    char* framed_text = calloc(len, sizeof(char));
    is_memory_allocated(framed_text);

    for (uint16_t i = 0; i < lines_count; i++) {
        free(lines[i]);
    }
    free(lines);

    uint8_t doc_width_bak = DOC_WIDTH;
    set_doc_width(max_line + 2);
    char* tmp_str = center(str, NULL);
    lines = split('\n', tmp_str);
    free(tmp_str);
    tmp_str = get_str_from_sym('=', max_line);
    strcat(framed_text, " .+-");
    strcat(framed_text, tmp_str);
    strcat(framed_text, "-+. \n");
    char* al = NULL;
    for (uint16_t i = 0; i < lines_count; i++) {
        strcat(framed_text, " ||");
        strcat(framed_text, lines[i]);
        al = get_str_from_sym(' ', DOC_WIDTH - strlen(lines[i]));
        strcat(framed_text, al);
        free(al);
        strcat(framed_text, "|| \n");
        free(lines[i]);
    }
    set_doc_width(doc_width_bak);
    free(lines);

    strcat(framed_text, " '+-");
    strcat(framed_text, tmp_str);
    free(tmp_str);
    strcat(framed_text, "-+' ");
    return framed_text;
}

char* get_list(char* str, char** attrs)
{
    char** items = split('\n', str);
    uint16_t items_count = get_elements_count('\n', str);
    uint16_t align = 0;
    if (attrs == NULL) {
        align = get_number_len(items_count);
    }
    uint16_t len = strlen(str) + items_count * (align + 4);
    char* lst = calloc(len, sizeof(char));
    is_memory_allocated(lst);

    for (uint16_t i = 0; i < items_count; i++) {
        uint16_t mrk_len = align + 4;
        char* mrk_str = calloc(mrk_len, sizeof(char));
        is_memory_allocated(mrk_str);
        if (attrs == NULL) {
            char* al = get_str_from_sym(' ', align - get_number_len(i+1));
            sprintf(mrk_str, " %d) ", i + 1);
            strcat(mrk_str, al);
            free(al);
        } else {
            sprintf(mrk_str, " %c ", attrs[0][0]);
        }
        strcat(lst, mrk_str);
        free(mrk_str);
        strcat(lst, items[i]);
        if (i != items_count - 1) strcat(lst, "\n");
    }
    //Cleaning
    for (uint16_t i = 0; i < items_count; i++) {
        free(items[i]);
    }
    free(items);

    return lst;
}

char* get_lines(char* str, char** attrs)
{
    uint16_t count = 1;
    if (attrs != NULL) {
        if (attrs[0] != NULL) {
            if(is_num(attrs[0])) {
                uint16_t c = atoi(attrs[0]);
                if (c > 0) count = c;
            }
        }
    }
    char* lines = get_str_from_sym('\n', count);
    return lines;
}


/***************************************************************************
* Calculations and Visualization
***************************************************************************/
char* calc(char* str, char** attrs)
{
    char** expressions = split('\n', str);
    uint16_t expr_count = get_elements_count('\n', str);
    uint16_t res_len = 0;
    char* result_str = NULL;
    char* tmp = NULL;
    for (uint16_t i = 0; i < expr_count; i++) {
        tmp = calloc(1000, sizeof(char));
        is_memory_allocated(tmp);
        int error;
        double result = te_interp(expressions[i], &error);
        if (attrs == NULL) {
            if (error) {
                sprintf(tmp, "error");
            } else {
                sprintf(tmp, "%g", result);
            }
        } else {
            if (error) {
                sprintf(tmp, "%s = error", expressions[i]);
            } else {
                sprintf(tmp, "%s = %g", expressions[i], result);
            }
        }
        uint16_t len = strlen(tmp) + 1;
        res_len += len;
        free(expressions[i]);
        expressions[i] = calloc(len, sizeof(char));
        is_memory_allocated(expressions[i]);
        strcpy(expressions[i], tmp);
        free(tmp);
    }
    result_str = calloc(res_len, sizeof(char));
    is_memory_allocated(result_str);
    for (uint16_t i = 0; i < expr_count; i++) {
        strcat(result_str, expressions[i]);
        if (i < expr_count - 1) strcat(result_str, "\n");
        free(expressions[i]);
    }
    free(expressions);
    return result_str;
}

char* get_table(char* str, char** attrs)
{
    uint16_t  rows_count   = get_rows_count(str);
    uint16_t* cells_in_row = get_cells_count(str);
    char***   table_data   = get_table_data(str);
    uint8_t nb = in_str_array(attrs, "nb");//no border
    uint8_t nc = in_str_array(attrs, "nc");//no calculations
    uint8_t na = in_str_array(attrs, "na");//don't align numbers to the right
    if (nb == 1) na = 1;
    if (nc == 0) calc_in_table(table_data, rows_count, cells_in_row);
    align_to_columns(table_data, rows_count, cells_in_row, na);
    uint16_t* rows_len     = get_rows_len(table_data, rows_count, cells_in_row);
    uint16_t  max_row_len  = get_max_row_len(table_data, rows_count, cells_in_row);
    if (max_row_len < DOC_WIDTH - 2) max_row_len = DOC_WIDTH - 2;
    char*     table        = calloc(rows_count * (max_row_len + 3) + 1, sizeof(char));
    is_memory_allocated(table);
    for (uint16_t i = 0; i < rows_count; i++) {
        if (nb == 0) strcat(table, "|");
        uint16_t* cells_len = get_cells_len(table_data[i], cells_in_row[i]);
        uint16_t  align     = max_row_len - rows_len[i];

        while (align > 0) {
            uint16_t min_cell_i = get_min_index(cells_len, cells_in_row[i]);
            char*    al_str     = calloc(strlen(table_data[i][min_cell_i]) + 2, sizeof(char));
            is_memory_allocated(al_str);
            if (is_number(table_data[i][min_cell_i]) && na == 0) {
                strcat(al_str, " ");
                strcat(al_str, table_data[i][min_cell_i]);
            } else {
                strcpy(al_str, table_data[i][min_cell_i]);
                strcat(al_str, " ");
            }
            free(table_data[i][min_cell_i]);
            table_data[i][min_cell_i] = al_str;
            cells_len[min_cell_i]++;
            align--;
        }
        for (uint16_t j = 0; j < cells_in_row[i]; j++) {
            strcat(table, table_data[i][j]);
            if(nb == 0){
                strcat(table, "|");
            } else {
                strcat(table, " ");
            }
        }
        strcat(table, "\n");
        free(cells_len);
    }
    char* result = (nb == 0) ? add_table_border(table) : table;
    //Cleaning
    if (nb == 0) free(table);
    for (uint16_t i = 0; i < rows_count; i++) {
        for (uint16_t j = 0; j < cells_in_row[i]; j++) {
            free(table_data[i][j]);
        }
        free(table_data[i]);
    }
    free(table_data);
    free(rows_len);
    free(cells_in_row);

    return result;
}

char* get_histogram(char* str, char** attrs)
{
    char sym = '#';

    if (attrs != NULL) {
        if (attrs[0] != NULL) {
            sym = attrs[0][0];
        }
    }
    uint16_t lines_count = get_elements_count('\n', str);
    char** names = calloc(lines_count, sizeof(char*));
    is_memory_allocated(names);
    char** values = calloc(lines_count, sizeof(char*));
    is_memory_allocated(values);
    get_histogram_data(str, names, values);

    uint32_t len = (DOC_WIDTH + 1) * lines_count;
    char* histogram = calloc(len, sizeof(char));
    is_memory_allocated(histogram);

    char* tmp = NULL;
    uint16_t max_name = get_max_len(names, lines_count);
    double max_value = get_max_value(values, lines_count);
    uint16_t max_value_len = get_max_len(values, lines_count);
    uint16_t hist_width = DOC_WIDTH - max_name - max_value_len - 8;
    double hist_sym = max_value / (double)(hist_width);

    for (uint16_t i = 0; i < lines_count; i++) {
        if (strcmp(values[i], " ") != 0){
            char* al = get_str_from_sym(' ', max_name - strlen(names[i]));
            tmp = calloc(DOC_WIDTH + 1, sizeof(char));
            sprintf(tmp, " %s%s | ", al, names[i]);
            free(al);
            double v = 0;
            char* tmp2 = calloc(strlen(values[i]) + 5, sizeof(char));
            sprintf(tmp2, " | %s ", values[i]);
            if (is_num(values[i]) == 1) v = strtod(values[i], NULL);
            uint16_t hist_len = (uint16_t)round(v / (double)hist_sym);
            char* h = get_str_from_sym(sym, hist_len);
            strcat(tmp, h);
            free(h);
            h = get_str_from_sym(' ', hist_width - hist_len);
            strcat(tmp, h);
            strcat(tmp, tmp2);

            free(h);
            free(tmp2);
        } else {
            strcat(histogram, "\n");
        }
        strcat(histogram, tmp);
        free(tmp);
        free(names[i]);
        free(values[i]);
        if (i < lines_count - 1) strcat(histogram, "\n");
    }
    free(names);
    free(values);
    return histogram;
}


/***************************************************************************
* Files
***************************************************************************/
char* insert(char* str, char** attrs)
{
    char* inserting_text = NULL;
    uint64_t ins_len = 0;
    if (attrs != NULL) {
        uint16_t files_count = get_arr_size(attrs);
        char** file_contents = calloc(files_count, sizeof(char*));
        is_memory_allocated(file_contents);
        for (uint16_t i = 0; i < files_count; i++) {
            if (attrs[i] != NULL) {
                file_contents[i] = get_file_content(attrs[i]);
                if (file_contents[i] != NULL) {
                    change_symbols('<', '\f', file_contents[i]);
                    change_symbols('>', '\a', file_contents[i]);
                    ins_len += strlen(file_contents[i]);
                }
            } else file_contents[i] = NULL;
        }
        ins_len += files_count * 3;
        inserting_text = calloc(ins_len, sizeof(char));
        is_memory_allocated(inserting_text);
        strcat(inserting_text, "\n");
        for (uint16_t i = 0; i < files_count; i++) {
            if (file_contents[i] != NULL) {
                strcat(inserting_text, file_contents[i]);
                strcat(inserting_text, "\n");
                free(file_contents[i]);
            }
            strcat(inserting_text, "\n");
        }
        free(file_contents);
    } else {
        printf("  Error inserting txt: file not specified\n");
        inserting_text = calloc(2, sizeof(char));
        strcpy(inserting_text, "\n");
    }
    return inserting_text;
}