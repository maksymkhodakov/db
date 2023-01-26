#pragma once

#include <stdio.h>
#include "Master.h"
#include "Structures.h"

void printMaster(struct Master master)
{
	printf("Master\'s name: %s\n", master.name);
	printf("Master\'s status: %d\n", master.status);
}

void printSlave(struct Slave slave, struct Master master)
{
	printf("Master: %s, %d scores\n", master.name, master.status);
	printf("Price: %d\n", slave.price);
	printf("Amount: %d\n", slave.amount);
}