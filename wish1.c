/*  
    Programa: Ejecución de lote de comandos
    Autores:  [Laidy Castaño Castaño], [Yuly Yecenia Albear Romo], [Omar Alberto Torres]
    Profesor: [Dany Alexandro Munera ]
    Curso:    [Sistemas operativos y laboratorio]
    Fecha:    [Abril 10 del 2024]
*/
/********************************Insercción de librería standard***********************************/
//Correcto falta funcionalidad deredireccion
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/*********************************Definición de constantes del programa ****************************/

#define MAXIMA_LONGITUD_PATH 512
#define MAXIMA_LONGITUD_COMANDO 512
#define MAXIMOS_ARGUMENTOS 256
#define EXEC_SUCCESS(status) ((status) == 0) 
//#define  MAX_INPUT_LENGTH 1024
/**************************************** Variables globales ***************************************/

char **rutas; 
int nume_rutas = 0;

/*********************************** Definición prototipo de funciones *****************************/

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
void redirigir_salida_entrada_a_archivos(char *comando);

/***************************************** Función principal ***************************************/

/** La funcion main controla el flujo del program y ciclo del proceso interactivo ******************/
int main(int argc, char *argv[]) {
    inicializar_rutas();
    char comando[MAXIMA_LONGITUD_COMANDO];
    if (argc == 2) {
        procesoTres(argc, argv);            //Se hace proceso batch
        return 0;
    }
    while (1) {
        prompt();
        fgets(comando, MAXIMA_LONGITUD_COMANDO, stdin);        //Se lee entrada estandar
        if (strcmp(comando, "\n") == 0) { // Verificar si se ingresó solo el salto de línea
            continue; // Volver a solicitar el comando
        }
        if (contiene_ampersand(comando)) {
            proceso(comando); // Se ejecuta comandos en segundo plano
        } else if (strchr(comando, '>') || strchr(comando, '<')) {    //Usuario hace uso del redireccionamiento
            redirigir_salida_entrada_a_archivos(comando); // Invoca la función de redirección directamente
        } else {
            procesoOne(comando); // Llama a procesoOne() para procesar el comando interno/externo
        }
    }
    return 0;
}

/******************************** Procesos por lotes **********************************************/
/**La funcion ProcesoTres responsable del proesamiendo del lote de comandos*/
/**/
void procesoTres(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <commands_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *comandos_file = fopen(argv[1], "r");
    if (comandos_file == NULL) {
        perror("Error opening commands file");
        exit(EXIT_FAILURE);
    }

    char *linea = NULL;
    size_t longitud_linea = 0;

    // Leer cada línea del archivo de comandos
    while (getline(&linea, &longitud_linea, comandos_file) != -1) {
        // Eliminar el carácter de nueva línea, si está presente
        linea[strcspn(linea, "\n")] = '\0';

        // Salir del bucle si se encuentra el comando "exit"
        if (strcmp(linea, "exit") == 0) {
            break;
        }

        pid_t pid = fork(); // Crear proceso hijo
        if (pid < 0) {
            perror("Error en fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Proceso hijo
            // Ejecutar el comando utilizando system
            if (system(linea) == -1) {
                perror("Error executing command");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS); // Salir del proceso hijo después de ejecutar el comando
        } else {
            // Proceso padre
            // Esperar a que el hijo termine
            wait(NULL);
        }
    }

    // Liberar memoria y cerrar el archivo
    free(linea);
    fclose(comandos_file);

    // Salir del programa después de procesar todas las líneas del archivo
    exit(EXIT_SUCCESS);
}
/**La función prompt() muestra simbolo */
void prompt() {
    printf("wish> ");
}
/**Toma una linea de entrada y la divide en tokens que se almacenan en el arreglo de cadenas args*/

void parsear_comando(char *comando, char **args) {
    char *token;                                       //Puntero token(argumento o comando)
    int i = 0;
    //Se ejecuta mientras existan tokens y no se haya alcanzado el maximo numero de tokens
    while ((token = strsep(&comando, " \n")) != NULL && i < MAXIMOS_ARGUMENTOS - 1) {
        if (*token != '\0') {
            args[i++] = token;                     //Se almacena token
        }
    }
    args[i] = NULL;       //Marca el final de la lista de argumentos
}
/**Esta funcion es responsable de ejecutar los comandos externos (Comandos que no son internos al shell)*/
/** Comandos externos como: ls, ls -a, cat, grep, wc cp , mv, mkdir etc*/
void ejecutar_comando_externo(char **args) {
    pid_t pid;                                         //Identificador del hijo
    int status;

    for (int i = 0; i < nume_rutas; i++) {           //Se itera sobre las rutas 
        char comando[MAXIMA_LONGITUD_COMANDO];       //Array para el comando
        strcpy(comando, rutas[i]);                   //Se copia la ruta en el arreglo comando
        strcat(comando, args[0]);                    // Se concatena el primer argumento(nombre del comando)  al final de la ruta
        
        pid = fork();                               //Se crea un hijo quien hereda el codigo del padre
        if (pid == 0) {
            execvp(comando, args);                  //Se ejecuta el comadno 
            exit(1);                                //Si hay retorno es que hubo falla
        } else if (pid < 0) {
            perror("fork");                         //Se sale con error
            exit(1);
        } else {
            waitpid(pid, &status, 0);
            if (EXEC_SUCCESS(status)) {  // Uso de la macro definida, sale siretornó un cero
                return;
            }
        }
    }
    fprintf(stderr, "An error has occurred\n");
}

/*
* Esta funcion es responsable de la ejecución de comandos internos del shell com por ejemplo:
* 1. exit
* 2. cd
* . path
*/

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
/***Inicializa ruta para busqueda de comandos************/
void inicializar_rutas() {
    rutas = malloc(MAXIMA_LONGITUD_PATH * sizeof(char*));
    rutas[0] = strdup("/bin/");
    rutas[1] = strdup("/usr/bin/");
    rutas[2] = strdup("/path/to/date/directory/");
    rutas[3] = strdup("/path/to/pwd/directory/");
    nume_rutas = 4;
}

/*Se parsea comandos para operaciones en segundo plano*/
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


/*Esta función ejecuta comandos, ya sea en primer plano o en segundo plano, según se especifique.*/
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
/***Funcion que controla flujo para ejecucion de funciones en segundo plano****/
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

/*Esta función se encarga de controlar el flujo para la ejecución de comandos en segundo plano y la detección de la salida (exit).*/
void  proceso(char * comando){
     if (strcmp(comando, "exit\n") == 0) {
        exit(0);
     }
     procesoTwo(comando);
}

/**La deteccion del simbolo & es fundamental para el contexto y ir a ejecucion comandos segundo plano*/
int contiene_ampersand(const char *cadena) {
    return strchr(cadena, '&') != NULL;
}

/**La funcion controla el flujo hacia ejecutar comandos internos o externos**/

void procesoOne(char *comando) {
    char *args[MAXIMOS_ARGUMENTOS];
    
    if (strcmp(comando, "exit\n") == 0) {
        exit(0);
    }
    // Parsear el comando
    parsear_comando(comando, args);
    if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "cd") == 0 || strcmp(args[0], "path") == 0) {
        ejecutar_comando_interno(args);
    } else {
        ejecutar_comando_externo(args);
    }
}
/** Redirige la entrada y salida estándar según sea necesario **/
void redirigir_salida_entrada_a_archivos(char *comando) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Buscar el operador de redirección ">" en el comando
        char *posicion_redireccion_salida = strstr(comando, ">");
        if (posicion_redireccion_salida != NULL) {
            // Obtener el nombre del archivo de salida después del operador ">"
            char *nombre_archivo_salida = strtok(posicion_redireccion_salida + 1, " \t\n");
            if (nombre_archivo_salida != NULL) {
                // Abrir el archivo de salida
                int fd = open(nombre_archivo_salida, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }

                // Redirigir la salida estándar al archivo
                dup2(fd, STDOUT_FILENO);
                close(fd);

                // Eliminar el operador de redirección y el nombre del archivo de salida del comando
                *posicion_redireccion_salida = '\0';
            } else {
                // Si no hay nada después del operador de redirección, mostrar un mensaje de error
                printf("Error: No se proporcionó un nombre de archivo de salida después del operador de redirección.\n");
                exit(EXIT_FAILURE);
            }
        }

        // Buscar el operador de redirección "<" en el comando
        char *posicion_redireccion_entrada = strstr(comando, "<");
        if (posicion_redireccion_entrada != NULL) {
            // Obtener el nombre del archivo de entrada después del operador "<"
            char *nombre_archivo_entrada = strtok(posicion_redireccion_entrada + 1, " \t\n");
            if (nombre_archivo_entrada != NULL) {
                // Abrir el archivo de entrada
                int fd = open(nombre_archivo_entrada, O_RDONLY);
                if (fd < 0) {
                    perror("open");
                    exit(1);
                }

                // Redirigir la entrada estándar desde el archivo
                dup2(fd, STDIN_FILENO);
                close(fd);

                // Eliminar el operador de redirección y el nombre del archivo de entrada del comando
                *posicion_redireccion_entrada = '\0';
            } else {
                // Si no hay nada después del operador de redirección, mostrar un mensaje de error
                printf("Error: No se proporcionó un nombre de archivo de entrada después del operador de redirección.\n");
                exit(EXIT_FAILURE);
            }
        }

        // Ejecutar el comando con el shell
        execlp("sh", "sh", "-c", comando, NULL);

        // Si execlp falla, mostrar un mensaje de error
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
        // Proceso padre
        int status;
        waitpid(pid, &status, 0);
    }
}
