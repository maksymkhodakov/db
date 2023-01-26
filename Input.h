#pragma once
#include <stdio.h>
#include <string.h>
#include "Structures.h"
#include "Master.h"

void readMaster(struct Master* master)
{
	char name[16];
	int status;

	name[0] = '\0';

	printf("Enter master\'s name: ");
	scanf("%s", name);

	strcpy(master->name, name);

	printf("Enter master\'s status: ");
	scanf("%d", &status);

	master->status = status;
}

void readSlave(struct Slave* slave)
{
	int x;

	printf("Enter price: ");
	scanf("%d", &x);

	slave->price = x;

	printf("Enter amount: ");
	scanf("%d", &x);

	slave->amount = x;
}