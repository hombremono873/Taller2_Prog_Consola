/********************************Insercción de librería standard***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*********************************Definición de constantes del programa ****************************/
#define MAXIMA_LONGITUD_PATH 200
#define MAXIMA_LONGITUD_COMANDO 200
#define MAXIMOS_ARGUMENTOS 100
//#define MAXIMA_LONGITUD_COMANDOS 200
#define MAXIMA_LONGITUD_LINEA 200
//#define MAXIMOS_ARGUMENTOS 100
#define EXEC_SUCCESS(status) ((status) == 0) 
#define MAX_LINE_LENGTH 1024
/****************************************Variables globales ***************************************/
char **rutas; 
int nume_rutas = 0;

FILE* getFile(int argc, char *argv[]);
char** almacenarArgumentos(char *token, char *line_copy) ;
void prompt();
void parsear_comando(char *comando, char **args);
void ejecutar_comando_externo(char **args);
void ejecutar_comando_interno(char **args);
void inicializar_rutas();
void procesoOne(char *comando);
void procesoTres(int argc, char *argv[]);
void parsear_comandos(char *comando, char **args, int *segundoplano);
void ejecutar_comando(char **args, int segundoplano);
void procesoTwo(char *comando);
void  proceso(char * comando);
int contiene_ampersand(const char *cadena);

int main(int argc, char *argv[]) {
    inicializar_rutas();
    char comando[MAXIMA_LONGITUD_COMANDO];
    if (argc == 2) {
        procesoTres(argc, argv);
        return 0;
    }
    while (1) {
        prompt();
        fgets(comando, MAXIMA_LONGITUD_COMANDO, stdin);
        if (strcmp(comando, "\n") == 0) { // Verificar si se ingresó solo el salto de línea
            continue; // Volver a solicitar el comando
        }
        if (contiene_ampersand(comando)) 
            proceso(comando);
        procesoOne(comando); // Llama a procesoOne() para procesar el comando
    }
    return 0;
}
/********************************************Proceso por lotes***************************************/

/*FILE* getFile(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "An error has occurred\n");
        exit(1);
    }
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "An error has occurred\n");
        exit(1); 
    }

    return file;
}

char** almacenarArgumentos(char *token, char *line_copy) {
    char **args = NULL;
    int arg_count = 0;
    while (token != NULL) {
        args = realloc(args, (arg_count + 1) * sizeof(char *));
        if (args == NULL) {
            fprintf(stderr, "An error has occurred\n");
            exit(1);
        }
        args[arg_count] = strdup(token);
        if (args[arg_count] == NULL) {
            fprintf(stderr, "An error has occurred\n");
            exit(1);
        }
        token = strsep(&line_copy, " ");   //Funciona muy bien usando strsept()
        arg_count++;
    }

    return args;
}

void procesoTres(int argc, char *argv[]) {
    char line[MAXIMA_LONGITUD_LINEA];
    FILE *file = getFile(argc, argv);
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        char *line_copy = strdup(line);
        if (line_copy == NULL) {
            fprintf(stderr, "An error has occurred\n");
            exit(1);
        }
        char command[MAXIMA_LONGITUD_LINEA];
        char **args = NULL;
        int arg_count = 0;
        char *token = strsep(&line_copy, " ");
        strcpy(command, token);
        token = strsep(&line_copy, " ");
        args = almacenarArgumentos(token, line_copy);
        for (int i = 0; i < arg_count; i++) {
            free(args[i]);
        }
        free(args);
        pid_t pid = fork();
        if (pid == -1) {
            fprintf(stderr, "An error has occurred\n");
            exit(1);
        }
        if (pid == 0) {
            execvp(command, args);
            fprintf(stderr, "An error has occurred\n");
            exit(1);
        }

        int status;
        if (wait(&status) != -1) {  // Verificar si el proceso hijo ha terminado correctamente
            fclose(file);
            continue;
        } else {
            fprintf(stderr, "An error has occurred\n");
            exit(1);
        }
    }
}
*/
/*****************************************************************************************************/
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
void prompt() {
    printf("wish> ");
}

void parsear_comando(char *comando, char **args) {
    char *token;
    int i = 0;
    while ((token = strsep(&comando, " \n")) != NULL && i < MAXIMOS_ARGUMENTOS - 1) {
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
    char *args[MAXIMOS_ARGUMENTOS];
    
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
