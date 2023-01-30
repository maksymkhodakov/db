#pragma once
#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Validations.h"
#include "Crew.h"

#define TRAIN_IND "train.ind"
#define TRAIN_DATA "train.fl"
#define TRAIN_GARBAGE "train_garbage.txt"
#define INDEXER_SIZE sizeof(struct Indexer)
#define TRAIN_SIZE sizeof(struct Train)

void noteDeletedTrain(int id) {
	FILE* garbageZone = fopen(TRAIN_GARBAGE, "rb");
	int garbageAmount;
	fscanf(garbageZone, "%d", &garbageAmount);
	int* deleteIds = malloc(garbageAmount * sizeof(int));

	for (int i = 0; i < garbageAmount; i++) {
		fscanf(garbageZone, "%d", deleteIds + i);
	}

	fclose(garbageZone);
	garbageZone = fopen(TRAIN_GARBAGE, "wb");
	fprintf(garbageZone, "%d", garbageAmount + 1);

	for (int i = 0; i < garbageAmount; i++) {
		fprintf(garbageZone, " %d", deleteIds[i]);
	}

	fprintf(garbageZone, " %d", id);
	free(deleteIds);
	fclose(garbageZone);
}

void overwriteGarbageId(int garbageCount, FILE* garbageZone, struct Train* record) {
	int* deleteIds = malloc(garbageCount * sizeof(int));

	for (int i = 0; i < garbageCount; i++) {
		fscanf(garbageZone, "%d", deleteIds + i);
	}
	record->id = deleteIds[0];

	fclose(garbageZone);
	fopen(TRAIN_GARBAGE, "wb");
	fprintf(garbageZone, "%d", garbageCount - 1);

	for (int i = 1; i < garbageCount; i++) {
		fprintf(garbageZone, " %d", deleteIds[i]);
	}

	free(deleteIds);
	fclose(garbageZone);
}

int insertTrain(struct Train record) {
	FILE* indexTable = fopen(TRAIN_IND, "a+b");
	FILE* database = fopen(TRAIN_DATA, "a+b");
	FILE* garbageZone = fopen(TRAIN_GARBAGE, "a+b");
	struct Indexer indexer;
	int garbageCount = 0;
	fscanf(garbageZone, "%d", &garbageCount);

	if (garbageCount) {
		overwriteGarbageId(garbageCount, garbageZone, &record);
		fclose(indexTable);
		fclose(database);
		indexTable = fopen(TRAIN_IND, "r+b");
		database = fopen(TRAIN_DATA, "r+b");
		fseek(indexTable, (record.id - 1) * INDEXER_SIZE, SEEK_SET);
		fread(&indexer, INDEXER_SIZE, 1, indexTable);
		fseek(database, indexer.address, SEEK_SET);
	} else {
		long indexerSize = INDEXER_SIZE;
		fseek(indexTable, 0, SEEK_END);
		if (ftell(indexTable)) {
			fseek(indexTable, -indexerSize, SEEK_END);
			fread(&indexer, INDEXER_SIZE, 1, indexTable);
			record.id = indexer.id + 1;
		} else {
			record.id = 1;
		}
	}
	record.firstCrewAddress = -1;
	record.crewsCount = 0;
	fwrite(&record, TRAIN_SIZE, 1, database);

	indexer.id = record.id;
	indexer.address = (record.id - 1) * TRAIN_SIZE;
	indexer.exists = 1;
	printf("Your train id is %d\n", record.id);

	fseek(indexTable, (record.id - 1) * INDEXER_SIZE, SEEK_SET); 
	fwrite(&indexer, INDEXER_SIZE, 1, indexTable);
	fclose(indexTable);
	fclose(database);

	return 1;
}

int getTrain(struct Train* train, int id, char* error) {
	FILE* indexTable = fopen(TRAIN_IND, "rb");
	FILE* database = fopen(TRAIN_DATA, "rb");
	if (!checkFileExists(indexTable, database, error)) {
		return 0;
	}
	struct Indexer indexer;
	if (!checkIndexExists(indexTable, error, id)) {
		return 0;
	}
	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);
	fread(&indexer, INDEXER_SIZE, 1, indexTable);
	if (!checkRecordExists(indexer, error)) {
		return 0;
	}
	fseek(database, indexer.address, SEEK_SET);
	fread(train, sizeof(struct Train), 1, database);
	fclose(indexTable);
	fclose(database);
	return 1;
}

int updateTrain(struct Train train, char* error) {
	FILE* indexTable = fopen(TRAIN_IND, "r+b");
	FILE* database = fopen(TRAIN_DATA, "r+b");
    struct Indexer indexer;
    int id = train.id;
	if (!checkFileExists(indexTable, database, error)) {
		return 0;
	}
	if (!checkIndexExists(indexTable, error, id)) {
		return 0;
	}
	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);
	fread(&indexer, INDEXER_SIZE, 1, indexTable);
	if (!checkRecordExists(indexer, error)) {
		return 0;
	}
	fseek(database, indexer.address, SEEK_SET);
	fwrite(&train, TRAIN_SIZE, 1, database);
	fclose(indexTable);
	fclose(database);
	return 1;
}

int deleteTrain(int id, char* error) {
	FILE* indexTable = fopen(TRAIN_IND, "r+b");
    struct Train train;
    struct Indexer indexer;
    if (indexTable == NULL) {
		strcpy(error, "database files are not created yet");
		return 0;
	}
	if (!checkIndexExists(indexTable, error, id)) {
		return 0;
	}
    getTrain(&train, id, error);
	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);
	fread(&indexer, INDEXER_SIZE, 1, indexTable);
	indexer.exists = 0;
	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);
	fwrite(&indexer, INDEXER_SIZE, 1, indexTable);
	fclose(indexTable);
    noteDeletedTrain(id);
	
	if (train.crewsCount) {
		FILE* crewDb = fopen(CREW_DATA, "r+b");
		struct Crew crew;
		fseek(crewDb, train.firstCrewAddress, SEEK_SET);
		for (int i = 0; i < train.crewsCount; i++) {
			fread(&crew, CREW_SIZE, 1, crewDb);
			fclose(crewDb);
            deleteCrew(train, crew, crew.crewId, error);
            crewDb = fopen(CREW_DATA, "r+b");
			fseek(crewDb, crew.nextAddress, SEEK_SET);
		}
		fclose(crewDb);
	}
	return 1;
}