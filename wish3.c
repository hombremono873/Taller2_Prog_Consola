#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

#define MAXIMA_LONGITUD_COMANDOS 200
#define MAXIMOS_ARGUMENTOS 100

void prompt() {
    printf("wish> ");
}

void parsear_comandos(char *comando, char **args, int *segundoplano) {
    char *token;
    int i = 0;
   
    token = strtok(comando, " \n");
    while (token != NULL && i < MAXIMOS_ARGUMENTOS - 1) {
        args[i] = token;
        if (strcmp(token, "&") == 0) {
            *segundoplano = 1;
            args[i] = NULL; // Eliminar el "&" de los argumentos
            break;
        }
        token = strtok(NULL, " \n");
        i++;
    }
    args[i] = NULL;
}

void ejecutar_comando(char **args, int segundoplano) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        // Usar fprintf para imprimir el mensaje de error en stderr
        fprintf(stderr, "An error has occurred\n");
        exit(1);
    } else if (pid < 0) {
        // Error en la creación del proceso hijo
        // Usar fprintf para imprimir el mensaje de error en stderr
        fprintf(stderr, "An error has occurred\n");
        exit(1);
    } else {
        // Proceso padre
        if (!segundoplano) {
            // Esperar a que el proceso hijo termine si no está en segundo plano
            waitpid(pid, &status, 0);
        }
    }
}

void procesoTwo(int argc, char *argv[]){
   char command[MAXIMA_LONGITUD_COMANDOS];
    char *args[MAXIMOS_ARGUMENTOS];
    int segundoplano = 0;

    while (1) {
        prompt();
        fgets(command, MAXIMA_LONGITUD_COMANDOS, stdin);
        if (strcmp(command, "exit\n") == 0) {
            exit(0);
        }

        // Dividir el comando en comandos individuales
        char *token;
        token = strtok(command, "\n"); // Usar el salto de línea como delimitador
        while (token != NULL) {
            parsear_comandos(token, args, &segundoplano);
            ejecutar_comando(args, segundoplano);
            token = strtok(NULL, "\n");
            // Si el comando se ejecuta en segundo plano, no esperar a que termine
            segundoplano = 0;
        }
    }
}
void flujoPrograma(int argc, char *argv[]){
     procesoTwo(argc,argv);
}
int main(int argc, char *argv[]) {
    flujoPrograma(argc, argv);
    return 0;
}