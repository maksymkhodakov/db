#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "Crew.h"

int getTrain(struct Train* train, int id, char* error);

void loopThroughTrains(int indAmount, struct Train *train, char *dummy, int *trainCount, int *crewCount);

int checkFileExists(FILE* indexTable, FILE* database, char* error) {
	if (indexTable == NULL || database == NULL) {
		strcpy(error, "DB files do not exits");
		return 0;
	}
	return 1;
}

int checkIndexExists(FILE* indexTable, char* error, int id) {
	fseek(indexTable, 0, SEEK_END);
	long indexTableSize = ftell(indexTable);
	if (indexTableSize == 0 || id * sizeof(struct Indexer) > indexTableSize) {
		strcpy(error, "no such ID in table");
		return 0;
	}
	return 1;
}

int checkRecordExists(struct Indexer indexer, char* error) {
	if (!indexer.exists) {
		strcpy(error, "the record has been deleted");
		return 0;
	}
	return 1;
}

int checkKeyPairUnique(struct Train train, int crewId) {
	FILE* crewsDb = fopen(CREW_DATA, "a+b");
	struct Crew crew;
	fseek(crewsDb, train.firstCrewAddress, SEEK_SET);
	for (int i = 0; i < train.crewsCount; i++) {
		fread(&crew, CREW_SIZE, 1, crewsDb);
		fclose(crewsDb);
		if (crew.crewId == crewId) {
			return 0;
		}
        crewsDb = fopen(CREW_DATA, "r+b");
		fseek(crewsDb, crew.nextAddress, SEEK_SET);
	}
	fclose(crewsDb);
	return 1;
}

void info() {
	FILE* indexTable = fopen("train.ind", "rb");
	if (indexTable == NULL) {
		printf("Error: database files do not exist\n");
		return;
	}
	int trainCount = 0;
	int crewCount = 0;
	fseek(indexTable, 0, SEEK_END);
	int indAmount = ftell(indexTable) / sizeof(struct Indexer);
	struct Train train;
	char dummy[51];
    loopThroughTrains(indAmount, &train, dummy, &trainCount, &crewCount);
    fclose(indexTable);
	printf("Total trains: %d\n", trainCount);
	printf("Total crews: %d\n", crewCount);
}

void loopThroughTrains(int indAmount, struct Train *train, char *dummy, int *trainCount, int *crewCount) {
    for (int i = 1; i <= indAmount; i++) {
        if (getTrain(train, i, dummy)) {
            (*trainCount)++;
            (*crewCount) += (*train).crewsCount;
            printf("Train #%d has %d crews\n", i, (*train).crewsCount);
        }
    }
}
