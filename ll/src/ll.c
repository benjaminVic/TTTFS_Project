#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../header/ll.h"


error read_physical_block(disk_id id, block b, uint32_t num){
	error errorTest;
	errorTest.errno = 0;
	return errorTest;
}

error write_physical_block(disk_id id, block b, uint32_t num){
	error errorTest;
	errorTest.errno = 0;
	return errorTest;
}

int main(int argc, char *argv[]){
	//****************************
	// ZONE DE TEST
	//****************************

	disk_id disk0;
	disk0.id = 0;

	block blockA;
	blockA.donnees = malloc(BLCK_SIZE * sizeof(char));

	read_physical_block(disk0, blockA, 0);

	free(blockA.donnees);

	return 0;
}
