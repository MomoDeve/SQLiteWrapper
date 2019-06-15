#include <iostream>
#include "SQLite.h"

static int callback(void *data, int argc, char **argv, char **azColName)
{
	int i;
	fprintf(stderr, "%s: ", (const char*)data);

	for (i = 0; i < argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}

	printf("\n");
	return 0;
}

using namespace momo;

int main() 
{
	SQLite3 database;
	database.open("sampleSQLiteDB.dblite");

	SQLBuilder<Operation::CREATE> sqlCreate("COMPANY");
	sqlCreate.addColumn("ID", INT, NOT_NULL, PRIMARY_KEY);
	sqlCreate.addColumn("NAME", TEXT, NOT_NULL);
	sqlCreate.addColumn("AGE", INT, NOT_NULL);
	sqlCreate.addColumn("ADDRESS", TEXT);
	sqlCreate.addColumn("SALARY", REAL);

	database << sqlCreate;
	if (!database.success())
	{
		std::cout << "error while creating table: " << database.getErrorMessage() << std::endl;
	}

	SQLBuilder<Operation::INSERT> sqlInsert(sqlCreate.tableName, "ID, NAME, AGE, ADDRESS, SALARY");
	sqlInsert.addValues(pack(2, "Allen", 25, "Texas", 15000.00));
	sqlInsert.addValues(pack(3, "Teddy", 23, "Norway", 20000.0));
	sqlInsert.addValues(pack(4, "Mark", 25, "Rich-Mond", 65000.00));
	sqlInsert.addValues(pack(1, "Paul", 32, "Califonia", 20000.0));
	sqlInsert.addValues(pack(5, "Alex", 16, "Moscow", 10000.00));

	database << sqlInsert;
	if (!database.success())
	{
		std::cout << database.getErrorMessage() << std::endl;
	}

	const char* sqlSelect = "SELECT * from COMPANY";
	if (!database.execute(sqlSelect, callback, 0))
	{
		std::cout << database.getErrorMessage() << std::endl;
	}

	database.close();
	system("pause");
	return 0;
}