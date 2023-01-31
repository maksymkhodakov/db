#pragma once
#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include "Entities.h"
#include "Validations.h"
#include "Train.h"

#define CREW_DATA "crew.fl"
#define CREW_GARBAGE "crew_garbage.txt"
#define CREW_SIZE sizeof(struct Crew)

struct Crew linkLoop(FILE *database, struct Train *train, struct Crew *previous);

int updateTrain(struct Train train, char* error);

struct Crew linkLoop(FILE *database, struct Train *train, struct Crew *previous) {
    for (int i = 0; i < (*train).crewsCount; i++) {
        fread(previous, CREW_SIZE, 1, database);
        fseek(database, (*previous).nextAddress, SEEK_SET);
    }
    return (*previous);
}

void reopenDatabase(FILE* database) {
	fclose(database);
	database = fopen(CREW_DATA, "r+b");
}

void linkAddresses(FILE* database, struct Train train, struct Crew crew) {
	reopenDatabase(database);
	struct Crew previous;
	fseek(database, train.firstCrewAddress, SEEK_SET);
    previous = linkLoop(database, &train, &previous);
    previous.nextAddress = crew.selfAddress;
	fwrite(&previous, CREW_SIZE, 1, database);
}

void relinkAddresses(FILE* database, struct Crew previous, struct Crew crew, struct Train* train) {
	if (crew.selfAddress == train->firstCrewAddress) {
		if (crew.selfAddress == crew.nextAddress) {
            train->firstCrewAddress = -1;
		} else {
            train->firstCrewAddress = crew.nextAddress;
		}
	} else {
		if (crew.selfAddress == crew.nextAddress) {
			previous.nextAddress = previous.selfAddress;
		}
		else {
			previous.nextAddress = crew.nextAddress;
		}
		fseek(database, previous.selfAddress, SEEK_SET);
		fwrite(&previous, CREW_SIZE, 1, database);
	}
}

void overwriteGarbageAddress(int garbageCount, FILE* garbageZone, struct Crew* record) {
	long* deletedIds = malloc(garbageCount * sizeof(long));
	for (int i = 0; i < garbageCount; i++) {
		fscanf(garbageZone, "%ld", deletedIds + i);
	}

	record->selfAddress = deletedIds[0];
	record->nextAddress = deletedIds[0];

	fclose(garbageZone);
	fopen(CREW_GARBAGE, "wb");
	fprintf(garbageZone, "%d", garbageCount - 1);

	for (int i = 1; i < garbageCount; i++) {
		fprintf(garbageZone, " %ld", deletedIds[i]);
	}

	free(deletedIds);
	fclose(garbageZone);
}

void noteDeletedCrew(long address) {
    FILE* garbageZone = fopen(CREW_GARBAGE, "rb");
    int garbageCount;
    fscanf(garbageZone, "%d", &garbageCount);
    long* deletedAddresses = malloc(garbageCount * sizeof(long));

    for (int i = 0; i < garbageCount; i++) {
        fscanf(garbageZone, "%ld", deletedAddresses + i);
    }

    fclose(garbageZone);
    garbageZone = fopen(CREW_GARBAGE, "wb");
    fprintf(garbageZone, "%d", garbageCount + 1);

    for (int i = 0; i < garbageCount; i++) {
        fprintf(garbageZone, " %ld", deletedAddresses[i]);
    }

    fprintf(garbageZone, " %ld", address);
    free(deletedAddresses);
    fclose(garbageZone);
}

int getCrew(struct Train train, struct Crew* crew, int crewId, char* error) {
    if (!train.crewsCount) {
        strcpy(error, "This train has no crew");
        return 0;
    }
    FILE* database = fopen(CREW_DATA, "rb");
    fseek(database, train.firstCrewAddress, SEEK_SET);
    fread(crew, CREW_SIZE, 1, database);

    for (int i = 0; i < train.crewsCount; i++) {
        if (crew->crewId == crewId) {
            fclose(database);
            return 1;
        }
        fseek(database, crew->nextAddress, SEEK_SET);
        fread(crew, CREW_SIZE, 1, database);
    }
    strcpy(error, "No such crew in database");
    fclose(database);
    return 0;
}

int insertCrew(struct Train train, struct Crew crew, char* error) {
    crew.exists = 1;
	FILE* database = fopen(CREW_DATA, "a+b");
	FILE* garbageZone = fopen(CREW_GARBAGE, "a+b");
	int garbageCount;											

	fscanf(garbageZone, "%d", &garbageCount);

	if (garbageCount) {
		overwriteGarbageAddress(garbageCount, garbageZone, &crew);
		reopenDatabase(database);
		fseek(database, crew.selfAddress, SEEK_SET);
	} else {
		fseek(database, 0, SEEK_END);
		int dbSize = ftell(database);
        crew.selfAddress = dbSize;
        crew.nextAddress = dbSize;
	}

	fwrite(&crew, CREW_SIZE, 1, database);
	if (!train.crewsCount) {
        train.firstCrewAddress = crew.selfAddress;
	} else {
		linkAddresses(database, train, crew);
	}
	fclose(database);
	train.crewsCount++;
    updateTrain(train, error);
	return 1;
}

int updateCrew(struct Crew crew, int crewId) {
	FILE* database = fopen(CREW_DATA, "r+b");
	fseek(database, crew.selfAddress, SEEK_SET);
	fwrite(&crew, CREW_SIZE, 1, database);
	fclose(database);
	return 1;
}

void deleteCrew(struct Train train, struct Crew crew, int crewId, char* error) {
	FILE* database = fopen(CREW_DATA, "r+b");
	struct Crew previous;
	fseek(database, train.firstCrewAddress, SEEK_SET);
	do {
		fread(&previous, CREW_SIZE, 1, database);
		fseek(database, previous.nextAddress, SEEK_SET);
	}
	while (previous.nextAddress != crew.selfAddress && crew.selfAddress != train.firstCrewAddress);

	relinkAddresses(database, previous, crew, &train);
    noteDeletedCrew(crew.selfAddress);
	crew.exists = 0;

	fseek(database, crew.selfAddress, SEEK_SET);
	fwrite(&crew, CREW_SIZE, 1, database);
	fclose(database);

	train.crewsCount--;
    updateTrain(train, error);
}