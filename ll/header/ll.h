#define MAX_OPEN_DISK 64
#define BLCK_SIZE 1024 // 1024 Octects
#define PATH_TFS "/tfs/" // Emplacement des disques TFS

// Erreurs
#define _NOERROR 0
#define _DISK_NOT_FOUND 1
#define _DISK_UNMOUNTED 2
#define _NUM_BLCK_TOO_BIG 3
#define _DISK_ID_TOO_BIG 4
#define _DISK_ID_EXIST 5
#define _READ_ERROR 6

// Définition des types
typedef int error;
typedef int disk_id;
typedef char block[BLCK_SIZE];

typedef struct{
	FILE *disk_descriptor;
	int nb_blocks; 
	int flag;
} DISK;

//_________________________________________________________
// Fonctions principales
error start_disk(char *name, disk_id id);
error read_block(disk_id id, block b, uint32_t num);
error write_block(disk_id id, block b, uint32_t num);
error sync_disk(disk_id id);
error stop_disk(disk_id id);

//_________________________________________________________
// Fonctions auxiliaires
char* strError(error err);
void printBlock(block b);