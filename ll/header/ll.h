#define MAX_OPEN_DISK 64
#define BLCK_SIZE 1024 // 1024 Octects
#define PATH_SATATFS "/tfs/" // Emplacement des disques TFS

#define _NOERROR 0
#define _ERROR 1

typedef struct{
	int errno;
} error;

typedef struct{
	int id;
} disk_id;

typedef struct{
	disk_id id;
	int disk_descriptor;
	int size;
} DISK;

typedef struct{
	char* donnees;
} block;

error start_disk(char *name, disk_id *id);
error read_block(disk_id id, block b, uint32_t num);
error write_block(disk_id id, block b, uint32_t num);
error sync_disk(disk_id id);
error stop_disk(disk_id id);