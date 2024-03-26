#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 100
#define MAXIMA_LONGITUD_PATH 100

char **rutas;
int nume_rutas;

void inicializar_rutas() {
    rutas = malloc(MAXIMA_LONGITUD_PATH * sizeof(char*));
    rutas[0] = strdup("/bin/");
    rutas[1] = strdup("/usr/bin/");
    // Puedes agregar más rutas aquí según sea necesario
    nume_rutas = 2; // Ajusta esto al número total de rutas que has inicializado
}

int main() {
    // Inicializar las rutas
    inicializar_rutas();

    // Abrir el archivo
    FILE *file = fopen("comandos.txt", "r");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    // Leer y ejecutar cada línea del archivo
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        // Eliminar el salto de línea si existe
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';
        
        // Intentar ejecutar el comando con cada ruta
        int i;
        int comando_ejecutado = 0;
        for (i = 0; i < nume_rutas; i++) {
            char comando[MAX_LINE_LENGTH + MAXIMA_LONGITUD_PATH];
            snprintf(comando, sizeof(comando), "%s", rutas[i]);

            // Intentar ejecutar el comando
            printf("Ejecutando comando: %s%s\n", comando, line);
            if (system(strcat(comando, line)) == 0) {
                comando_ejecutado = 1;
                break;
            }
        }

        // Si el comando no se pudo ejecutar con ninguna ruta, mostrar error
        if (!comando_ejecutado) {
            fprintf(stderr, "Error al ejecutar el comando: %s\n", line);
        }
    }

    // Cerrar el archivo
    fclose(file);

    // Liberar memoria de las rutas
    for (int i = 0; i < nume_rutas; i++) {
        free(rutas[i]);
    }
    free(rutas);

    return 0;
}