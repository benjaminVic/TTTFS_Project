#include <stdio.h>
#include <stdlib.h>

#define BLCK_SIZE 1024 // 1024 Octects
#define MAX_SIZE 4294967295 // Nombre de blocks maximum (Taille du disque 2^32-1)

int create_disk(uint32_t size, char* name);