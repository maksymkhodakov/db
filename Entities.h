#pragma once

struct Train {
	int id;
	char name[16];
	int size;
    char color[16];
    char type[16];
	long firstCrewAddress;
	int crewsCount;
};

struct Crew {
	int trainId;
	int crewId;
    char crewName[20];
    int crewAmount;
    int ratingScore;
	int exists;
	long selfAddress;
	long nextAddress;
};

struct Indexer {
	int id;	
	int address;
	int exists;
};