#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

void reverse_line(char *line){
    int len = strlen(line);
    for (int i = 0; i < len / 2; i++)
    {
        char temp = line[i];
        line[i] = line[len - i - 1];
        line[len - i - 1] = temp;
    }
}

int is_text_file(const char *filename){
    const char *ext = strrchr(filename, '.');
    return ext && (strcmp(ext, ".txt") == 0);
}

void process_file(const char *src_path, const char *dest_path){
    FILE *src_file = fopen(src_path, "r");
    if (!src_file)
    {
        perror("Error opening source file");
        return;
    }

    FILE *dest_file = fopen(dest_path, "w");
    if (!dest_file)
    {
        perror("Error opening destination file");
        fclose(src_file);
        return;
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, src_file))
    {
        size_t len = strlen(buffer);
        int has_newline = 0;

        if (len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
            has_newline = 1;
        }

        reverse_line(buffer);

        if (has_newline)
        {
            fprintf(dest_file, "%s\n", buffer);
        }
        else
        {
            fputs(buffer, dest_file);
        }
    }

    fclose(src_file);
    fclose(dest_file);
}


void process_directory(const char *src_dir, const char *dest_dir){
    DIR *dir = opendir(src_dir);
    if (!dir)
    {
        perror("Error opening source directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)))
    {
        struct stat entry_stat;
        char src_path[BUFFER_SIZE];
        snprintf(src_path, BUFFER_SIZE, "%s/%s", src_dir, entry->d_name);

        if (stat(src_path, &entry_stat) == 0 && S_ISREG(entry_stat.st_mode) && is_text_file(entry->d_name))
        {
            char dest_path[BUFFER_SIZE];
            snprintf(dest_path, BUFFER_SIZE, "%s/%s", dest_dir, entry->d_name);

            process_file(src_path, dest_path);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]){
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <source_directory> <destination_directory>\n", argv[0]);
        return 1;
    }

    struct stat st = {0};
    if (stat(argv[2], &st) == -1)
    {
        mkdir(argv[2], 0700);
    }

    process_directory(argv[1], argv[2]);

    return 0;
}
