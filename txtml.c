#include "txtml_tags.h"


void print_logo()
{
    puts("  __            __             __        ");
    puts(" /\\ \\__        /\\ \\__  /'\\_/`\\/\\ \\       ");
    puts(" \\ \\ ,_\\  __  _\\ \\ ,_\\/\\      \\ \\ \\      ");
    puts("  \\ \\ \\/ /\\ \\/'\\\\ \\ \\/\\ \\ \\__\\ \\ \\ \\  __ ");
    puts(" __\\ \\ \\_\\/>  </ \\ \\ \\_\\ \\ \\_/\\ \\ \\ \\L\\ \\");
    puts("/\\_\\\\ \\__\\/\\_/\\_\\ \\ \\__\\\\ \\_\\\\ \\_\\ \\____/");
    puts("\\/_/ \\/__/\\//\\/_/  \\/__/ \\/_/ \\/_/\\/___/ ");
}
int main(int argc, char* argv[]) {
    print_logo();
    char source_file_extension[] = ".tml";
    char result_file_extension[] = ".txt";
    char** files = get_files_in_dir(".", source_file_extension);
    uint16_t files_count = get_files_count(".", source_file_extension);
    if (files_count == 0) {
        printf("Error: .tml files not found\n");
        exit(EXIT_SUCCESS);
    }
    uint16_t i;
    //char source[] = "<h1>Время: <datetime></h1>\n<right><calc s>12^5</calc></right>\n<doc_width 40><sep 1>\n";
    //write_to_file("test.tml", source);

    printf(".txtML translation system v1.0\nCopyright (C) 2023 Dmitriy Eliseev\n\n");
    for (i = 0; i < files_count; i++) {
        printf("processing file: %s\n", files[i]);
        char* file_content = get_file_content(files[i]);
        char* result = execute_all_tags(file_content);
        char* result_file = change_file_extension(files[i], result_file_extension);
        write_to_file(result_file, result);
        printf("  done\n");
        free(file_content);
    }

    return 0;
}