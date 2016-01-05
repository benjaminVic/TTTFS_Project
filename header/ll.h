#include <stdint.h>

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
#define _DIRECT_TAB_IS_EMPTY 17

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

//NOTE CARTOUCHE : les ou sont toujours les codes d'erreur sauf si

/*
Démarre le disque et le rajoute dans la varialbe d'environnement _disks
Ouvre le fichier en lecture/ecriture (r+)
@param *name : nom du disque
@param id : numéro disque dans _disks
*/
error start_disk(char *name, disk_id id);
/*
Lit un emplacement mémoire
@param id : id du disk
@param b : block recevant les informations
@param num : numéro du block auquel on accède
*/
error read_block(disk_id id, block b, uint32_t num);
/*
Ecrit dans un emplacement mémoire
@param id : id du disk
@param b : block recevant les informations
@param num : numéro du block auquel on accède
*/
error write_block(disk_id id, block b, uint32_t num);
error sync_disk(disk_id id);
/*
Ferme le ficher .tfs et désactive le pointeur sur le disk
@param id : numéro de disque dans _disks
*/
error stop_disk(disk_id id);

//_________________________________________________________
// Fonctions informations DISQUE et PARTITIONS

/*
Mêt à jour number avec le numéro du premier bloc mémoire de partition de id
@param id : id du disk
@param partition : numero de la partition
@param *number : valeur récupérée
*/
error getFirstPartitionBlck(disk_id id, int partition, uint32_t *number);
/*
Mêt à jour number avec la taille en nb de block de partition de id
@param id : id du disk
@param partition : numero de la partition
@param *number : valeur récupérée
*/
error getPartitionSize(disk_id id, int partition, uint32_t *number);
/*
Mêt à jour number avec la taille en nb de block de la table des ficher
de partition de id
@param id : id du disk
@param partition : numero de la partition
@param *number : valeur récupérée
*/
error getFilesTableSize(disk_id id, int partition, uint32_t *number);

/*
Transmet via *infDisk la taille du disk ainsi que le nb de partition et taille
@param id : id du disk
@param *infDisk : valeurs récupérée
*/
error readDiskInfos(disk_id id, DISK_INFO *infDisk);


/*
Indique la taille et les information d'espaces de block via *infPartition
@param id : id du disk
@param *infPartition : 
@param partition : numéro de la partition
*/
error readPartitionInfos(disk_id id, PARTITION_INFO *infPartition, int partition);
/*
Crée la taille et les information d'espaces de block via *infPartition
@param id : id du disk
@param *infPartition : 
@param partition : numéro de la partition
*/
error writePartitionInfos(disk_id id, PARTITION_INFO infPartition, int partition);

/*
Affiche les information de infPartition
@param infPartition : information de la partition à afficher
*/
void printInfoPartition(PARTITION_INFO infPartition);

//_________________________________________________________
// Fonctions auxiliaires

/*
Affiche le message correspondant à une erreur donnée
@param err : constante d'une erreur
*/
char* strError(error err);
/*
Réecris tous les bloques du disque avec une valeur vide
@param id : id du disk
@param block_debut : bloque d'origine
@param block_fin : bloque de fin de la réecritureb
*/
error eraseDisk(disk_id id, int block_debut, int block_fin);

//_________________________________________________________
// Fonctions blocks
/*
Converti b en little indian (inverse l'ordre des octets)
@param b : block a inverser
*/
void blockToLtleIndian(block b);

/*
Ecrit un int à une position d'un block
@param b : block où doit être inscirt l'int
@param position : position où on inscrira dans le block
@param number : Int a inscrire (occupe 32bits)
*/
error writeIntToBlock(block b, int position, uint32_t number);
/*
Lit l'int inscrit dans un block
@param b : block où doit être lu l'int
@param position : emplacement à lire
@out uint32_t : valeur lue a l'emplacement ou -1
*/
uint32_t readBlockToInt(block b, int position);

/*
Ecrit une chaine de caractère à position dans un block
@param b : block où doit être inscrit la chaine de caractère
@param position : position où on inscrira dans le block
@param *str : tableau de char à inscrire
@param size : taille du tableau de caractère à inscrire
*/
error writeStrToBlock(block b, int position, char *str, int size);
/*
Lit l'int inscrit dans un block
@param b : block où doit être lu la chaine de caractère
@param position : emplacement à lire
@param *str : valeur lue à l'emplacement
@param size : taille du tableau de caractère à lire
*/
error readBlockToStr(block b, int position, char *str, int size);

/*
Crée un dossier
@param b : block destination
@param position : position où on inscrira dans le block
@param dir_ent : numéro de fichier & le nom de l'entrée
*/
error writeDirEntryToBlock(block b, int position, DIR_ENTRY dir_ent);
/*
Lis un dossier
@param b : block destination
@param position : position où on inscrira dans le block
@param dir_ent : numéro de fichier & le nom de l'entrée
*/
error readBlockToDirEntry(block b, int position, DIR_ENTRY *dir_ent);

/*
Libère de la place dans un block
@param b : block à partiellemnt vider
@param debut : position de début
@param fin : position de fin
*/
error eraseBlock(block b, int debut, int fin);
/*
Affiche le contenu d'un block
@param b : block
*/
void printBlock(block b);

//_________________________________________________________
// Fonctions table des fichiers
/*
Initialise la table des fichiers en la remplissant d'entrées vierge
@param id : id du disk
@param partition : partition pourlaquelle la table doit être créée
*/
error initFilesTable(disk_id id, int partition);

/*
Ecrit les information d'un ficher dans la table des fichiers
@param disk_id : id du disque concerné
@param partition : partition cible du disk
@param file_ent : informations du fichier
@param file_pos : position
*/
error writeFileEntryToTable(disk_id, int partition, FILE_ENTRY file_ent, int file_pos);

/*
Lit les information d'un ficher dans la table des fichiers
@param disk_id : id du disque concerné
@param partition : partition cible du disk
@param file_ent : informations du fichier
@param file_pos : position
*/
error readFileEntryFromTable(disk_id, int partition, FILE_ENTRY *file_ent, int file_pos);

/*
Ajoute un fichier dans la table des entrées et mêt à jour les info de la partition
@param id : id du disque concerné
@param partition : partition cible du disk
@param file_size : taille du fichier
@param file_type : indication de type de l'entrée
@param file_subtype : sous-type, pris en compte si type = 0;
*/
error addFileEntryToTable(disk_id id, int partition, int file_size, int file_type, int file_subtype);

/*
Remplace le fichier à file_pos par une entrée vide
@param id : id du disque concerné
@param partition : partition cible du disk
@param file_pos : position du ficher à supprimer
*/
error removeFileEntryInTable(disk_id id, int partition, int file_pos);

/*
Affiche le contenu de la table des fichiers
@param file_ent : table des fichier a afficher
*/
void printFileEntry(FILE_ENTRY file_ent);

//_________________________________________________________
// Fonctions chainages blocs
/*
Remplis de block contenant uniquement le numéro de block le
reste du tableau pour initialiser et chainer les block
@param id : id du disque concerné
@param partition : partition cible du disk
*/
error initFreeBlockChain(disk_id id, int partition);
/*
Supprime le dernier bloc libre en le rendant premier emplacement libre
et mêt à jour les infos de partition
@param id : id du disque concerné
@param partition : partition cible du disk
*/
error removeFirstBlockInChain(disk_id id, int partition);
/*
Rajoute le block au premier emplacement libre 
et mêt à jour les infos de partition
@param id : id du disque concerné
@param partition : partition cible du disk
*/
error addBlockInChain(disk_id id, int partition, int number);

//_________________________________________________________
// Fonctions entrées de fichier
/*
Initialise *file_ent pour initFilesTable
@param *file_ent : variable à initiliser
*/
void initFileEntry(FILE_ENTRY *file_ent);
/*

@param id : id du disque concerné
@param partition : partition cible du disk
*/
error addNewBlock(disk_id id, int partition, int file_pos);
/*

@param id : id du disque concerné
@param partition : partition cible du disk
*/
error removeLastBlock(disk_id id, int partition, int file_pos);






