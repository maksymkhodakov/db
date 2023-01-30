#include <stdio.h>
#include "Entities.h"
#include "Train.h"
#include "Crew.h"
#include "Input.h"
#include "Output.h"

int main() {
	struct Train train;
	struct Crew crew;
	while (1) {
		int choice;
		int id;
		char error[51];
		printf("Options:\n0 EXIT\n1 Insert Train\n2 Get Train\n3 Update Train\n4 Delete Train\n5 Insert Crew\n6 Get Crew\n7 Update Crew\n8 Delete Crew\n9 General Info\n");
        printf("ENTER SOMETHING TO PROCEED\n");
		scanf("%d", &choice);
		switch (choice) {
			case 0:
				return 0;
			case 1:
                readTrain(&train);
                insertTrain(train);
				break;
			case 2:
				printf("Enter ID: ");
				scanf("%d", &id);
                getTrain(&train, id, error) ? printTrain(train) : printf("Error: %s\n", error);
				break;
			case 3:
				printf("Enter ID: ");
				scanf("%d", &id);
                train.id = id;
                readTrain(&train);
                updateTrain(train, error) ? printf("Updated successfully\n") : printf("Error: %s\n", error);
				break;
			case 4:
				printf("Enter ID: ");
				scanf("%d", &id);
                deleteTrain(id, error) ? printf("Deleted successfully\n") : printf("Error: %s\n", error);
				break;
			case 5:
				printf("Enter train ID: ");
				scanf("%d", &id);
				if (getTrain(&train, id, error)) {
                    crew.trainId = id;
					printf("Enter crew ID: ");
					scanf("%d", &id);
					if (checkKeyPairUnique(train, id)) {
                        crew.crewId = id;
                        readCrew(&crew);
                        insertCrew(train, crew, error);
						printf("Insertion complete!!!. To get crew use train\'s and crew\'s IDs\n");
					} else {
						printf("Error: non-unique key\n");
					}
				} else {
					printf("Error: %s\n", error);
				}
				break;
			case 6:
				printf("Enter train ID: ");
				scanf("%d", &id);
				if (getTrain(&train, id, error)) {
					printf("Enter crew ID: ");
					scanf("%d", &id);
                    getCrew(train, &crew, id, error) ? printCrew(crew, train) : printf("Error: %s\n", error);
				} else {
					printf("Error: %s\n", error);
				}
				break;
			case 7:
				printf("Enter train ID: ");
				scanf("%d", &id);
				if (getTrain(&train, id, error)) {
					printf("Enter crew ID: ");
					scanf("%d", &id);
					if (getCrew(train, &crew, id, error)) {
                        readCrew(&crew);
                        updateCrew(crew, id);
						printf("Updated successfully\n");
					} else {
						printf("Error: %s\n", error);
                    }
                } else {
					printf("Error: %s\n", error);
				}
				break;
			case 8:
				printf("Enter train ID: ");
				scanf("%d", &id);
				if (getTrain(&train, id, error)) {
					printf("Enter crew ID: ");
					scanf("%d", &id);
					if (getCrew(train, &crew, id, error)) {
                        deleteCrew(train, crew, id, error);
						printf("Deleted successfully\n");
					} else {
						printf("Error: %s\n", error);
					}
				} else {
					printf("Error: %s\n", error);
				}
				break;
			case 9:
				info();
				break;
			default:
				printf("Invalid input, TRY AGAIN!!!\n");
		}
		printf("==============\n");
	}
}