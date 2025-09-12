#include "lexer.h"
#include "parser.h"
#include "symtab.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

// Forward declare new functions
int transpile_repl(const char* source);
static void handle_repl();

// --- CLI Functions ---
static char* read_file_to_string(const char* path) {
    FILE* file = fopen(path, "r");
    if (file == NULL) { fprintf(stderr, "Could not open file \"%s\".\n", path); return NULL; }
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) { fprintf(stderr, "Not enough memory to read \"%s\".\n", path); return NULL; }
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}

static void print_usage(const char* prog_name) { fprintf(stderr, "Usage: %s <file.super> | init <project> | build-lib <file> | check <file> | repl | --show-tokens <file> | --help\n", prog_name); }
static void show_tokens(const char* filename) { char *source = read_file_to_string(filename); if (source == NULL) return; init_lexer(source); printf("--- Tokens for %s ---\n", filename); for (;;) { Token token = get_next_token(); printf("[%s] '%.*s'\n", token_type_to_string(token.type), token.length, token.start); if (token.type == TOKEN_EOF) break; } free(source); }

static int transpile_and_run(const char* filename) {
    const char* out_c_file = "super_out.c";
    const char* out_exe_file = "super_out";
    char* source = read_file_to_string(filename);
    if (source == NULL) return 1;
    char link_libs[1024] = "";
    init_lexer(source);
    Token token;
    for(;;) {
        token = get_next_token();
        if (token.type == TOKEN_IMPORT) {
            token = get_next_token();
            if (token.type == TOKEN_STRING) {
                char lib_name[1024];
                strncpy(lib_name, token.start + 1, token.length - 2);
                lib_name[token.length - 2] = '\0';
                char lib_c_file[1024];
                snprintf(lib_c_file, sizeof(lib_c_file), "%s.c ", lib_name);
                strcat(link_libs, lib_c_file);
            }
        }
        if (token.type == TOKEN_EOF || token.type == TOKEN_FN) break;
    }
    FILE* output_file = fopen(out_c_file, "w");
    if (output_file == NULL) { fprintf(stderr, "Could not open output file.\n"); free(source); return 1; }
    int transpilation_result = transpile(source, output_file, MODE_RUN);
    fclose(output_file);
    free_symbol_table();
    free(source);
    if (transpilation_result != 0) { fprintf(stderr, "Transpilation failed.\n"); remove(out_c_file); return 1; }
    char compile_command[2048];
    snprintf(compile_command, sizeof(compile_command), "gcc -o %s %s %s", out_exe_file, out_c_file, link_libs);
    int compile_status = system(compile_command);
    if (compile_status != 0) { fprintf(stderr, "C compilation failed.\n"); remove(out_c_file); return 1; }
    char run_command[1024];
    snprintf(run_command, sizeof(run_command), "./%s", out_exe_file);
    system(run_command);
    remove(out_c_file);
    remove(out_exe_file);
    return 0;
}

static int handle_build_lib(const char* filename) {
    const char* basename_ptr = strrchr(filename, '/');
    basename_ptr = basename_ptr ? basename_ptr + 1 : filename;
    char basename[1024];
    strncpy(basename, basename_ptr, sizeof(basename));
    basename[sizeof(basename) - 1] = '\0';
    char* dot = strrchr(basename, '.');
    if (dot) *dot = '\0';
    char out_h_file[1024];
    snprintf(out_h_file, sizeof(out_h_file), "%s.h", basename);
    char out_c_file[1024];
    snprintf(out_c_file, sizeof(out_c_file), "%s.c", basename);
    char* source = read_file_to_string(filename);
    if (source == NULL) return 1;
    FILE* h_file = fopen(out_h_file, "w");
    if (h_file == NULL) { fprintf(stderr, "Could not open file %s\n", out_h_file); free(source); return 1; }
    if (transpile(source, h_file, MODE_BUILD_LIB_HEADER) != 0) { fprintf(stderr, "Failed to build library header.\n"); fclose(h_file); free(source); remove(out_h_file); return 1; }
    fclose(h_file);
    free_symbol_table();
    FILE* c_file = fopen(out_c_file, "w");
    if (c_file == NULL) { fprintf(stderr, "Could not open file %s\n", out_c_file); free(source); return 1; }
    fprintf(c_file, "#include \"%s\"\n\n", out_h_file);
    if (transpile(source, c_file, MODE_BUILD_LIB_SOURCE) != 0) { fprintf(stderr, "Failed to build library source.\n"); fclose(c_file); free(source); remove(out_h_file); remove(out_c_file); return 1; }
    fclose(c_file);
    free_symbol_table();
    free(source);
    printf("Library '%s' and '%s' built successfully.\n", out_h_file, out_c_file);
    return 0;
}

static int handle_check(const char* filename) {
    char* source = read_file_to_string(filename);
    if (source == NULL) return 1;
    FILE* null_output = fopen("/dev/null", "w");
    if (null_output == NULL) { fprintf(stderr, "Could not open /dev/null\n"); free(source); return 1; }
    int result = transpile(source, null_output, MODE_CHECK);
    fclose(null_output);
    free_symbol_table();
    free(source);
    if (result == 0) { printf("Syntax OK\n"); }
    return result;
}

static int handle_init(const char* project_name) {
    if (mkdir(project_name, 0777) != 0) { perror("Error creating project directory"); return 1; }
    char src_path[1024];
    snprintf(src_path, sizeof(src_path), "%s/src", project_name);
    if (mkdir(src_path, 0777) != 0) { perror("Error creating src directory"); return 1; }
    char main_file_path[1024];
    snprintf(main_file_path, sizeof(main_file_path), "%s/main.super", src_path);
    FILE* main_file = fopen(main_file_path, "w");
    if (main_file == NULL) { fprintf(stderr, "Could not create main.super file.\n"); return 1; }
    fprintf(main_file, "fn main() -> int {\n");
    fprintf(main_file, "    print(\"Hello, %s!\");\n", project_name);
    fprintf(main_file, "    return 0;\n");
    fprintf(main_file, "}\n");
    fclose(main_file);
    printf("Project '%s' created successfully.\n", project_name);
    return 0;
}

static void handle_repl() {
    char line[1024];
    printf("Super REPL v0.1\n");
    printf("Enter an expression to check syntax, or 'exit' to quit.\n");
    for (;;) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        if (strcmp(line, "exit\n") == 0) {
            break;
        }
        transpile_repl(line);
    }
}

int main(int argc, char *argv[]) {
    int slang_mode = 0;
    int arg_offset = 1;
    if (argc > 1 && strcmp(argv[1], "--slang") == 0) {
        slang_mode = 1;
        set_slang_mode(1);
        arg_offset = 2;
    }
    if (argc < arg_offset + 1) { print_usage(argv[0]); return 1; }
    const char* command = argv[arg_offset];
    if (strcmp(command, "init") == 0 || (slang_mode && strcmp(command, "rizz") == 0)) { if (argc != arg_offset + 2) { fprintf(stderr, "Error: 'init' requires a project name.\n"); return 1; } return handle_init(argv[arg_offset + 1]); }
    if (strcmp(command, "build-lib") == 0 || (slang_mode && strcmp(command, "cook") == 0)) { if (argc != arg_offset + 2) { fprintf(stderr, "Error: 'build-lib' requires a filename.\n"); return 1; } return handle_build_lib(argv[arg_offset + 1]); }
    if (strcmp(command, "check") == 0) { if (argc != arg_offset + 2) { fprintf(stderr, "Error: 'check' requires a filename.\n"); return 1; } return handle_check(argv[arg_offset + 1]); }
    if (strcmp(command, "repl") == 0) { handle_repl(); return 0; }
    if (strcmp(command, "--help") == 0) { print_usage(argv[0]); return 0; }
    if (strcmp(command, "--show-tokens") == 0) { if (argc != arg_offset + 2) { fprintf(stderr, "Error: '--show-tokens' requires a filename.\n"); return 1; } show_tokens(argv[arg_offset + 1]); return 0; }
    if (argc == arg_offset + 1) { if (access(command, F_OK) == 0) { return transpile_and_run(command); } else { char src_path[1024]; snprintf(src_path, sizeof(src_path), "src/%s", command); if (access(src_path, F_OK) == 0) { return transpile_and_run(src_path); } } }
    print_usage(argv[0]);
    return 1;
}
