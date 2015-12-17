#define MAX_OPEN_DISK 64
#define BLCK_SIZE 1024 // 1024 Octects
#define MAGIC_NUMBER 827541076 //(hex = 0x31534654)

// Types de fichiers
#define TFS_REGULAR 0
#define TFS_DIRECTORY 1
#define TFS_PSEUDO 2

// Sous type pseudos
#define TFS_DATE 0
#define TFS_DISK 1

// Flags
#define _UNMOUNTED 0
#define _MOUNTED 1

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
#define _POS_IN_TABLE_TOO_BIG 11
#define _POS_IN_PARTITION_TOO_BIG 12
#define _TABLE_IS_FULL 13
#define _CANNOT_REMOVE_FILE_ENTRY 14
#define _PARTITION_IS_FULL 15
#define _DIRECT_TAB_IS_FULL 16

// Définition des types
typedef int error;
typedef int disk_id;
typedef unsigned char block[BLCK_SIZE];

typedef struct{
	FILE *disk_descriptor;
	uint32_t nb_blocks; 
	uint32_t flag;
} DISK;

typedef struct{
	uint32_t size;
	uint32_t nb_partitions;
	uint32_t p_sizes[(BLCK_SIZE/4)-2];
} DISK_INFO;

typedef struct{
	uint32_t TTTFS_MAGIC_NUMBER;
	uint32_t TTTFS_VOLUME_BLOCK_SIZE;
	uint32_t TTTFS_VOLUME_BLOCK_COUNT;
	uint32_t TTTFS_VOLUME_FREE_BLOCK_COUNT;
	uint32_t TTTFS_VOLUME_FIRST_FREE_BLOCK;
	uint32_t TTTFS_VOLUME_MAX_FILE_COUNT;
	uint32_t TTTFS_VOLUME_FREE_FILE_COUNT;
	uint32_t TTTFS_VOLUME_FIRST_FREE_FILE;
} PARTITION_INFO;

typedef struct{
	uint32_t tfs_size;
	uint32_t tfs_type;
	uint32_t tfs_subtype;
	uint32_t tfs_direct[10];
	uint32_t tfs_indirect1;
	uint32_t tfs_indirect2;
	uint32_t tfs_next_free;
} FILE_ENTRY;

typedef struct{
	uint32_t number;
	char name[28];
} DIR_ENTRY;

//_________________________________________________________
// Fonctions principales
error start_disk(char *name, disk_id id);
error read_block(disk_id id, block b, uint32_t num);
error write_block(disk_id id, block b, uint32_t num);
error sync_disk(disk_id id);
error stop_disk(disk_id id);

//_________________________________________________________
// Fonctions informations DISQUE et PARTITIONS
error getFirstPartitionBlck(disk_id id, int partition, uint32_t *number);
error getPartitionSize(disk_id id, int partition, uint32_t *number);
error getFilesTableSize(disk_id id, int partition, uint32_t *number);

error readDiskInfos(disk_id id, DISK_INFO *infDisk);

error readPartitionInfos(disk_id id, PARTITION_INFO *infPartition, int partition);
error writePartitionInfos(disk_id id, PARTITION_INFO infPartition, int partition);

void printInfoPartition(PARTITION_INFO infPartition);

//_________________________________________________________
// Fonctions auxiliaires
char* strError(error err);
error eraseDisk(disk_id id, int block_debut, int block_fin);

//_________________________________________________________
// Fonctions blocks
void blockToLtleIndian(block b);

error writeIntToBlock(block b, int position, uint32_t number);
uint32_t readBlockToInt(block b, int position);

error writeStrToBlock(block b, int position, char *str, int size);
error readBlockToStr(block b, int position, char *str, int size);

error writeDirEntryToBlock(block b, int position, DIR_ENTRY dir_ent);
error readBlockToDirEntry(block b, int position, DIR_ENTRY *dir_ent);

error eraseBlock(block b, int debut, int fin);
void printBlock(block b);

//_________________________________________________________
// Fonctions table des fichiers
error initFilesTable(disk_id id, int partition);
error initFreeBlockChain(disk_id id, int partition);

error writeFileEntryToTable(disk_id, int partition, FILE_ENTRY file_ent, int file_pos);
error readFileEntryFromTable(disk_id, int partition, FILE_ENTRY *file_ent, int file_pos);

error addFileEntryToTable(disk_id id, int partition, int file_size, int file_type, int file_subtype);
error removeFileEntryInTable(disk_id id, int partition, int file_pos);

void printFileEntry(FILE_ENTRY file_ent);

//_________________________________________________________
// Fonctions entrées de fichier
void initFileEntry(FILE_ENTRY *file_ent);
error addNewBlock(disk_id id, int partition, int file_pos);






