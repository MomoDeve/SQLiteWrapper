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
	 
	SQLBuilder<OPERATION::CREATE> sqlCreate("COMPANY");
	sqlCreate
		.addColumn("ID", INT, NOT_NULL, PRIMARY_KEY)
		.addColumn("NAME", TEXT, NOT_NULL)
		.addColumn("AGE", INT, NOT_NULL)
		.addColumn("ADDRESS", "VARCHAR(10)")
		.addColumn("SALARY", REAL);

	database << sqlCreate;
	if (!database.success())
	{
		std::cout << "error while creating table: " << database.getErrorMessage() << std::endl;
	}

	SQLBuilder<OPERATION::INSERT> sqlInsert(sqlCreate.tableName, "ID, NAME, AGE, ADDRESS, SALARY");
	sqlInsert
		.addValues(pack(0, "Allen", 25, "Texas", 15000.00))
		.addValues(pack(1, "Teddy", 23, "Norway", 20000.0))
		.addValues(pack(2, "Mark", 25, "Rich-Mond", 65000.00))
		.addValues(pack(3, "Paul", 32, "Califonia", 20000.0))
		.addValues(pack(4, "Alex", 16, "Moscow", 10000.00))
		.addValues(pack(5, "Harry", 25, "Boston", 40000));

	database << sqlInsert;
	if (!database.success())
	{
		std::cout << database.getErrorMessage() << std::endl;
	}
	SQLBuilder<OPERATION::SELECT> select(sqlCreate.tableName);

	//sqlCreate.tableName = "MY_DB";
	SQLBuilder<OPERATION::ALTER> sqlAlter(sqlCreate.tableName);
	sqlAlter
		.renameColumn("SALARY", "MONTH_PAYOFF")
		.renameColumn("ADDRESS", "HOME");

	database << sqlAlter;
	if (!database.success())
	{
		std::cout << database.getErrorMessage() << std::endl;
	}

	SQLBuilder<OPERATION::DELETE> sqlDelete(sqlCreate.tableName);
	sqlDelete.where("ID < 2").where("MONTH_PAYOFF < 20000");
	database << sqlDelete;

	SQLBuilder<OPERATION::SELECT> sqlSelect(sqlCreate.tableName);
	sqlSelect.callback = callback;

	database << sqlSelect;
	if (!database.success())
	{
		std::cout << database.getErrorMessage() << std::endl;
	}
	SQLBuilder<OPERATION::DROP> sqlDrop(sqlCreate.tableName);
	database.execute(sqlDrop);
	database.close();
	system("pause");
	return 0;
}