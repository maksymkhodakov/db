#include <stdio.h>
#include "Structures.h"
#include "Master.h"
#include "Slave.h"
#include "Input.h"
#include "Output.h"


int main()
{
	struct Master master;
	struct Slave slave;

	while (1)
	{  
		int choice;
		int id;
		char error[51];

		printf("Choose option:\n0 - Quit\n1 - Insert Master\n2 - Get Master\n3 - Update Master\n4 - Delete Master\n5 - Insert Slave\n6 - Get Slave\n7 - Update Slave\n8 - Delete Slave\n9 - Info\n");
		scanf("%d", &choice);

		switch (choice)
		{
			case 0:
				return 0;

			case 1:
				readMaster(&master);
				insertMaster(master);
				break;
				
			case 2:
				printf("Enter ID: ");
				scanf("%d", &id);
				getMaster(&master, id, error) ? printMaster(master) : printf("Error: %s\n", error);
				break;

			case 3:
				printf("Enter ID: ");
				scanf("%d", &id);

				master.id = id;
				
				readMaster(&master);
				updateMaster(master, error) ? printf("Updated successfully\n") : printf("Error: %s\n", error);
				break;

			case 4:
				printf("Enter ID: ");
				scanf("%d", &id);
				deleteMaster(id, error) ? printf("Deleted successfully\n") : printf("Error: %s\n", error);
				break;

			case 5:
				printf("Enter master\'s ID: ");
				scanf("%d", &id);

				if (getMaster(&master, id, error))
				{
					slave.masterId = id;
					printf("Enter product ID: ");
					scanf("%d", &id);

					if (checkKeyPairUniqueness(master, id))		// Унікальність по коду деталі
					{
						slave.productId = id;
						readSlave(&slave);
						insertSlave(master, slave, error);
						printf("Inserted successfully. To access, use master\'s and product\'s IDs\n");
					}
					else
					{
						printf("Error: non-unique product key\n");
					}
				}
				else
				{
					printf("Error: %s\n", error);
				}
				break;

			case 6:
				printf("Enter master\'s ID: ");
				scanf("%d", &id);

				if (getMaster(&master, id, error))
				{
					printf("Enter product ID: ");
					scanf("%d", &id);
					getSlave(master, &slave, id, error) ? printSlave(slave, master) : printf("Error: %s\n", error);
				}
				else
				{
					printf("Error: %s\n", error);
				}
				break;

			case 7:
				printf("Enter master\'s ID: ");
				scanf("%d", &id);

				if (getMaster(&master, id, error))
				{
					printf("Enter product ID: ");
					scanf("%d", &id);
					
					if (getSlave(master, &slave, id, error))
					{
						readSlave(&slave);
						updateSlave(slave, id);
						printf("Updated successfully\n");
					}
					else
					{
						printf("Error: %s\n", error);
					}
				}
				else
				{
					printf("Error: %s\n", error);
				}
				break;

			case 8:
				printf("Enter master\'s ID: ");
				scanf("%d", &id);

				if (getMaster(&master, id, error))
				{
					printf("Enter product ID: ");
					scanf("%d", &id);

					if (getSlave(master, &slave, id, error))
					{
						deleteSlave(master, slave, id, error);
						printf("Deleted successfully\n");
					}
					else
					{
						printf("Error: %s\n", error);
					}
				}
				else
				{
					printf("Error: %s\n", error);
				}
				break;

			case 9:
				info();
				break;

			default:
				printf("Invalid input, please try again\n");
		}

		printf("---------\n");
	}
}