#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//Programa que ejecuta comandos internos y externos funcionando

#define MAXIMA_LONGITUD_PATH 200
#define MAXIMA_LONGITUD_COMANDO 200
#define MAXIMO_ARGUMENTOS 100

char **rutas; // Puntero a cadena de caracteres que contiene lista de paths
int nume_rutas = 0;

void prompt() {
    printf("wish> ");
}
void parsear_comando(char *comando, char **args) {
    char *token;
    int i = 0;
    while ((token = strsep(&comando, " \n")) != NULL && i < MAXIMO_ARGUMENTOS - 1) {
        if (*token != '\0') {
            args[i++] = token;
        }
    }
    args[i] = NULL;
}

void ejecutar_comando_externo(char **args) {
    pid_t pid;
    int status;

    for (int i = 0; i < nume_rutas; i++) {
        char comando[MAXIMA_LONGITUD_COMANDO];
        strcpy(comando, rutas[i]);
        strcat(comando, args[0]);
        
        pid = fork();
        if (pid == 0) {
            execvp(comando, args);
            fprintf(stderr, "An error has occurred\n");
            exit(1);
        } else if (pid < 0) {
            perror("fork");
            exit(1);
        } else {
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                return;
            }
        }
    }
    // Si el comando no se encontró en ninguna ruta, imprimir un mensaje de error
    fprintf(stderr, "An error has occurred\n");
}

void ejecutar_comando_interno(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            chdir(getenv("HOME"));
        } else {
            chdir(args[1]);
        }
    } else if (strcmp(args[0], "path") == 0) {
        // Comando para cambiar las rutas de búsqueda de ejecutables
        if (args[1] != NULL) {
            // Liberar la memoria de las rutas anteriores
            for (int i = 0; i < nume_rutas; i++) {
                free(rutas[i]);
            }
            free(rutas); // Liberar la memoria del arreglo de punteros

            nume_rutas = 0;

            // Crear un nuevo arreglo de rutas
            rutas = malloc(MAXIMA_LONGITUD_PATH * sizeof(char*));

            // Agregar nuevas rutas
            int i = 1;
            while (args[i] != NULL) {
                rutas[nume_rutas] = strdup(args[i]);
                nume_rutas++;
                i++;
            }
            if (nume_rutas >= MAXIMA_LONGITUD_PATH) {
                fprintf(stderr, "Se ha alcanzado el límite máximo de rutas.\n");
            }
        } else {
            // Imprimir las rutas actuales
            printf("Rutas actuales:\n");
            for (int i = 0; i < nume_rutas; i++) {
                printf("%s\n", rutas[i]);
            }
        }
    } else {
        // Otros comandos internos
        printf("Comando interno ejecutado: %s\n", args[0]);
    }
}

void procesoOne(int argc, char *argv[]){
 // Crear el arreglo de rutas vacío
    rutas = malloc(MAXIMA_LONGITUD_PATH * sizeof(char*));

    char comando[MAXIMA_LONGITUD_COMANDO];
    char *args[MAXIMO_ARGUMENTOS];

    // Crear algunas rutas iniciales
    rutas[0] = strdup("/bin/");
    rutas[1] = strdup("/usr/bin/");
    rutas[2] = strdup("/path/to/date/directory/");
    rutas[3] = strdup("/path/to/pwd/directory/");

    // Calcular el número de rutas
    nume_rutas = 0;
    while (rutas[nume_rutas] != NULL) {
        nume_rutas++;
    }

    while (1) {
        prompt();
        fgets(comando, MAXIMA_LONGITUD_COMANDO, stdin);
        if (strcmp(comando, "exit\n") == 0) {
            exit(0);
        }
        parsear_comando(comando, args);
        
        // Verificar si el comando es interno o externo
        if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "cd") == 0 || strcmp(args[0], "path") == 0) {
            // Comando interno
            ejecutar_comando_interno(args);
        } else {
            // Comando externo
            ejecutar_comando_externo(args);
        }
    }
}
void flujoPrograma(int argc, char *argv[]){
     procesoOne(argc, argv);
}
int main(int argc, char *argv[]) {
    flujoPrograma(argc, argv);
    return 0;
} 
