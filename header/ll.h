#define MAX_OPEN_DISK 64
#define BLCK_SIZE 1024 // 1024 Octects

// Description d'une partition
#define TTTFS_MAGIC_NUMBER 827541076 //(hex = 0x31534654)
#define TTTFS_VOLUME_BLOCK_SIZE 1024
#define TTTFS_VOLUME_BLOCK_COUNT(n) n
#define TTTFS_VOLUME_FREE_BLOCK_COUNT(n) n
#define TTTFS_VOLUME_FIRST_FREE_BLOCK(n) n
#define TTTFS_VOLUME_MAX_FILE_COUNT(n) n
#define TTTFS_VOLUME_FREE_FILE_COUNT(n) n
#define TTTFS_VOLUME_FIRST_FREE_FILE(n) n

// Types de fichiers
#define TFS_REGULAR 0
#define TFS_DIRECTORY 1
#define TFS_PSEUDO 2

// Sous type pseudos
#define TFS_DATE 0
#define TFS_DISK 1

// Erreurs
#define _NOERROR 0
#define _DISK_NOT_FOUND 1
#define _DISK_UNMOUNTED 2
#define _NUM_BLCK_TOO_BIG 3
#define _DISK_ID_TOO_BIG 4
#define _DISK_ID_EXIST 5
#define _READ_ERROR 6
#define _WRITE_ERROR 7
#define _POS_IN_BLCK_TOO_BIG 8
#define _PARTITION_NOT_FOUND 9
#define _MAX_FILES_TOO_BIG 10

// Définition des types
typedef int error;
typedef int disk_id;
typedef unsigned char block[BLCK_SIZE];

typedef struct{
	FILE *disk_descriptor;
	uint32_t nb_blocks; 
	int flag;
} DISK;

typedef struct{
	uint32_t tfs_size;
	int tfs_type;
	int tfs_subtype;
	uint32_t tfs_direct[10];
	uint32_t tfs_indirect1;
	uint32_t tfs_indirect2;
	uint32_t tfs_next_free;
} FILE_ENTRY;

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
void blockToLtleIndian(block b);
error writeIntToBlock(block b, int position, uint32_t number);
int readBlockToInt(block b, int position);
error eraseBlock(block b, int debut, int fin);
error eraseDisk(disk_id id, int block_debut, int block_fin);