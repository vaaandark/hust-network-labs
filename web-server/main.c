#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "def.h"
#include "file.h"
#include "server.h"

int listener;

void usage(const char *command) {
    fprintf(stderr, "Usage: %s -c configuration\n", command);
}

void clean_up(int __attribute__((unused)) sig) {
    close(listener);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    int opt;
    char *config = NULL;
    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
            case 'c':
                config = optarg;
                break;
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    if (config == NULL) {
        fprintf(stderr, "%s: has no configuration file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ServerInfo server_info = server_new(config);

    struct sigaction act;
    act.sa_handler = clean_up;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESETHAND;
    sigaction(SIGINT, &act, 0);

    listener = server_setup(&server_info);
    if (listener == -1) {
        INFO("Cannot setup server\n");
    }

    server_go(&server_info, listener);

    return 0;
}
