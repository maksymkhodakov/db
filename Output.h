#pragma once

#include <stdio.h>
#include "Train.h"
#include "Entities.h"

void printTrain(struct Train train) {
	printf("Train\'s name: %s\n", train.name);
	printf("Train\'s color: %s\n", train.color);
	printf("Train\'s crews count: %d\n", train.crewsCount);
	printf("Train\'s size: %d\n", train.size);
	printf("Train\'s type: %s\n", train.type);
}

void printCrew(struct Crew crew, struct Train train) {
	printf("Train info: %s, %s \n", train.name, train.type);
	printf("Crew name: %s\n", crew.crewName);
	printf("Crew amount: %d\n", crew.crewAmount);
	printf("Crew rating score: %d\n", crew.ratingScore);
}