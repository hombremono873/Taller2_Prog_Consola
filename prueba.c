
/*Insercción de librerias*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE_LENGTH 1024
void procesoTres(int argc, char *argv[]){
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <archivo_de_comandos>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *comandos_file = fopen(argv[1], "r");
    if (comandos_file == NULL) {
        perror("Error al abrir el archivo de comandos");
        exit(EXIT_FAILURE);
    }

    char *linea = NULL;
    size_t longitud_linea = 0;

    // Leer cada línea del archivo de comandos
    while (getline(&linea, &longitud_linea, comandos_file) != -1) {
        // Eliminar el carácter de nueva línea, si está presente
        linea[strcspn(linea, "\n")] = '\0';

        pid_t pid = fork();

        if (pid < 0) {
            perror("Error en fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Proceso hijo
            char *linea_copia = strdup(linea); // Copiar la línea de comandos
            if (linea_copia == NULL) {
                perror("Error al copiar la línea de comandos");
                exit(EXIT_FAILURE);
            }

            char *token;
            const char *delim = " ";
            char *args[64]; // Máximo de 64 argumentos

            // Parsear la línea en argumentos utilizando strsep
            int i = 0;
            char *temp_linea = linea_copia;
            while ((token = strsep(&temp_linea, delim)) != NULL && i < 63) {
                args[i++] = token;
            }
            args[i] = NULL;

            // Ejecutar el comando utilizando execvp
            if (execvp(args[0], args) == -1) {
                perror("Error al ejecutar el comando");
                exit(EXIT_FAILURE);
            }
        } else {
            // Proceso padre
            // Esperar a que el hijo termine
            wait(NULL);
        }
    }
    // Liberar memoria y cerrar el archivo
    free(linea);
    fclose(comandos_file);
}
int main(int argc, char *argv[]) {
    procesoTres(argc, argv);
    return 0;
}