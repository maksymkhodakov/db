#pragma once
#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include "Structures.h"
#include "Checks.h"
#include "Master.h"

#define SLAVE_DATA "slave.fl"
#define SLAVE_GARBAGE "slave_garbage.txt"
#define SLAVE_SIZE sizeof(struct Slave)

/*
Functions performing all the needed operations on the slaves.
*/

int updateMaster(struct Master master, char* error);

void reopenDatabase(FILE* database)
{
	fclose(database);
	database = fopen(SLAVE_DATA, "r+b");
}

void linkAddresses(FILE* database, struct Master master, struct Slave slave)
{
	reopenDatabase(database);								// Змінюємо режим на "читання з та запис у будь-яке місце"

	struct Slave previous;

	fseek(database, master.firstSlaveAddress, SEEK_SET);

	for (int i = 0; i < master.slavesCount; i++)		    // Пробігаємомо зв'язаний список до останньої поставки
	{
		fread(&previous, SLAVE_SIZE, 1, database);
		fseek(database, previous.nextAddress, SEEK_SET);
	}

	previous.nextAddress = slave.selfAddress;				// Зв'язуємо адреси
	fwrite(&previous, SLAVE_SIZE, 1, database);				// Заносимо оновлений запис назад до файлу
}

void relinkAddresses(FILE* database, struct Slave previous, struct Slave slave, struct Master* master)
{
	if (slave.selfAddress == master->firstSlaveAddress)		// Немає попередника (перший запис)...
	{
		if (slave.selfAddress == slave.nextAddress)			// ...ще й немає наступника (запис лише один)
		{
			master->firstSlaveAddress = -1;					// Неможлива адреса для безпеки
		}
		else                                                // ...а наступник є,
		{
			master->firstSlaveAddress = slave.nextAddress;  // робимо його першим
		}
	}
	else                                                    // Попередник є...
	{
		if (slave.selfAddress == slave.nextAddress)			// ...але немає наступника (останній запис)
		{
			previous.nextAddress = previous.selfAddress;    // Робимо попередник останнім
		}
		else                                                // ... а разом з ним і наступник
		{
			previous.nextAddress = slave.nextAddress;		// Робимо наступник наступником попередника
		}

		fseek(database, previous.selfAddress, SEEK_SET);	// Записуємо оновлений попередник
		fwrite(&previous, SLAVE_SIZE, 1, database);			// назад до таблички
	}
}

void noteDeletedSlave(long address)
{
	FILE* garbageZone = fopen(SLAVE_GARBAGE, "rb");			// "rb": відкриваємо бінарний файл для читання

	int garbageCount;
	fscanf(garbageZone, "%d", &garbageCount);

	long* delAddresses = malloc(garbageCount * sizeof(long)); // Виділяємо місце під список "сміттєвих" адрес

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageZone, "%ld", delAddresses + i);		// Заповнюємо його
	}

	fclose(garbageZone);									// За допомогою цих двох команд
	garbageZone = fopen(SLAVE_GARBAGE, "wb");				// повністю очищуємо файл зі "сміттям"
	fprintf(garbageZone, "%d", garbageCount + 1);			// Записуємо нову кількість "сміттєвих" адрес

	for (int i = 0; i < garbageCount; i++)
	{
		fprintf(garbageZone, " %ld", delAddresses[i]);		// Заносимо "сміттєві" адреси назад...
	}

	fprintf(garbageZone, " %ld", address);					// ...і дописуємо до них адресу щойно видаленого запису
	free(delAddresses);										// Звільняємо виділену під масив пам'ять
	fclose(garbageZone);									// Закриваємо файл
}

void overwriteGarbageAddress(int garbageCount, FILE* garbageZone, struct Slave* record)
{
	long* delIds = malloc(garbageCount * sizeof(long));		// Виділяємо місце під список "сміттєвих" адрес

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageZone, "%ld", delIds + i);				// Заповнюємо його
	}

	record->selfAddress = delIds[0];						// Для запису замість логічно видаленої "сміттєвої"
	record->nextAddress = delIds[0];

	fclose(garbageZone);									// За допомогою цих двох команд
	fopen(SLAVE_GARBAGE, "wb");							    // повністю очищуємо файл зі "сміттям"
	fprintf(garbageZone, "%d", garbageCount - 1);			// Записуємо нову кількість "сміттєвих" адрес

	for (int i = 1; i < garbageCount; i++)
	{
		fprintf(garbageZone, " %ld", delIds[i]);				// Записуємо решту "сміттєвих" адрес
	}

	free(delIds);											// Звільняємо виділену під масив пам'ять
	fclose(garbageZone);									// Закриваємо файл
}

int insertSlave(struct Master master, struct Slave slave, char* error)
{
	slave.exists = 1;

	FILE* database = fopen(SLAVE_DATA, "a+b");
	FILE* garbageZone = fopen(SLAVE_GARBAGE, "rb");

	int garbageCount;											

	fscanf(garbageZone, "%d", &garbageCount);

	if (garbageCount)											// Наявні видалені записи
	{
		overwriteGarbageAddress(garbageCount, garbageZone, &slave);
		reopenDatabase(database);								// Змінюємо режим доступу файлу
		fseek(database, slave.selfAddress, SEEK_SET);			// Ставимо курсор на "сміття" для подальшого перезапису
	}
	else                                                        // Видалених немає, пишемо в кінець файлу
	{
		fseek(database, 0, SEEK_END);

		int dbSize = ftell(database);

		slave.selfAddress = dbSize;	
		slave.nextAddress = dbSize;
	}

	fwrite(&slave, SLAVE_SIZE, 1, database);					// Записуємо поставку до свого файлу

	if (!master.slavesCount)								    // Поставок ще немає, пишемо адресу першої
	{
		master.firstSlaveAddress = slave.selfAddress;
	}
	else                                                        // Поставки вже є, оновлюємо "адресу наступника" останньої
	{
		linkAddresses(database, master, slave);
	}

	fclose(database);											// Закриваємо файл

	master.slavesCount++;										// Стало на одну поставку більше
	updateMaster(master, error);								// Оновлюємо запис постачальника

	return 1;
}

int getSlave(struct Master master, struct Slave* slave, int productId, char* error)
{
	if (!master.slavesCount)									// У постачальника немає поставок
	{
		strcpy(error, "This master has no slaves yet");
		return 0;
	}

	FILE* database = fopen(SLAVE_DATA, "rb");


	fseek(database, master.firstSlaveAddress, SEEK_SET);		// Отримуємо перший запис
	fread(slave, SLAVE_SIZE, 1, database);

	for (int i = 0; i < master.slavesCount; i++)				// Шукаємо потрібний запис по коду деталі
	{
		if (slave->productId == productId)						// Знайшли
		{
			fclose(database);
			return 1;
		}

		fseek(database, slave->nextAddress, SEEK_SET);
		fread(slave, SLAVE_SIZE, 1, database);
	}
	
	strcpy(error, "No such slave in database");					// Не знайшли
	fclose(database);
	return 0;
}

// На вхід подається поставка з оновленими значеннями, яку треба записати у файл
int updateSlave(struct Slave slave, int productId)
{
	FILE* database = fopen(SLAVE_DATA, "r+b");

	fseek(database, slave.selfAddress, SEEK_SET);
	fwrite(&slave, SLAVE_SIZE, 1, database);
	fclose(database);
	
	return 1;
}

void deleteSlave(struct Master master, struct Slave slave, int productId, char* error)
{
	FILE* database = fopen(SLAVE_DATA, "r+b");
	struct Slave previous;

	fseek(database, master.firstSlaveAddress, SEEK_SET);

	do		                                                    // Шукаємо попередника запису (його може й не бути,
	{															// тоді в попередника занесеться сам запис)
		fread(&previous, SLAVE_SIZE, 1, database);
		fseek(database, previous.nextAddress, SEEK_SET);
	}
	while (previous.nextAddress != slave.selfAddress && slave.selfAddress != master.firstSlaveAddress);

	relinkAddresses(database, previous, slave, &master);
	noteDeletedSlave(slave.selfAddress);						// Заносимо адресу видаленого запису у "смітник"

	slave.exists = 0;											// Логічно не існуватиме

	fseek(database, slave.selfAddress, SEEK_SET);				// ...але фізично
	fwrite(&slave, SLAVE_SIZE, 1, database);					// записуємо назад
	fclose(database);

	master.slavesCount--;										// Однією поставкою менше
	updateMaster(master, error);

}