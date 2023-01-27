#pragma once
#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Checks.h"
#include "Slave.h"

#define MASTER_IND "master.ind"
#define MASTER_DATA "master.fl"
#define MASTER_GARBAGE "master_garbage.txt"
#define INDEXER_SIZE sizeof(struct Indexer)
#define MASTER_SIZE sizeof(struct Master)


/*
Functions performing all the needed operations on the masters.
*/

void noteDeletedMaster(int id)
{
	FILE* garbageZone = fopen(MASTER_GARBAGE, "rb");		// "rb": відкриваємо бінарний файл для читання

	int garbageCount;
	fscanf(garbageZone, "%d", &garbageCount);

	int* delIds = malloc(garbageCount * sizeof(int));		// Виділяємо місце під список "сміттєвих" індексів

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageZone, "%d", delIds + i);				// Заповнюємо його
	}

	fclose(garbageZone);									// За допомогою цих двох команд
	garbageZone = fopen(MASTER_GARBAGE, "wb");				// повністю очищуємо файл зі "сміттям"
	fprintf(garbageZone, "%d", garbageCount + 1);			// Записуємо нову кількість "сміттєвих" індексів

	for (int i = 0; i < garbageCount; i++)
	{
		fprintf(garbageZone, " %d", delIds[i]);				// Заносимо "сміттєві" індекси назад...
	}

	fprintf(garbageZone, " %d", id);						// ...і дописуємо до них індекс щойно видаленого запису
	free(delIds);											// Звільняємо виділену під масив пам'ять
	fclose(garbageZone);									// Закриваємо файл
}

void overwriteGarbageId(int garbageCount, FILE* garbageZone, struct Master* record)
{
	int* delIds = malloc(garbageCount * sizeof(int));		// Виділяємо місце під список "сміттєвих" індексів

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageZone, "%d", delIds + i);				// Заповнюємо його
	}

	record->id = delIds[0];									// Для запису замість логічно видаленого "сміттєвого"

	fclose(garbageZone);									// За допомогою цих двох команд
	fopen(MASTER_GARBAGE, "wb");							// повністю очищуємо файл зі "сміттям"
	fprintf(garbageZone, "%d", garbageCount - 1);			// Записуємо нову кількість "сміттєвих" індексів

	for (int i = 1; i < garbageCount; i++)
	{
		fprintf(garbageZone, " %d", delIds[i]);				// Записуємо решту "сміттєвих" індексів
	}

	free(delIds);											// Звільняємо виділену під масив пам'ять
	fclose(garbageZone);									// Закриваємо файл
}

int insertMaster(struct Master record)
{
	FILE* indexTable = fopen(MASTER_IND, "a+b");			// "a+b": відкрити бінарний файл
	FILE* database = fopen(MASTER_DATA, "a+b");				// для запису в кінець та читання
	FILE* garbageZone = fopen(MASTER_GARBAGE, "a+b");		// "rb": відкрити бінарний файл для читання
	struct Indexer indexer;
	int garbageCount = 0;

	fscanf(garbageZone, "%d", &garbageCount);

	if (garbageCount)										// Наявні "сміттєві" записи, перепишемо перший з них
	{
		overwriteGarbageId(garbageCount, garbageZone, &record);

		fclose(indexTable);									// Закриваємо файли для зміни
		fclose(database);									// режиму доступу в подальшому

		indexTable = fopen(MASTER_IND, "r+b");				// Знову відкриваємо і змінюємо режим на
		database = fopen(MASTER_DATA, "r+b");				// "читання з та запис у довільне місце файлу"

		fseek(indexTable, (record.id - 1) * INDEXER_SIZE, SEEK_SET);
		fread(&indexer, INDEXER_SIZE, 1, indexTable);
		fseek(database, indexer.address, SEEK_SET);			// Ставимо курсор на "сміття" для подальшого перезапису	
	}
	else                                                    // Видалених записів немає
	{
		long indexerSize = INDEXER_SIZE;

		fseek(indexTable, 0, SEEK_END);						// Ставимо курсор у кінець файлу таблички

		if (ftell(indexTable))								// Розмір індексної таблички ненульовий (позиція від початку)
		{
			fseek(indexTable, -indexerSize, SEEK_END);		// Ставимо курсор на останній індексатор
			fread(&indexer, INDEXER_SIZE, 1, indexTable);	// Читаємо останній індексатор

			record.id = indexer.id + 1;						// Нумеруємо запис наступним індексом
		}
		else                                                // Індексна табличка порожня
		{
			record.id = 1;									// Індексуємо наш запис як перший
		}
	}

	record.firstSlaveAddress = -1;
	record.slavesCount = 0;

	fwrite(&record, MASTER_SIZE, 1, database);				// Записуємо в потрібне місце БД-таблички передану структуру

	indexer.id = record.id;									// Вносимо номер запису в індексатор
	indexer.address = (record.id - 1) * MASTER_SIZE;		// Вносимо адресу запису в індексатор
	indexer.exists = 1;										// Прапорець існування запису

	printf("Your master\'s id is %d\n", record.id);

	fseek(indexTable, (record.id - 1) * INDEXER_SIZE, SEEK_SET); 
	fwrite(&indexer, INDEXER_SIZE, 1, indexTable);			// Записуємо індексатор у відповідну табличку, куди треба
	fclose(indexTable);										// Закриваємо файли
	fclose(database);

	return 1;
}

int getMaster(struct Master* master, int id, char* error)
{
	FILE* indexTable = fopen(MASTER_IND, "rb");				// "rb": відкрити бінарний файл
	FILE* database = fopen(MASTER_DATA, "rb");				// тільки для читання
	
	if (!checkFileExistence(indexTable, database, error))
	{
		return 0;
	}
	
	struct Indexer indexer;

	if (!checkIndexExistence(indexTable, error, id))
	{
		return 0;
	}

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);	// Отримуємо індексатор шуканого запису
	fread(&indexer, INDEXER_SIZE, 1, indexTable);			// за вказаним номером

	if (!checkRecordExistence(indexer, error))
	{
		return 0;
	}

	fseek(database, indexer.address, SEEK_SET);				// Отримуємо шуканий запис з БД-таблички
	fread(master, sizeof(struct Master), 1, database);		// за знайденою адресою
	fclose(indexTable);										// Закриваємо файли
	fclose(database);

	return 1;
}

int updateMaster(struct Master master, char* error)
{
	FILE* indexTable = fopen(MASTER_IND, "r+b");			// "r+b": відкрити бінарний файл
	FILE* database = fopen(MASTER_DATA, "r+b");				// для читання та запису

	if (!checkFileExistence(indexTable, database, error))
	{	
		return 0;
	}

	struct Indexer indexer;
	int id = master.id;

	if (!checkIndexExistence(indexTable, error, id))
	{
		return 0;
	}

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);	// Отримуємо індексатор шуканого запису
	fread(&indexer, INDEXER_SIZE, 1, indexTable);			// за вказаним номером

	if (!checkRecordExistence(indexer, error))
	{
		return 0;
	}

	fseek(database, indexer.address, SEEK_SET);				// Позиціонуємося за адресою запису в БД
	fwrite(&master, MASTER_SIZE, 1, database);				// Оновлюємо запис
	fclose(indexTable);										// Закриваємо файли
	fclose(database);

	return 1;
}

int deleteMaster(int id, char* error)
{
	FILE* indexTable = fopen(MASTER_IND, "r+b");			// "r+b": відкрити бінарний файл
															// для читання та запису	
	if (indexTable == NULL)
	{
		strcpy(error, "database files are not created yet");
		return 0;
	}

	if (!checkIndexExistence(indexTable, error, id))
	{
		return 0;
	}

	struct Master master;
	getMaster(&master, id, error);

	struct Indexer indexer;

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);	// Отримуємо індексатор шуканого запису
	fread(&indexer, INDEXER_SIZE, 1, indexTable);			// за вказаним номером

	indexer.exists = 0;										// Запис логічно не існуватиме...

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);

	fwrite(&indexer, INDEXER_SIZE, 1, indexTable);			// ...але фізично буде присутній
	fclose(indexTable);										// Закриваємо файл [NB: якщо не закрити, значення не оновиться]

	noteDeletedMaster(id);									// Заносимо індекс видаленого запису до "сміттєвої зони"

	
	if (master.slavesCount)								// Були поставки, видаляємо всі
	{
		FILE* slavesDb = fopen(SLAVE_DATA, "r+b");
		struct Slave slave;

		fseek(slavesDb, master.firstSlaveAddress, SEEK_SET);

		for (int i = 0; i < master.slavesCount; i++)
		{
			fread(&slave, SLAVE_SIZE, 1, slavesDb);
			fclose(slavesDb);
			deleteSlave(master, slave, slave.productId, error);
			
			slavesDb = fopen(SLAVE_DATA, "r+b");
			fseek(slavesDb, slave.nextAddress, SEEK_SET);
		}

		fclose(slavesDb);
	}
	return 1;
}