#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE_LENGTH 100

int main(int argc, char *argv[]) {
    // Verificar que se haya proporcionado un argumento (nombre del archivo de comandos)
    if (argc != 2) {
        fprintf(stderr, "Uso: %s comandos.txt\n", argv[0]);
        exit(EXIT_FAILURE); // Salir del programa con un código de error
    }

    // Intenta abrir el archivo de comandos en modo de lectura ("r")
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        // Usar fprintf para imprimir el mensaje de error en stderr
        fprintf(stderr, "Error al abrir el archivo %s\n", argv[1]);
        exit(EXIT_FAILURE); // Salir del programa con un código de error
    }

    // Variable para almacenar cada línea leída del archivo
    char line[MAX_LINE_LENGTH];

    // Leer y ejecutar los comandos desde el archivo de comandos
    while (fgets(line, sizeof(line), file)) {
        // Eliminar el carácter de nueva línea de la línea leída
        line[strcspn(line, "\n")] = 0;

        // Crear una copia de la línea actual
        char *line_copy = strdup(line);
        if (line_copy == NULL) {
            // Usar fprintf para imprimir el mensaje de error en stderr
            fprintf(stderr, "Error al duplicar la línea\n");
            exit(EXIT_FAILURE); // Salir del programa con un código de error
        }

        // Variables para almacenar el comando y sus argumentos
        char command[MAX_LINE_LENGTH];
        char **args = NULL;
        int arg_count = 0;

        // Parsear la línea para obtener el comando y sus argumentos
        char *token = strsep(&line_copy, " ");
        strcpy(command, token); // El primer token es el comando
        token = strsep(&line_copy, " "); // Saltar el comando

        // Almacenar los argumentos en un array dinámico
        while (token != NULL) {
            args = realloc(args, (arg_count + 1) * sizeof(char *));
            if (args == NULL) {
                // Usar fprintf para imprimir el mensaje de error en stderr
                fprintf(stderr, "Error al asignar memoria\n");
                exit(EXIT_FAILURE); // Salir del programa con un código de error
            }
            args[arg_count] = strdup(token);
            if (args[arg_count] == NULL) {
                // Usar fprintf para imprimir el mensaje de error en stderr
                fprintf(stderr, "Error al duplicar la cadena\n");
                exit(EXIT_FAILURE); // Salir del programa con un código de error
            }
            token = strsep(&line_copy, " ");
            arg_count++;
        }
        args = realloc(args, (arg_count + 1) * sizeof(char *));
        if (args == NULL) {
            // Usar fprintf para imprimir el mensaje de error en stderr
            fprintf(stderr, "Error al asignar memoria\n");
            exit(EXIT_FAILURE); // Salir del programa con un código de error
        }
        args[arg_count] = NULL; // Marcar el final del array de argumentos

        // Crear un proceso hijo
        pid_t pid = fork();

        // Verificar si el proceso hijo se creó correctamente
        if (pid == -1) {
            // Usar fprintf para imprimir el mensaje de error en stderr
            fprintf(stderr, "Error al crear el proceso hijo\n");
            exit(EXIT_FAILURE); // Salir del programa con un código de error
        }

        // En el proceso hijo, ejecutar el comando
        if (pid == 0) {
            execvp(command, args);
            // Si execvp devuelve, ha ocurrido un error
            // Usar fprintf para imprimir el mensaje de error en stderr
            fprintf(stderr, "Error al ejecutar el comando %s\n", command);
            exit(EXIT_FAILURE); // Salir del proceso hijo con un código de error
        }

        // Liberar la memoria utilizada para almacenar los argumentos
        for (int i = 0; i < arg_count; i++) {
            free(args[i]);
        }
        free(args);

        // Esperar a que el proceso hijo termine
        int status;
        if (wait(&status) == -1) {
            // Usar fprintf para imprimir el mensaje de error en stderr
            fprintf(stderr, "Error al esperar al proceso hijo\n");
            exit(EXIT_FAILURE); // Salir del programa con un código de error
        }
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                // Usar fprintf para imprimir el mensaje de error en stderr
                fprintf(stderr, "El comando %s finalizó con un código de error: %d\n", command, WEXITSTATUS(status));
                exit(EXIT_FAILURE); // Salir del programa con un código de error
            }
        } else {
            // Usar fprintf para imprimir el mensaje de error en stderr
            fprintf(stderr, "El comando %s finalizó de manera anormal\n", command);
            exit(EXIT_FAILURE); // Salir del programa con un código de error
        }
    }

    // Cerrar el archivo
    fclose(file);

    return 0; // Salir del programa con éxito
}