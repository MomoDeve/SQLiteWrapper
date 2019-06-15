#pragma once

#include "sqlite3.h"
#include <string>
#include <vector>
#include <sstream>
#include <typeinfo>

namespace momo
{
	typedef int(*SQLite3_callback)(void*, int, char**, char**);

	class SQLite3
	{
		std::string _name;
		std::string _errorMessage;
		bool _success;
		sqlite3* _database;
		bool _isOpen;
	public:
		/*
		returns true if sqlite runs is threadsafe mode, false either
		*/
		static bool isThreadSafe();

		/*
		creating an empty object with no database
		*/
		SQLite3();

		/*
		creating an object using already existing database with name provided
		*/
		SQLite3(sqlite3* _database, const std::string& name);

		/* 
		creating a new database using name provided
	    example: SQLite3 db("myDB.dblite");
		*/
		SQLite3(const std::string& name);

		/*
		returns true if database is opened, false either
		*/
		bool isOpen() const;


		/*
		returns true if last operation was successful (no errors), false either
		*/
		bool success() const;

		/*
		returns last error message accured in the database
		*/
		const std::string& getErrorMessage() const;

		/* 
		open/create new db using name provided
		if another db was already opened, it will be closed before
		returns true on success, false on failure
		*/
		bool open(const std::string& name);

		/*
		execute an SQL command (as string)
		if an error accurs, it can be got using getErrorMessage() method
		returns true on success, false on failure
		*/
		bool execute(const std::string& SQL);

		/*
		execute an SQL command (as string) with callback function
		if an error accurs, it can be got using getErrorMessage() method
		returns true on success, false on failure
		*/
		bool execute(const std::string& SQL, SQLite3_callback function, void* callbackArg);

		/*
		execute an SQL command (as string) passed usign << operator
		if an error accurs, it can be got using getErrorMessage() method
		*/
		SQLite3& operator<<(const std::string& SQL);

		/*
		closes db if it was opened. Automatically called in the destructor
		*/
		void close();

		/*
		automatically calling close() method at the end of object lifetime
		*/
		~SQLite3();
	};

	/*
	enum of all operations which can be passes as template parameter to SQLBuilder<>
	*/
	enum Operation
	{
		SELECT,
		INSERT,
		CREATE
	};

	/*
	enum of all types which can be passed to addColumn function in SQLBuilder<CREATE> class
	*/
	enum Type
	{
		INT,
		TEXT,
		NUMERIC,
		REAL,
		BLOB
	};

	/*
	enum of predefined Booleans which can be passed to addColumn function in SQLBuilder<CREATE> class
	*/
	enum Boolean
	{
		IS_NULL = 0,
		NOT_NULL = 1,
		PRIMARY_KEY = 1,
		NOT_PRIMARY_KEY = 0
	};

	/*
	unspecialized template of SQLBuilder
	*/
	template<Operation op>
	class SQLBuilder { };
	
	/*
	SQLBuilder class for creating tables in the database
	*/
	template<>
	class SQLBuilder<Operation::CREATE>
	{
		static const char* convertType(Type type);

		std::vector<std::string> _columns;
	public:
		std::string tableName;

		/*
		creates object with _table = "UNNAMED"
		_table can be set later by using SQLBuilder.table = "ANY NAME";
		*/
		SQLBuilder();

		/*
		table will be created with name passed into constructor
		*/
		SQLBuilder(std::string tableName);

		/*
		adds column to the table

		example: 
		addColumn("NAME", Type::TEXT, NOT_NULL, PRIMARY_KEY) produces
				  "NAME TEXT NOT NULL PRIMARY KEY"
		*/
		void addColumn(const std::string& name, Type type, bool isNull = IS_NULL, bool isPrimaryKey = NOT_PRIMARY_KEY);

		/*
		adds column to the table using default SQL

		example:
		SQLBuilder << "NAME TEXT NOT NULL PRIMARY KEY";
		*/
		SQLBuilder<Operation::CREATE>& operator<<(std::string column);

		/*
		converts SQLBuilder object to SQL 
		can be passed to execute method of database: execute(sqlBuilder)
		*/
		operator std::string() const;
	};

	/*
	SQLBuilder class for inserting values in the database
	*/
	template<>
	class SQLBuilder<Operation::INSERT>
	{
		std::string _insertionLine;
		std::vector<std::string> _values;
	public:
		/*
		values will be inserted into table with name passed into constructor
		VALUES(...) are set using values parameter
		
		example:
		SQLBuilder builer("MYTABLE", "ID, NAME, AGE");
		will produce line: INSERT INTO MYTABLE (ID, NAME, AGE)
		*/
		SQLBuilder(const std::string& tableName, const std::string& values);

		/*
		adds values to the SQL INSERT request

		example:
		addValues("122, 'ALEX', 23");
		will produce line: VALUES (122, 'ALEX', 23)

		hint: use pack(args...) to create a single line from multiple parameters
		*/
		void addValues(std::string values);

		/*
		converts SQLBuilder object to SQL
		can be passed to execute method of database: execute(sqlBuilder)
		*/
		operator std::string() const;
	};

	/*
	template specialization for 1 argument
	for more info, see pack(args...) declaration
	*/
	template<typename Last>
	std::string pack(Last arg)
	{
		std::stringstream out;
		auto& type = typeid(arg);
		if (type == typeid(const char*) || type == typeid(char*) || type == typeid(std::string))
		{
			out << "'" << arg << "'";
		}
		else
		{
			out << arg;
		}
		return out.str();
	}

	/*
	creates a single line of values from parameters and can be passed into SQLBuilder methods

	example:
	pack(122, "ALEX", 23) will return "122, 'ALEX', 23"
	*/
	template<typename First, typename... Other>
	std::string pack(First arg, Other... args)
	{
		std::stringstream out;
		auto& type = typeid(arg);
		if (type == typeid(const char*) || type == typeid(char*) || type == typeid(std::string))
		{
			out << "'" << arg << "', ";
		}
		else
		{
			out << arg << ", ";
		}
		out << pack(args...);
		return out.str();
	}
}