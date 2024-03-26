#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXIMA_LONGITUD_PATH 200
#define MAXIMA_LONGITUD_COMANDO 200
#define MAXIMO_ARGUMENTOS 100
#define MAXIMA_LONGITUD_COMANDOS 200
#define MAXIMOS_ARGUMENTOS 100
#define EXEC_SUCCESS(status) ((status) == 0) 

char **rutas; 
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
            exit(1);
        } else if (pid < 0) {
            perror("fork");
            exit(1);
        } else {
            waitpid(pid, &status, 0);
            if (EXEC_SUCCESS(status)) {  // Uso de la macro definida
                return;
            }
        }
    }
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
        if (args[1] != NULL) {
            for (int i = 0; i < nume_rutas; i++) {
                free(rutas[i]);
            }
            free(rutas);
            nume_rutas = 0;
            rutas = malloc(MAXIMA_LONGITUD_PATH * sizeof(char*));
            int i = 1;
            while (args[i] != NULL) {
                rutas[nume_rutas] = strdup(args[i]);
                nume_rutas++;
                i++;
            }
            if (nume_rutas >= MAXIMA_LONGITUD_PATH) {
                fprintf(stderr, "An error has occurred\n");
            }
        } else {
                for (int i = 0; i < nume_rutas; i++) {
                printf("%s\n", rutas[i]);
            }
        }
    } else {
       // printf("Comando interno ejecutado: %s\n", args[0]);
    }
}

void inicializar_rutas() {
    rutas = malloc(MAXIMA_LONGITUD_PATH * sizeof(char*));
    rutas[0] = strdup("/bin/");
    rutas[1] = strdup("/usr/bin/");
    rutas[2] = strdup("/path/to/date/directory/");
    rutas[3] = strdup("/path/to/pwd/directory/");
    nume_rutas = 4;
}

void procesoOne(char *comando) {
    char *args[MAXIMO_ARGUMENTOS];
    
    if (strcmp(comando, "exit\n") == 0) {
        exit(0);
    }
    
    parsear_comando(comando, args);
    
    if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "cd") == 0 || strcmp(args[0], "path") == 0) {
        ejecutar_comando_interno(args);
    } else {
        ejecutar_comando_externo(args);
    }
}
/**SEgunda parte*/
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
        fprintf(stderr, "An error has occurred\n");
        exit(1);
    } else if (pid < 0) {
        fprintf(stderr, "An error has occurred\n");
        exit(1);
    } else {
        if (!segundoplano) {
            waitpid(pid, &status, 0);
        }
    }
}

void procesoTwo(char *comando) {
    char *args[MAXIMOS_ARGUMENTOS];
    int segundoplano = 0;

    // Dividir el comando en comandos individuales
    char *token;
    token = strtok(comando, "\n"); // Usar el salto de línea como delimitador
    while (token != NULL) {
        parsear_comandos(token, args, &segundoplano);
        ejecutar_comando(args, segundoplano);
        token = strtok(NULL, "\n");
        // Si el comando se ejecuta en segundo plano, no esperar a que termine
        segundoplano = 0;
    }
}
void  proceso(char * comando){
     if (strcmp(comando, "exit\n") == 0) {
        exit(0);
     }
     procesoTwo(comando);
}
int contiene_ampersand(const char *cadena) {
    return strchr(cadena, '&') != NULL;
}

int main(int argc, char *argv[]) {
    inicializar_rutas();
    
    char comando[MAXIMA_LONGITUD_COMANDO];
    
    while (1) {
        prompt();
        fgets(comando, MAXIMA_LONGITUD_COMANDO, stdin);
        if(contiene_ampersand(comando)) 
          proceso(comando);
        else  procesoOne(comando); // Llama a procesoOne() para procesar el comando
       
    }
    
    return 0;
}

