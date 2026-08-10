#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

@def(foreach_line(content: char*)) {
    char* _p = _content;
    while (*_p) {
        char* _line_start = _p;
        while (*_p && *_p != '\n') {
            _p++;
        }
        
        int _len = _p - _line_start;
        // Optionally strip carriage return
        if (_len > 0 && _line_start[_len - 1] == '\r') {
            _len--;
        }

        char line[1024];
        if (_len >= sizeof(line)) _len = sizeof(line) - 1;
        memcpy(line, _line_start, _len);
        line[_len] = '\0';

        $body

        if (*_p == '\n') _p++;
    }
}

int main() {
    FILE *f = fopen("../01_basics/basics.srcmap", "rb");
    if (!f) {
        printf("Run 01_basics first!\n");
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *str = malloc(fsize + 1);
    fread(str, 1, fsize, f);
    fclose(f);
    str[fsize] = 0;

    printf("Parsing srcmap...\n");

    int line_count = 0;
    
    @foreach_line(str) {
        if (strlen(line) == 0) continue;
        
        // Print the first 5 rows to show what's happening
        if (line_count < 5) {
            printf("Row %d: %s\n", line_count, line);
            
            // A simple demonstration of parsing the CSV:
            char *comma = strchr(line, ',');
            if (comma) {
                *comma = '\0'; // split
                printf("  -> First column: %s\n", line);
            }
        }
        line_count++;
    }

    printf("Total records parsed: %d\n", line_count);
    free(str);
    return 0;
}
