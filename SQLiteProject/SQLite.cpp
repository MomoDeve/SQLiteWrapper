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

bool momo::SQLite3::execute(const std::string& SQL, momo::sqlite3_callback function, momo::callback_arg arg)
{
	char* error;
	_success = true;
	if (sqlite3_exec(_database, SQL.c_str(), function, arg, &error))
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

const char* momo::SQLBuilder<momo::OPERATION::CREATE>::convertType(momo::TYPE type)
{
	switch (type)
	{
	case momo::TYPE::INT:
		return "INT";
	case momo::TYPE::TEXT:
		return "TEXT";
	case momo::TYPE::NUMERIC:
		return "NUMERIC";
	case momo::TYPE::REAL:
		return "REAL";
	case momo::TYPE::BLOB:
		return "BLOB";
	}
	return "TEXT";
}

momo::SQLBuilder<momo::OPERATION::CREATE>::SQLBuilder()
	: tableName("UNNAMED")
{

}

momo::SQLBuilder<momo::OPERATION::CREATE>::SQLBuilder(std::string tableName)
	: tableName(std::move(tableName))
{

}

momo::SQLBuilder<momo::OPERATION::CREATE>& momo::SQLBuilder<momo::OPERATION::CREATE>::addColumn(const std::string& name, TYPE type, bool isNull, bool isPrimaryKey)
{
	addColumn(name, convertType(type), isNull, isPrimaryKey);
	return *this;
}

momo::SQLBuilder<momo::OPERATION::CREATE>& momo::SQLBuilder<momo::OPERATION::CREATE>::addColumn(const std::string & name, const std::string & type, bool isNull, bool isPrimaryKey)
{
	const char* IsNULL = (isNull == NULL ? " NULL" : " NOT NULL");
	const char* IsKEY = (isPrimaryKey == momo::PRIMARY_KEY ? " PRIMARY KEY" : "");
	*this << (name + ' ' + type + IsNULL + IsKEY);
	return *this;
}

momo::SQLBuilder<momo::OPERATION::CREATE>& momo::SQLBuilder<momo::OPERATION::CREATE>::operator<<(std::string column)
{
	if(!_columns.empty()) _columns.back() += ',';
	_columns.push_back(std::move(column));
	return *this;
}

momo::SQLBuilder<momo::OPERATION::CREATE>::operator std::string() const
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

momo::SQLBuilder<momo::OPERATION::INSERT>::SQLBuilder(const std::string& tableName, const std::string& values)
	: _insertionLine("INSERT INTO " + tableName + '(' + values + ')')
{

}

momo::SQLBuilder<momo::OPERATION::INSERT>::operator std::string() const
{
	std::stringstream SQL;
	for (const auto& value : _values)
	{
		SQL << _insertionLine;
		SQL << value;
	}
	return SQL.str();
}

momo::SQLBuilder<momo::OPERATION::INSERT>& momo::SQLBuilder<momo::OPERATION::INSERT>::addValues(std::string values)
{
	_values.push_back("VALUES (" + values + ");");
	return *this;
}

momo::SQLBuilder<momo::OPERATION::SELECT>::SQLBuilder(std::string tableName)
	: _tableName(std::move(tableName)), _columns(), callback(nullptr), callbackArg(nullptr)
{

}

momo::SQLBuilder<momo::OPERATION::SELECT>::SQLBuilder(std::string tableName, std::string columns)
	: _tableName(std::move(tableName)), _columns(std::move(columns)), callback(nullptr), callbackArg(nullptr)
{

}

momo::SQLBuilder<momo::OPERATION::SELECT>& momo::SQLBuilder<momo::OPERATION::SELECT>::addColumn(const std::string& columnName)
{
	if (!_columns.empty()) _columns += ',';
	_columns += columnName;
	return *this;
}

momo::SQLBuilder<momo::OPERATION::SELECT>& momo::SQLBuilder<momo::OPERATION::SELECT>::addColumn(const std::string& columnName, const std::string& alias)
{
	if (!_columns.empty()) _columns += ',';
	_columns += columnName + " AS " + alias;
	return *this;
}

momo::SQLBuilder<momo::OPERATION::SELECT>& momo::SQLBuilder<momo::OPERATION::SELECT>::where(const std::string& whereExpression)
{
	if (!_whereExpression.empty()) _whereExpression += "AND";
	_whereExpression = '(' + whereExpression + ')';
	return *this;
}

momo::SQLBuilder<momo::OPERATION::SELECT>& momo::SQLBuilder<momo::OPERATION::SELECT>::orderBy(const std::string& column, momo::ORDER order)
{
	if (!_orderExpression.empty()) _orderExpression += ',';
	_orderExpression += column + (order == ORDER::ASC ? " ASC" : " DESC");
	return *this;
}

momo::SQLBuilder<momo::OPERATION::SELECT>::operator std::string() const
{
	std::stringstream SQL;
	SQL << "SELECT " << (_columns.empty() ? "*" : _columns);
	SQL << " FROM " << _tableName;
	if (!_whereExpression.empty()) SQL << " WHERE " << _whereExpression;
	if (!_orderExpression.empty()) SQL << " ORDER BY " << _orderExpression;
	SQL << ';';
	return SQL.str();
}

momo::SQLite3& momo::operator<<(SQLite3& database, const SQLBuilder<OPERATION::SELECT>& sql)
{
	database.execute(sql, sql.callback, sql.callbackArg);
	return database;
}
