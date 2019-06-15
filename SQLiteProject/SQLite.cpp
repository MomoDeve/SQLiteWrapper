#include "SQLite.h"

bool momo::SQLite3::isThreadSafe()
{
	return sqlite3_threadsafe();
}

momo::SQLite3::SQLite3()
	: _database(nullptr), _success(true), _isOpen(false)
{

}

momo::SQLite3::SQLite3(sqlite3* database, const std::string& name)
	: _name(name), _database(database), _success(true), _isOpen(true)
{

}

momo::SQLite3::SQLite3(const std::string& name)
	: _success(true), _isOpen(true)
{
	open(name);
}

bool momo::SQLite3::isOpen() const
{
	return _isOpen;
}

bool momo::SQLite3::success() const
{
	return _success;
}

const std::string& momo::SQLite3::getErrorMessage() const
{
	return _errorMessage;
}

bool momo::SQLite3::open(const std::string& name)
{
	close();
	_name = name;
	if (sqlite3_open(_name.c_str(), &_database))
	{
		_errorMessage = std::string(sqlite3_errmsg(_database));
		_isOpen = false;
	}
	return _isOpen;
}

bool momo::SQLite3::execute(const std::string& SQL)
{
	return execute(SQL, nullptr, nullptr);
}

bool momo::SQLite3::execute(const std::string& SQL, SQLite3_callback function, void* callbackArg)
{
	char* error;
	_success = true; 
	if (sqlite3_exec(_database, SQL.c_str(), function, callbackArg, &error))
	{
		_errorMessage = std::string(error);
		_success = false;
		sqlite3_free(error);
	}
	return _success;
}

momo::SQLite3& momo::SQLite3::operator<<(const std::string& SQL)
{
	execute(SQL);
	return *this;
}

void momo::SQLite3::close()
{
	if (_isOpen)
	{
		sqlite3_close(_database);
	}
}

momo::SQLite3::~SQLite3()
{
	close();
}

const char* momo::SQLBuilder<momo::Operation::CREATE>::convertType(momo::Type type)
{
	switch (type)
	{
	case momo::INT:
		return "INT";
	case momo::TEXT:
		return "TEXT";
	case momo::NUMERIC:
		return "NUMERIC";
	case momo::REAL:
		return "REAL";
	case momo::BLOB:
		return "BLOB";
	}
	return "TEXT";
}

momo::SQLBuilder<momo::Operation::CREATE>::SQLBuilder()
	: tableName("UNNAMED")
{

}

momo::SQLBuilder<momo::Operation::CREATE>::SQLBuilder(std::string tableName)
	: tableName(std::move(tableName))
{

}

void momo::SQLBuilder<momo::Operation::CREATE>::addColumn(const std::string& name, Type type, bool isNull, bool isPrimaryKey)
{
	const char* IsNULL = (isNull == NULL ? " NULL" : " NOT NULL");
	const char* IsKEY = (isPrimaryKey == momo::PRIMARY_KEY ? " PRIMARY KEY" : "");
	*this << (name + ' ' + convertType(type) + IsNULL + IsKEY);
}

momo::SQLBuilder<momo::Operation::CREATE>& momo::SQLBuilder<momo::Operation::CREATE>::operator<<(std::string column)
{
	if(!_columns.empty()) _columns.back() += ',';
	_columns.push_back(std::move(column));
	return *this;
}

momo::SQLBuilder<momo::Operation::CREATE>::operator std::string() const
{
	std::stringstream SQL;
	SQL << "CREATE TABLE " << tableName << '(';
	for (const auto& column : _columns)
	{
		SQL << column;
	}
	SQL << ");";
	return SQL.str();
}

momo::SQLBuilder<momo::Operation::INSERT>::SQLBuilder(const std::string& tableName, const std::string& values)
	: _insertionLine("INSERT INTO " + tableName + '(' + values + ')')
{

}

momo::SQLBuilder<momo::Operation::INSERT>::operator std::string() const
{
	std::stringstream SQL;
	for (const auto& value : _values)
	{
		SQL << _insertionLine;
		SQL << value;
	}
	return SQL.str();
}

void momo::SQLBuilder<momo::Operation::INSERT>::addValues(std::string values)
{
	_values.push_back("VALUES (" + values + ");");
}
