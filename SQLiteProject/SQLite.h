#pragma once

#include "sqlite3.h"
#include <string>
#include <vector>
#include <sstream>
#include <typeinfo>
#include <ostream>

namespace momo
{
	typedef int(*sqlite3_callback)(void*, int, char**, char**);
	typedef void* callback_arg;

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
		bool execute(const std::string& SQL, sqlite3_callback function, callback_arg arg);

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
	enum OPERATION
	{
		SELECT,
		INSERT,
		CREATE,
		DROP
	};

	/*
	enum of all types which can be passed to addColumn function in SQLBuilder<CREATE> class
	*/
	enum TYPE
	{
		INT,
		TEXT,
		NUMERIC,
		REAL,
		BLOB,
	};

	/*
	enum of predefined Booleans which can be passed to addColumn function in SQLBuilder<CREATE> class
	*/
	enum Boolean
	{
		IS_NULL = 0,
		NOT_NULL = 1,
		PRIMARY_KEY = 1,
		NOT_PRIMARY_KEY = 0,
	};

	/*
	enum of order constants which can be passed to order() function
	*/
	enum ORDER
	{
		ASC,
		DESC
	};

	/*
	unspecialized template of SQLBuilder
	*/
	template<OPERATION op>
	class SQLBuilder { };
	
	/*
	SQLBuilder class for creating tables in the database
	*/
	template<>
	class SQLBuilder<OPERATION::CREATE>
	{
		static const char* convertType(TYPE type);

		std::vector<std::string> _columns;
	public:
		/*
		name of table in the database
		*/
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
		SQLBuilder<OPERATION::CREATE>& addColumn(const std::string& name, TYPE type, bool isNull = IS_NULL, bool isPrimaryKey = NOT_PRIMARY_KEY);

		/*
		adds column to the table

		example:
		addColumn("NAME", "TEXT", NOT_NULL, PRIMARY_KEY) produces
				  "NAME TEXT NOT NULL PRIMARY KEY"
		*/
		SQLBuilder<OPERATION::CREATE>& addColumn(const std::string& name, const std::string& type, bool isNull = IS_NULL, bool isPrimaryKey = NOT_PRIMARY_KEY);

		/*
		adds column to the table using default SQL

		example:
		SQLBuilder << "NAME TEXT NOT NULL PRIMARY KEY";
		*/
		SQLBuilder<OPERATION::CREATE>& operator<<(std::string column);

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
	class SQLBuilder<OPERATION::INSERT>
	{
		std::string _insertionLine;
		std::vector<std::string> _values;
	public:
		/*
		values will be inserted into table with name passed into constructor
		VALUES(...) are set using `values` variable
		
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
		SQLBuilder<OPERATION::INSERT>& addValues(std::string values);

		/*
		converts SQLBuilder object to SQL
		can be passed to execute method of database: execute(sqlBuilder)
		*/
		operator std::string() const;
	};

	/*
	SQLBuilder class for selecting values from the database
	*/
	template<>
	class SQLBuilder<OPERATION::SELECT>
	{
		std::string _columns;
		std::string _tableName;
		std::string _whereExpression;
		std::string _orderExpression;
		std::string _havingExpression;
	public:
		/*
		callback function which will be called after select execution
		*/
		momo::sqlite3_callback callback;

		/*
		callback arg for callback function (see callback class member)
		*/
		momo::callback_arg callbackArg;

		/*
		initialize SQLBuilder with table name
		by default, instance will select * from database
		*/
		SQLBuilder(std::string tableName);
		/*
		initalize SQLBuilder with table name and columns
		to select all columns (*), use SQLBuilder(tableName) constructor
		to set alias for columns, use addColumn(columnName, alias) method
		*/
		SQLBuilder(std::string tableName, std::string columns);

		/*
		adds column to select statement
		for alias use addColumn(columnName, alias) instead
		*/
		SQLBuilder<OPERATION::SELECT>& addColumn(const std::string& columnName);

		/*
		adds column to select statement as alias provided
		*/
		SQLBuilder<OPERATION::SELECT>& addColumn(const std::string& columnName, const std::string& alias);

	    /*
		adds WHERE expression to the select statement. 
		This method can be called multiple times and expressions will be concatenated with `AND`
		example: sqlBuilder.where("ID < 1000").where("NAME = 'Alex'");
		will produce: WHERE (ID < 1000) AND (NAME = 'Alex')
		*/
		SQLBuilder<OPERATION::SELECT>& where(const std::string& whereExpression);

		/*
		add ORDER BY expression to select statement. 
		This method can be called multiple times to get multiple-row order
		example: sqlBuilder.orderBy("NAME", ORDER::ASC).orderBy("AGE", ORDER::DESC);
		will produce: ORDER BY NAME ASC, AGE DESC
		*/
		momo::SQLBuilder<momo::OPERATION::SELECT>& orderBy(const std::string& column, momo::ORDER order = ORDER::ASC);

		/*
		converts SQLBuilder object to SQL
		can be passed to execute method of database: execute(sqlBuilder)
		*/
		operator std::string() const;
	};

	/*
	SQLBuilder class for droping tables
	*/
	template<>
	class SQLBuilder<OPERATION::DROP>
	{
		std::string _tableName;
	public:
		SQLBuilder(std::string tableName)
			: _tableName(std::move(tableName))
		{
			
		}
	};

	SQLite3& operator<<(SQLite3& database, const SQLBuilder<OPERATION::SELECT>& sql);

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