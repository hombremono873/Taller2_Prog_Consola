#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 4
#define MAX_LINE_LENGTH 1024

void execute_command(char *command) {
    printf("Ejecutando comando: %s\n", command);
    system(command);
}

void procesoTres(char *commands[]) {
    for (int i = 0; i < MAX_COMMANDS; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Proceso hijo
            execute_command(commands[i]);
            exit(EXIT_SUCCESS);
        }
        // Proceso padre
        int status;
        waitpid(pid, &status, 0);
    }
}

char** construirArrayComandos(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char **commands = malloc(MAX_COMMANDS * sizeof(char *));
    if (commands == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    int command_count = 0;
    while (fgets(line, sizeof(line), file) != NULL && command_count < MAX_COMMANDS) {
        // Eliminar el salto de línea al final de la línea
        line[strcspn(line, "\n")] = '\0';
        // Copiar la línea al array de comandos
        commands[command_count] = strdup(line);
        if (commands[command_count] == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        command_count++;
    }

    fclose(file);
    return commands;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <comandos.txt>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char **commands = construirArrayComandos(argv[1]);
    procesoTres(commands);

    // Liberar la memoria asignada para los comandos
    for (int i = 0; i < MAX_COMMANDS; i++) {
        free(commands[i]);
    }
    free(commands);

    return EXIT_SUCCESS;
}