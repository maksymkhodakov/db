#pragma once
#include <stdio.h>
#include <string.h>
#include "Entities.h"
#include "Train.h"

void readTrain(struct Train* train) {
	char name[16];
    int size;
    char color[16];
    char type[16];

	name[0] = '\0';
	printf("Enter train\'s name: ");
	scanf("%s", name);
	strcpy(train->name, name);

	printf("Enter train\'s size: ");
	scanf("%d", &size);
    train->size = size;

    color[0] = '\0';
    printf("Enter train\'s color: ");
    scanf("%s", color);
    strcpy(train->color, color);

    type[0] = '\0';
    printf("Enter train\'s type: ");
    scanf("%s", type);
    strcpy(train->type, type);
}

void readCrew(struct Crew* crew) {
    char crewName[20];
    int crewAmount;
    int ratingScore;

    crewName[0] = '\0';
    printf("Enter crew name: ");
    scanf("%s", crewName);
    strcpy(crew->crewName, crewName);

	printf("Enter crew amount: ");
	scanf("%d", &crewAmount);
    crew->crewAmount = crewAmount;

	printf("Enter rating score of crew: ");
	scanf("%d", &ratingScore);
    crew->ratingScore = ratingScore;
}