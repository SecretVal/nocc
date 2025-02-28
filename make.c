#define STC_IMPLEMENTATION
#define STC_STRIP_PREFIX
#include <stc.h> 

#include <stdbool.h> 

#define project_name "./nocc"

void build(int argc, char **argv) {
    Cmd cmd = {0};
    cmd_push(&cmd, STC_COMPILER);
    cmd_push(&cmd, "-o", project_name);
    cmd_push(&cmd, "-Wall", "-Wextra", "-ggdb");
    cmd_push(&cmd, "main.c");
    if (argc >= 1) {
        char *sub_cmd = shift(argv, argc);
        if (strcmp(sub_cmd, "debug") == 0) {
            cmd_push(&cmd, "-DSTC_MIN_LOG_LEVEL=0");
        }
    }
    int res = cmd_exec(&cmd);
    if (res != 0) exit(res);
}

void run(int argc, char **argv) {
    Cmd cmd = {0};
    cmd_push(&cmd, project_name);
    while (argc > 0) {
        cmd_push(&cmd, shift(argv, argc));
    }
    int res = cmd_exec(&cmd);
    if (res != 0) exit(res);
}

int main(int argc, char **argv) {
    rebuild_file(argv, argc);
    (void*)shift(argv, argc);

    bool valid_cmd = false;
    if (!(argc >= 1)) {
        log(STC_ERROR, "Did not specify command: either ´run` or `build`");
        return 1;
    }
    char *sub_cmd = shift(argv, argc);
    if (strcmp(sub_cmd, "run") == 0) {
        char **args = malloc(sizeof(char*)*sizeof("debug"));
        args[0] = "debug";
        build(1, args);
        run(argc, argv);
        valid_cmd = true;
    }

    if (strcmp(sub_cmd, "build") == 0) {
        build(argc, argv);
        valid_cmd = true;
    }

    if (!valid_cmd) {
        log(STC_ERROR, " %s is not a known command", sub_cmd);
    }
    return 0;
}
