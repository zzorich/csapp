#include "cachelab.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

static unsigned int h_flag = 0;
static unsigned int v_flag = 0;
static int num_setbits = -1;
static int num_lines = -1;
static int num_blockbits = -1;
static char tracefile_name[101] = "";
static uint32_t num_hit = 0;
static uint32_t num_miss = 0;
static uint32_t num_eviction = 0;
static uint32_t time = 0;

enum access_result {HIT, MISS, MISS_EVC};

typedef struct {
    int time_stamp;
    int vaild;
    uint64_t tag;
} line;

static void parsing_error(int signal, const char *message) {
    printf("%s\n", message);
    exit(signal);
}

static void setflag(int argc, char *argv[]) {
    int i;
    for (i = 1; i < argc - 1; ++i) {
        char flag;
        if (strlen(argv[i]) != 2 || argv[i][0] != '-' || !isalpha(flag = argv[i][1])) {
                parsing_error(-1, "incorrect argument format deteced during parsing!");
        }
        /* assign the flag value */
        const char *file_name;
        switch(flag){
            case 'h':
                h_flag = 1;
                break;
            case 'v':
                v_flag = 1;
                break;
            case 's':
                num_setbits = atoi(argv[++i]);
                break;
            case 'E':
                num_lines = atoi(argv[++i]);
                break;
            case 'b':
                num_blockbits = atoi(argv[++i]);
                break;
            case 't':
                file_name = argv[++i];
                if (strlen(file_name) > 100) {
                    parsing_error(-2, "Unsupported file name length.");
                }
                strcpy(tracefile_name, file_name);
                break;
            default:
                parsing_error(-2, "Undefined or unrecognised flag.");
        }
    }
    if (!v_flag && (num_setbits < 0 || num_lines < 1 || num_blockbits < 0 || strcmp(tracefile_name, "") == 0)) {
        parsing_error(-3, "missing flag or unallowed input.");
    }
}

static uint64_t parsing_memory_address(const char *line) {
    uint64_t result = 0;
    int i = 0;
    char c;
    while(!isxdigit(line[i])) {
        ++i;
    }
    int j;
    for (j = 16; j > 0; --j) {
        c = tolower(line[i++]);
        if (c == '\n' || c == '\0' || c == ',') {
            return result;
        }
        if (isdigit(c)) {
            result <<= 4;
            result += (uint64_t) (c - '0');
        } else if (c <= 'f' && c >= 'a'){
            result <<= 4;
            result += (uint64_t) ((c - 'a') + 10);
        }
    }

    return result;
}

static enum access_result peek(uint64_t E, line cache[][E], uint64_t memory_address) {
    uint64_t set_index = (memory_address >> num_blockbits) & ((((uint64_t) 1) << num_setbits) - 1);
    uint64_t tag = memory_address >> (num_blockbits + num_setbits);
    time++;
    for (int j = 0; j < E; ++j) {
        line ln = cache[set_index][j];
        if (ln.vaild && ln.tag == tag) {
            num_hit++;
            cache[set_index][j].time_stamp = time;
            return HIT;
        }
    }

    uint64_t min_index = 0;
    uint32_t min_time = time;
    for (int j = 0; j < E; ++j) {
        int temp_time;
        if (!cache[set_index][j].vaild) {
            cache[set_index][j].vaild = 1;
            cache[set_index][j].time_stamp = time;
            cache[set_index][j].tag = tag;
            num_miss++;
            return MISS;
        }

        if ((temp_time = cache[set_index][j].time_stamp) < min_time) {
            min_index = j;
            min_time = temp_time;
        }
    }
    
    cache[set_index][min_index].time_stamp = time;
    cache[set_index][min_index].tag = tag;
    num_miss++;
    num_eviction++;
    return MISS_EVC;
}

int main (int argc, char *argv[])
{
    setflag(argc, argv);

    uint64_t S = ((uint64_t) 1) << num_setbits;
    line cache[S][num_lines];
    for (int i = 0; i < S; ++i) {
        for (int j = 0; j < num_lines; ++j) {
            cache[i][j].vaild = 0;
            cache[i][j].time_stamp = 0;
            cache[i][j].tag = 0;
        }
    }

    FILE *trace = fopen(tracefile_name, "r");
    if (!trace) {
        parsing_error(-5, "Fail to open the file.");
    }
    
    char ln[40];
    while(fgets(ln, 20, trace)) {
        if (!isspace(ln[0])) {
            continue;
        }

        uint64_t memory_address = parsing_memory_address(ln);
        char instuct_t = ln[1];
        enum access_result access = peek(num_lines, cache, memory_address);
        num_hit += (instuct_t == 'M'? 1:0);
        
        if (v_flag) {
            char *tail_message;
            ln[strcspn(ln, "\n")] = '\0';
            switch (access) {
                case HIT:
                    tail_message = " hit";
                    break;
                case MISS:
                    tail_message = " miss";
                    break;
                case MISS_EVC:
                    tail_message = " miss eviction";
                    break;
            }
            strcat(ln, tail_message);
            switch (instuct_t) {
                case 'L':
                case 'S':
                    printf("%s\n", ln);
                    break;
                case 'M':
                    strcat(ln, " hit");
                    printf("%s\n", ln);
                    break;
            }
        }
    }

    printSummary(num_hit, num_miss, num_eviction);
    return 0;
}

