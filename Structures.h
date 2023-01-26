#pragma once

/*
"Structures.h": master (supplier), product (detail), slave (supplement), indexer.
*/

struct Master
{
	int id;
	char name[16];
	int status;
	long firstSlaveAddress;
	int slavesCount;
};

struct Slave
{
	int masterId;
	int productId;
	int price;
	int amount;
	int exists;
	long selfAddress;
	long nextAddress;
};

struct Indexer
{
	int id;	
	int address;
	int exists;
};