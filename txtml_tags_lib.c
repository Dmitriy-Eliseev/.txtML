#include "txtml_tags_lib.h"

const uint8_t DEFAULT_DOC_WIDTH = 80;
uint8_t DOC_WIDTH = DEFAULT_DOC_WIDTH;
/***************************************************************************
* functions for working with errors
***************************************************************************/
void exit_on_error(char* msg, void* ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "%s", msg);
        exit(EXIT_FAILURE);
    }
}

void is_memory_allocated(void* mem_ptr)
{
    exit_on_error("Memory allocation error\n", mem_ptr);
}

void is_directory_opened(void* dir_ptr)
{
    exit_on_error("Error opening directory\n", dir_ptr);
}

void print_file_error(char* filename)
{
    printf("  Error opening file \"%s\"\n", filename);
}

void print_tag_error(char* tag)
{
    char* tag_name = get_tag_name(tag);
    printf("  Error: invalid tag \"%s\". Ignoring\n", tag_name);
    if (tag_name != tag) free(tag_name);
}


/***************************************************************************
* functions for working with files
***************************************************************************/
uint16_t get_files_count(char* dirname, char* file_extension)
{
    DIR *dir = opendir(dirname);
    is_directory_opened(dir);

    struct dirent *file;
    uint16_t i = 0;
    while ((file = readdir(dir)) != NULL) {
        char* extension = strrchr(file->d_name, '.');//find start of file extension
        if (extension != NULL && strcmp(extension, file_extension) == 0) i++;
    }
    closedir(dir);
    return i;
}

char** get_files_in_dir(char* dirname, char* file_extension)
{
    uint16_t files_count = get_files_count(dirname, file_extension);
    struct dirent *file;
    DIR *dir = opendir(dirname);
    is_directory_opened(dir);

    char** file_names = calloc(files_count, sizeof(char*));
    is_memory_allocated(file_names);
    uint16_t i = 0;
    while ((file = readdir(dir)) != NULL) {
        char* extension = strrchr(file->d_name, '.');//find start of file extension
        if (extension != NULL && strcmp(extension, file_extension) == 0) {
            file_names[i] = calloc(256,  sizeof(char));
            is_memory_allocated(file_names[i]);
            strcpy(file_names[i],file->d_name);
            i++;
        }
    }
    closedir(dir);
    return  file_names;
}

char* get_file_content(char* filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {print_file_error(filename); return NULL;}

    //getting file size
    fseek(file, 0, SEEK_END);
    uint32_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    //getting text from file
    char* str = (char*)calloc(file_size + 1,  sizeof(char));
    is_memory_allocated(str);
    size_t result = fread(str, 1, file_size, file);
    str[result] = '\0';

    fclose(file);
    return str;
}

void write_to_file(char* filename, char* str)
{
    FILE *file;
    file = fopen(filename, "w");
    if (file==NULL) print_file_error(filename);
    fprintf(file, "%s", str);
    fclose(file);
}

char* change_file_extension(char* filename, char* extension)
{
    char* result = calloc(strlen(filename) + strlen(extension) + 1, sizeof(char));
    is_memory_allocated(result);
    strcpy(result, filename);
    char* extension_start = strrchr(result, '.');//find start of file extension
    if (extension_start != NULL) strcpy(extension_start, extension);
    return result;
}


/***************************************************************************
* functions for working with TAGS
***************************************************************************/
char tag_list[][20] = { "date", "time", "datetime", "right", "center", "h1", "h2", "h3", "h4",
                        "doc_width", "def_width", "sep", "p", "frame", "list", "lines", "calc", "table",
                        "histogram", "insert"};
const int tag_count = sizeof(tag_list) / sizeof(tag_list[0]);
char* (*tag_functions[])(char*, char**) = { get_date, get_time, get_datetime, right, center, h1,
                                            h2, h3, h4, doc_width, def_width, separator, p, get_framed_text,
                                            get_list, get_lines, calc, get_table, get_histogram, insert };
char single_tags[][20] = { "date", "time", "datetime", "doc_width", "def_width", "sep", "lines", "insert" };
const int single_tags_count = sizeof(single_tags) / sizeof(single_tags[0]);


int8_t have_attributes(char* tag)
{
    uint16_t elements_count= get_elements_count(' ', tag);
    if (elements_count == 1) {
        return 0;
    }
    return elements_count;
}

char** get_tag_attributes(char* tag)
{
    char** attrs = NULL;
    if (have_attributes(tag) != 0) {
        attrs = (char**)calloc(have_attributes(tag),  sizeof(char*));
        is_memory_allocated(attrs);
        char** tag_attr = split(' ', tag);
        free(tag_attr[0]);
        for (uint16_t i = 1; i < have_attributes(tag); i++) {
            attrs[i-1] = calloc(strlen(tag_attr[i]) + 1, sizeof(char));
            is_memory_allocated(attrs[i-1]);
            strcpy(attrs[i-1], tag_attr[i]);
            free(tag_attr[i]);
        }
        free(tag_attr);
        attrs[have_attributes(tag) - 1] = NULL;
    }
    return attrs;
}

char* get_tag_name(char* tag)
{
    int8_t attr = have_attributes(tag);
    if (attr != 0) {
        char** tmp = split(' ', tag);
        char* res = strdup(tmp[0]);
        for (uint16_t i = 0; i < get_elements_count(' ', tag); i++) free(tmp[i]);
        free(tmp);
        return res;
    }
    return tag;
}

int8_t is_valid_tag(char* tag)
{
    char* tag_name = get_tag_name(tag);
    for (uint8_t i = 0; i < tag_count; i++) {
        if (strcmp(tag_list[i], tag_name) == 0) {
            if (tag != tag_name) free(tag_name);
            return i;
        }
    }
    if (tag != tag_name) free(tag_name);
    return -1;
}

int8_t is_single_tag(char* tag)
{
    char* tag_name = get_tag_name(tag);
    for (uint8_t i = 0; i < single_tags_count; i++) {
        if (strcmp(single_tags[i], tag_name) == 0) {
            if (tag != tag_name) free(tag_name);
            return 1;
        }
    }
    if (tag != tag_name) free(tag_name);
    return 0;
}

char* get_open_tag(char* tag)
{
    char* open_tag = (char*)calloc(strlen(tag) + 3,  sizeof(char));
    is_memory_allocated(open_tag);
    sprintf(open_tag, "<%s>", tag);
    return open_tag;
}

char* get_close_tag(char* tag)
{
    char* t_tag = strdup(tag);
    t_tag = rm_spaces_start_end(t_tag);
    if (is_single_tag(t_tag) == 0) {
        char* close_tag = NULL;
        char* tag_name = get_tag_name(t_tag);
        close_tag = (char*) calloc(strlen(tag_name) + 4,  sizeof(char));
        is_memory_allocated(close_tag);
        sprintf(close_tag, "</%s>", tag_name);
        free(tag_name);
        if (t_tag != tag_name) free(t_tag);
        return close_tag;
    } else {
        free(t_tag);
        return get_open_tag(tag);
    }
}

char* get_tag(const char* str)
{
    if (str == NULL) return NULL;
    char* tag = NULL;
    char* start = strchr(str, '<') + 1;
    char* end = strchr(str, '>');
    if (start == NULL || end == NULL) return NULL;
    if (end > start) {
        if (*start != '/') {
            tag = (char*)calloc(end - start + 1,  sizeof(char));
            is_memory_allocated(tag);
            memcpy(tag, start, end - start);
            tag[end - start] = '\0';
        }
    }
    return tag;
}

char* get_tag_content(char* str, char* tag)
{
    char* tag_content = NULL;
    if (is_single_tag(tag) != 0) {
        tag_content = (char *) calloc(3,  sizeof(char));
        is_memory_allocated(tag_content);
        strcpy(tag_content, " ");
        return tag_content;
    }
    char* open_tag = get_open_tag(tag);
    char* close_tag = get_close_tag(tag);
    char *start = strstr(str, open_tag) +
                  sizeof(char) * strlen(open_tag);
    char *end = strstr(str, close_tag);
    free(open_tag);
    free(close_tag);
    if (end == NULL || end - start == 0) {
        tag_content = (char *)  calloc(3,  sizeof(char));
        is_memory_allocated(tag_content);
        if (end != NULL && end - start == 0) {
            strcpy(tag_content, "\v");
        } 
        if (end == NULL) {
            strcpy(tag_content, "\r");
        }
        return tag_content;
    }
    tag_content = (char *) calloc(end - start + 1, sizeof(char));
    is_memory_allocated(tag_content);
    memcpy(tag_content, start, end - start);
    tag_content[end - start] = '\0';

    if (tag_content[0] == '\n') {
        char *tmp = calloc(strlen(tag_content), sizeof(char));
        is_memory_allocated(tmp);
        strcpy(tmp, &tag_content[1]);
        free(tag_content);
        tag_content = tmp;
    }
    if (tag_content[strlen(tag_content) - 1] == '\n') tag_content[strlen(tag_content) - 1] = '\0';

    return tag_content;
}

char* get_text_before_tag(char* str, char* tag)
{
    char* open_tag = get_open_tag(tag);
    char* start_tag = strstr(str, open_tag);
    char* text_before_tag = (char*)calloc(start_tag - &str[0] + 1, sizeof(char));
    is_memory_allocated(text_before_tag);
    memcpy(text_before_tag, &str[0], start_tag - &str[0]);
    text_before_tag[start_tag - &str[0]] = '\0';
    free(open_tag);
    return text_before_tag;
}

char* get_text_after_tag(char* str, char* tag)
{
    char* close_tag = get_close_tag(tag);
    char* end_tag = strstr(str, close_tag);
    if (end_tag == NULL) {
        if (is_valid_tag(tag) != -1) {
            printf("  Error: no closing tag found for \"%s\". Ignoring\n", tag);
        } else print_tag_error(tag);
        free(close_tag);
        close_tag = get_open_tag(tag);
        end_tag = strstr(str, close_tag);
    } else if (is_valid_tag(tag) == -1) print_tag_error(tag);
    end_tag = end_tag + sizeof(char) * strlen(close_tag);
    free(close_tag);
    char* text_after_tag = (char*)calloc(strlen(str) - (end_tag - &str[0]) + 1, sizeof(char));
    is_memory_allocated(text_after_tag);
    strcpy(text_after_tag, end_tag);
    return text_after_tag;
}



char* execute_tag(char* tag, char* tag_content)
{
    char* t_tag = strdup(tag);
    t_tag = rm_spaces_start_end(t_tag);
    int8_t tag_i = is_valid_tag(t_tag);
    
    if (tag_i != -1 && strcmp(tag_content, "\r") != 0) {
        char** attr = get_tag_attributes(t_tag);
        char* tag_result = (*tag_functions[tag_i])(tag_content, attr);
        for (uint16_t i = 0; i < have_attributes(tag) - 1; i++) {
            if (attr[i] != NULL) free(attr[i]);
            else break;
        }
        free(t_tag);
        free(attr);
        return tag_result;
    }
    free(t_tag);
    return tag_content;
}

char* execute_nested_tags(char* str)
{
    char* tag = get_tag(str);
    if (tag != NULL) {
        char* t_tag = strdup(tag);
        t_tag = rm_spaces_start_end(t_tag);

        char* tag_content = get_tag_content(str, tag);
        char* res = execute_nested_tags(tag_content);

        char* text_before_tag = get_text_before_tag(str, tag);
        char* text_after_tag = get_text_after_tag(str, t_tag);

        char* result = NULL;
        char* tag_result = execute_tag(tag, res);
        uint32_t len = strlen(text_before_tag) + strlen(tag_result) + strlen(text_after_tag) + 1;
        result = (char*)calloc(len,  sizeof(char));
        is_memory_allocated(result);
        sprintf(result, "%s%s%s", text_before_tag, tag_result, text_after_tag);
        free(text_before_tag);
        free(tag_result);
        free(text_after_tag);
        free(t_tag);
        if (tag_content != tag_result) free(tag_content);
        if (res != tag_content) free(res);
        free(tag);
        return result;
    }
    free(tag);
    return str;
}

char* execute_all_tags(char* str)
{
    char* result = strdup(str);
    is_memory_allocated(result);
    char* tmp = NULL;
    char* tag = get_tag(result);
    while (tag != NULL) {
        tmp = strdup(result);
        free(result);
        result = execute_nested_tags(tmp);
        free(tmp);
        free(tag);
        tag = get_tag(result);
    }
    return result;
}



/***************************************************************************
* functions for working with strings
***************************************************************************/
uint16_t get_elements_count(char sym, char* str)
{
    uint16_t count = 0;
    char* delims = calloc(3,  sizeof(char));
    is_memory_allocated(delims);
    sprintf(delims, "%c", sym);
    char* temp_str = strdup(str);
    char* token = strtok(temp_str, delims);
    while (token != NULL) {
        count++;
        token = strtok(NULL, delims);
    }
    free(delims);
    free(temp_str);
    return count;
}

char** split(char sym, char* str)
{
    char* delims = calloc(3, sizeof(char));
    is_memory_allocated(delims);
    sprintf(delims, "%c", sym);
    uint16_t count = get_elements_count(sym, str);
    char** elements = (char**)calloc(count, sizeof(char*));
    is_memory_allocated(elements);
    char* temp_str = strdup(str);
    count = 0;
    char* token = strtok(temp_str, delims);
    while (token != NULL) {
        elements[count] = strdup(token);
        is_memory_allocated(elements[count]);
        count++;
        token = strtok(NULL, delims);
    }
    free(delims);
    free(temp_str);
    return elements;
}

char* get_str_from_sym(char sym, uint16_t count)
{
    char* str = calloc(count + 1, sizeof(char));
    is_memory_allocated(str);
    char* symbol = calloc(2, sizeof(char));
    is_memory_allocated(symbol);
    sprintf(symbol, "%c", sym);
    for (uint16_t i = 0; i < count; i++) {
        strcat(str, symbol);
    }
    free(symbol);
    return str;
}

void change_symbols(char from, char to, char* str)
{
    for (uint64_t i = 0; i < strlen(str); i++) {
        if (str[i] == from) str[i] = to;
    }
}

uint8_t is_num(char* str)
{
    uint8_t result = 0;
    uint16_t len = strlen(str);
    for (uint16_t i = 0; i < len; i++) {
        if (isdigit(str[i]) || str[i] == '-' || str[i] == '.' || str[i] == ' ') {
            result = 1;
        } else return 0;
    }
    return result;
}

uint8_t is_number(char* str)
{
    uint8_t result = 0;
    uint16_t len = strlen(str);
    for (uint16_t i = 0; i < len; i++) {
        if (isdigit(str[i]) || str[i] == '-' || str[i] == '.' || str[i] == ',' || str[i] == ' ') {
            result = 1;
        } else return 0;
    }
    return result;
}

uint16_t get_number_len(uint16_t number)
{
    char* num_str = calloc(20, sizeof(char));
    is_memory_allocated(num_str);
    sprintf(num_str, "%d", number);
    uint16_t len = strlen(num_str);
    free(num_str);
    return len;
}

char* rm_spaces_from_str(char* str)
{
    char sym = ' ';
    char* result = calloc(strlen(str) + 2, sizeof(char));
    is_memory_allocated(result);
    uint64_t res_i = 0;
    for (uint64_t i = 0; i < strlen(str); i++) {
        if (str[i] != sym) {
            result[res_i] = str[i];
            res_i++;
        }
    }
    result[res_i + 1] = '\0';
    free(str);
    return result;
}

char* rm_spaces_start_end(char* str)
{
    uint64_t start = 0;
    uint64_t end = 0;
    for (uint64_t i = 0; i < strlen(str); i++) {
        if (str[i] != ' ') {
            start = i;
            break;
        }
    }
    for (uint64_t i = strlen(str) - 1; i >= 0; i--) {
        if (str[i] != ' ') {
            end = i;
            break;
        }
    }
    end += 1;
    char* result = calloc(end - start + 2, sizeof(char));
    is_memory_allocated(result);
    memcpy(result, &str[start], end-start);
    result[end-start] = '\0';
    free(str);
    return result;
}


/***************************************************************************
* functions for Text Formatting
***************************************************************************/
void set_doc_width(uint8_t width)
{
    if (width < 10) {
        printf("  Error: Document width cannot be less than 10 characters\n");
        DOC_WIDTH = 10;
    } else if (width > 200) {
        printf("  Error: Document width cannot be more than 250 characters\n");
        DOC_WIDTH = 250;
    } else {
        DOC_WIDTH = width;
    }
}


/***************************************************************************
* functions for working with arrays
***************************************************************************/
uint16_t get_arr_size(char** str_arr)
{
    uint16_t count = 0;
    while (str_arr[count] != NULL) count++;
    return count;
}

uint8_t in_str_array(char** arr, char* value)
{
    if (arr == NULL) return 0;
    uint16_t arr_size = get_arr_size(arr);
    for (uint16_t i = 0; i < arr_size; i++) {
        if (strcmp(arr[i], value) == 0) return 1;
    }
    return 0;
}

uint32_t get_max_len(char** str_arr, uint32_t arr_size)
{
    uint32_t max_len = strlen(str_arr[0]);
    for (uint32_t i = 0; i < arr_size; i++) {
        if (max_len < strlen(str_arr[i])) max_len = strlen(str_arr[i]);
    }
    return max_len;
}

uint16_t get_max(const uint16_t* arr, uint16_t size)
{
    uint16_t max = arr[0];
    for (uint16_t i = 1; i < size; i++) {
        if (max < arr[i]) max = arr[i];
    }
    return max;
}

uint16_t get_min(const uint16_t* arr, uint16_t size)
{
    uint16_t min = arr[0];
    for (uint16_t i = 1; i < size; i++) {
        if (min > arr[i]) min = arr[i];
    }
    return min;
}

int32_t get_index(const uint16_t* arr, uint16_t size, uint16_t value)
{
    int32_t index = -1;
    for (uint16_t i = 0; i < size; i++) {
        if (value == arr[i]) index = i;
    }
    return index;
}

uint16_t get_min_index(const uint16_t* arr, uint16_t size)
{
    uint16_t min = get_min(arr, size);
    uint16_t index = get_index(arr, size, min);
    return index;
}


/***************************************************************************
* Basic functions for some tags
***************************************************************************/
char* get_aligned_text(char* str, char** attrs)
{
    char** lines = split('\n', str);
    uint32_t lines_count = get_elements_count('\n', str);
    uint32_t max_line = get_max_len(lines, lines_count);
    uint32_t len = (max_line > DOC_WIDTH) ? max_line : DOC_WIDTH;
    len = (len + 1) * lines_count;
    char* aligned = calloc(len, sizeof(char));
    is_memory_allocated(aligned);
    for (uint32_t i = 0; i < lines_count; i++) {
        uint16_t spaces = (strlen(lines[i]) > DOC_WIDTH) ? 0 : DOC_WIDTH - strlen(lines[i]);
        if (attrs == NULL) spaces /= 2;
        char* al = get_str_from_sym(' ', spaces);
        strcat(aligned, al);
        free(al);
        strcat(aligned, lines[i]);
        spaces = DOC_WIDTH - strlen(lines[i]) - spaces;
        al = get_str_from_sym(' ', spaces);
        strcat(aligned, al);
        free(lines[i]);
        free(al);
        if (i < lines_count - 1) strcat(aligned, "\n");
    }
    free(lines);
    return aligned;
}

char* header(char* str, uint8_t header_type, char** attrs)
{
    char sep_sym = '\0';
    if (attrs != NULL) {
        if (attrs[0] != NULL) {
            sep_sym = attrs[0][0];
        }
    }
    if (header_type == 1 && sep_sym == '\0') sep_sym = '=';
    uint8_t doc_width_bak = DOC_WIDTH;
    if (DOC_WIDTH < strlen(str)) set_doc_width(strlen(str) + 4);
    char* header = NULL;
    uint16_t len = DOC_WIDTH + 1;
    if (header_type == 1) {
        char* centered_str = center(str, NULL);
        len = DOC_WIDTH * 2 + strlen(centered_str) + 3;
        header = calloc(len, sizeof(char));
        is_memory_allocated(header);
        char* tmp = get_str_from_sym(sep_sym, DOC_WIDTH);
        sprintf(header, "%s\n%s\n%s", tmp, centered_str, tmp);
        free(tmp);
        free(centered_str);
    } else {
        header = calloc(len, sizeof(char));
        is_memory_allocated(header);
        if (sep_sym == '\0') sep_sym = (header_type == 2) ? '=' : '-';
        char* tmp = get_str_from_sym(sep_sym, (DOC_WIDTH-strlen(str) - 1) / 2);
        sprintf(header, "%s %s ", tmp, str);
        free(tmp);
        tmp = get_str_from_sym(sep_sym, DOC_WIDTH - strlen(header));
        strcat(header, tmp);
        free(tmp);
    }
    set_doc_width(doc_width_bak);
    return header;
}

/***************************************************************************
* functions for working with tables
***************************************************************************/
uint16_t get_rows_count(char* tbl_str)
{
    return get_elements_count('\n', tbl_str);
}

uint16_t* get_cells_count(char* tbl_str)
{
    uint16_t  rows_count   = get_rows_count(tbl_str);
    uint16_t* cells_in_row = calloc(rows_count, sizeof(uint16_t));
    is_memory_allocated(cells_in_row);
    char** rows = split('\n', tbl_str);
    for (uint16_t i = 0; i < rows_count; i++) {
        cells_in_row[i] = get_elements_count('|', rows[i]);
        free(rows[i]);
    }
    free(rows);
    return cells_in_row;
}

char*** get_table_data(char* tbl_str)
{
    uint16_t  rows_count   = get_rows_count(tbl_str);
    char**    rows         = split('\n', tbl_str);
    char***   table_data   = (char***)calloc(rows_count, sizeof(char**));
    is_memory_allocated(table_data);
    for (uint16_t i = 0; i < rows_count; i++) {
        table_data[i] = split('|', rows[i]);
        free(rows[i]);
    }
    free(rows);
    return table_data;
}

uint16_t* get_cells_len(char** row, uint16_t cells_count)
{
    uint16_t* cells_len = calloc(cells_count, sizeof(uint16_t));
    is_memory_allocated(cells_len);
    for (uint16_t i = 0; i < cells_count; i++) {
        cells_len[i] = strlen(row[i]);
    }
    return cells_len;
}

uint16_t* get_rows_len(char*** table_data, uint16_t rows_count, const uint16_t* cells_in_row)
{
    uint16_t* rows_len = calloc(rows_count, sizeof(uint16_t));
    is_memory_allocated(rows_len);
    for (uint16_t i = 0; i < rows_count; i++) {
        uint16_t row_len = 0;
        for (uint16_t j = 0; j < cells_in_row[i]; j++) {
            row_len += strlen(table_data[i][j]);
        }
        rows_len[i] = row_len + cells_in_row[i] - 1;
    }
    return rows_len;
}

char* get_table_border(const char* row1, const char* row2)
{
    uint16_t row_len = strlen(row1);
    char* border = calloc(row_len + 1, sizeof(char));
    is_memory_allocated(border);
    for (uint16_t i = 0; i < row_len; i++) {
        if (row1[i] == '|' || row2[i] == '|') {
            strcat(border, "+");
        } else {
            strcat(border, "-");
        }
    }
    return border;
}

char* add_table_border(char* table_str)
{
    char**   rows = split('\n', table_str);
    uint16_t rows_count = get_rows_count(table_str);
    uint16_t row_len = strlen(rows[0]);
    uint32_t len = (rows_count * 2 + 1) * (row_len + 1);
    char*    table = calloc(len, sizeof(char));
    is_memory_allocated(table);
    char* space_str = get_str_from_sym(' ', row_len);
    for (uint16_t i = 0; i < rows_count; i++) {
        char* row1;
        char* row2;
        if (i == 0) {
            row1 = space_str;
            row2 = rows[i];
        } else if (i == rows_count - 1) {
            row1 = rows[i];
            row2 = space_str;
        } else {
            row1 = rows[i - 1];
            row2 = rows[i];
        }
        char* border = get_table_border(row1, row2);
        strcat(table, border);
        strcat(table, "\n");
        strcat(table, rows[i]);
        if (i != rows_count - 1) {
            free(border);
            strcat(table, "\n");
        } else {
            strcat(table, "\n");
            strcat(table, border);
            free(border);
        }
    }
    //Cleaning
    for (uint16_t i = 0; i < rows_count; i++) {
        free(rows[i]);
    }
    free(rows);
    free(space_str);

    return table;
}

void calc_in_table(char*** table_data, uint16_t rows_count, const uint16_t* cells_in_row)
{
    char* calc_res = NULL;
    for (uint16_t i = 0; i < rows_count; i++) {
        for (uint16_t j = 0; j < cells_in_row[i]; j++) {
            char* tmp = calloc(strlen(table_data[i][j]) + 1, sizeof(char));
            strcpy(tmp, table_data[i][j]);
            change_symbols(',', '.', tmp);

            calc_res = calc(tmp, NULL);
            if (strcmp(calc_res, "error") != 0) {
                free(table_data[i][j]);
                table_data[i][j] = strdup(calc_res);
            }
            free(calc_res);
            free(tmp);
        }
    }
}

uint16_t** get_column_width(char*** table_data, uint16_t rows_count, uint16_t* cells_in_row)
{
    uint16_t max_cells = get_max(cells_in_row, rows_count);
    //Memory allocation
    uint16_t** column_width = calloc(max_cells, sizeof(uint16_t*));
    is_memory_allocated(column_width);
    for (uint16_t i = 0; i < max_cells; i++) {
        column_width[i] = calloc(i + 1, sizeof(uint16_t));
        is_memory_allocated(column_width[i]);
    }

    for (uint16_t i = 0; i < rows_count; i++) {
        for (uint16_t j = 0; j < cells_in_row[i]; j++) {
            if (strlen(table_data[i][j]) > column_width[cells_in_row[i] - 1][j]) {
                column_width[cells_in_row[i] - 1][j] = strlen(table_data[i][j]);
            }
        }
    }
    return column_width;
}

void align_to_columns(char*** table_data, uint16_t rows_count, uint16_t* cells_in_row, uint8_t na)
{
    uint16_t** column_width = get_column_width(table_data, rows_count, cells_in_row);
    char* align = NULL;
    char* tmp = NULL;
    for (uint16_t i = 0; i < rows_count; i++) {
        for (uint16_t j = 0; j < cells_in_row[i]; j++) {
            uint16_t al_len = column_width[cells_in_row[i] - 1][j] - strlen(table_data[i][j]);
            if (al_len > 0) {
                align = get_str_from_sym(' ', al_len);
                tmp = strdup(table_data[i][j]);
                free(table_data[i][j]);
                table_data[i][j] = calloc(al_len + strlen(tmp) + 1, sizeof(char));
                if (is_number(tmp) && na == 0) {
                    sprintf(table_data[i][j], "%s%s", align, tmp);
                } else {
                    sprintf(table_data[i][j], "%s%s", tmp, align);
                }
                free(tmp);
                free(align);
            }
        }
    }
    //Cleaning
    uint16_t max_cells = get_max(cells_in_row, rows_count);
    for (uint16_t i = 0; i < max_cells; i++) free(column_width[i]);
    free(column_width);
}

uint16_t get_max_row_len(char*** table_data, uint16_t rows_count, uint16_t* cells_in_row)
{
    uint16_t max_row_len = 0;
    uint16_t max_cells = get_max(cells_in_row, rows_count);
    uint16_t** column_width = get_column_width(table_data, rows_count, cells_in_row);
    for (uint16_t i = 0; i < max_cells; i++) {
        uint16_t row_len = 0;
        for (uint16_t j = 0; j <= i; j++) {
            row_len += column_width[i][j];
        }
        row_len += i;
        if (row_len > max_row_len) max_row_len = row_len;
    }
    //Cleaning
    for (uint16_t i = 0; i < max_cells; i++) free(column_width[i]);
    free(column_width);
    return max_row_len;
}


/***************************************************************************
* functions for working with Histograms
***************************************************************************/
double get_max_value(char** values, uint16_t values_count)
{
    double max_value = 0;
    for (uint16_t i = 0; i < values_count; i++) {
        if (is_num(values[i])) {
            double vl = strtod(values[i], NULL);
            vl = (vl < 0) ? vl * (double)-1 : vl;//abs
            max_value = (max_value < vl) ? vl : max_value;
        }
    }
    return max_value;
}

void get_histogram_data(char* str, char** names, char** values)
{
    char** lines = split('\n', str);
    uint16_t lines_count = get_elements_count('\n', str);
    for (uint16_t i = 0; i < lines_count; i++) {
        char* name = NULL;
        char* value = NULL;
        uint16_t t_count = get_elements_count('|', lines[i]);
        if (t_count >= 2) {
            
            char** t = split('|', lines[i]);
            name = strdup(t[0]);
            is_memory_allocated(name);

            value = strdup(t[1]);
            is_memory_allocated(value);

            if (strcmp(value, " ") != 0) value = rm_spaces_from_str(value);
            change_symbols(',', '.', value);
            if (is_num(value) == 0) {
                free(value);
                value = calloc(6, sizeof(char));
                is_memory_allocated(value);
                strcpy(value, "error");
            }
            
            for (uint16_t j = 0; j < t_count; j++) free(t[j]);
            free(t);
        } else if (t_count == 1) {
            name = calloc(2, sizeof(char));
            is_memory_allocated(name);
            strcpy(name, " ");

            change_symbols(',', '.', lines[i]);

            if (is_num(lines[i])) {
                value = calloc(strlen(lines[i]) + 1, sizeof(char));
                is_memory_allocated(value);
                strcpy(value, lines[i]);
            } else {
                value = calloc(6, sizeof(char));
                is_memory_allocated(value);
                strcpy(value, "error");
            }
        }

        if (strcmp(name, " ") != 0) name = rm_spaces_start_end(name);
        value = rm_spaces_from_str(value);
        names[i] = calloc(strlen(name) + 1, sizeof(char));
        is_memory_allocated(names[i]);
        values[i] = calloc(strlen(value) + 1, sizeof(char));
        is_memory_allocated(values[i]);
        strcpy(names[i], name);
        strcpy(values[i], value);
        free(name);
        free(value);
        free(lines[i]);
    }
    free(lines);
}