#pragma once
#ifndef __MYSQL_H__
#define __MYSQL_H__

#ifdef _DEBUG
#pragma comment(lib, "../_external/mysql-connector-c++-9.2.0/lib64/debug/mysqlcppconn.lib")
#else //_DEBUG
#pragma comment(lib, "../_external/mysql-connector-c++-9.2.0/lib64/release/mysqlcppconn.lib")
#endif //_DEBUG
#include "../_external/mysql-connector-c++-9.2.0/include/mysql/jdbc.h"

#pragma comment(lib, "../_external/mysql80/lib/libmysql.lib")
#include "../_external/mysql80/include/mysql.h"

#include <string>

using namespace std;

struct MySqlDbInfo
{
    string server{};
    string username = "user_pl";
    string password = "pass_pl";
    string database = "world";
};

class DbConnection
{
private:
    sql::Connection* _conn{nullptr};
    bool _commit{false};
    sql::Statement* _statement{nullptr};
    int preparedStmtIndex{1};
    sql::PreparedStatement* _preparedStatement{nullptr};
    sql::ResultSet* _resultset{nullptr};

public:
    DbConnection(sql::Connection* conn, bool autoCommit = true) : _conn(conn)
    {
        _conn->setAutoCommit(autoCommit);
    }
    ~DbConnection()
    {
        if (_resultset) {
            delete _resultset;
        }
        if (_statement) {
            delete _statement;
        }
        if (_preparedStatement) {
            delete _preparedStatement;
        }
        if (!_conn->getAutoCommit()) {
            if (_commit) {
                _conn->commit();
            } else {
                _conn->rollback();
            }
        }
    }

    void SetCommit(bool commit)
    {
        _commit = commit;
    }

    sql::Statement* GetStatement() 
    { 
        _statement = _conn->createStatement(); 
        return _statement; 
    }

    sql::PreparedStatement* GetStatement(string query) {
        _preparedStatement = _conn->prepareStatement(query);
        return _preparedStatement;
    }

    sql::ResultSet* ExecuteQuery(string query)
    {
        if (!_statement) {
            GetStatement();
        }
        _resultset = _statement->executeQuery(query);
        return _resultset;
    }
    sql::ResultSet* ExecuteQuery()
    {
        if (_preparedStatement) {
            _resultset = _preparedStatement->executeQuery();
        }
        return _resultset;
    }

    void setBigInt(const sql::SQLString& value) { _preparedStatement->setBigInt(preparedStmtIndex++, value); }
    void setBlob(std::istream* blob) { _preparedStatement->setBlob(preparedStmtIndex++, blob); }
    void setBoolean(bool value) { _preparedStatement->setBoolean(preparedStmtIndex++, value); }
    void setDateTime(const sql::SQLString& value) { _preparedStatement->setDateTime(preparedStmtIndex++, value); }
    void setDouble(double value) { _preparedStatement->setDouble(preparedStmtIndex++, value); }
    void setInt(int32_t value) { _preparedStatement->setInt(preparedStmtIndex++, value); }
    void setUInt(uint32_t value) { _preparedStatement->setUInt(preparedStmtIndex++, value); }
    void setInt64(int64_t value) { _preparedStatement->setInt64(preparedStmtIndex++, value); }
    void setUInt64(uint64_t value) { _preparedStatement->setUInt64(preparedStmtIndex++, value); }
    void setNull(int sqlType) { _preparedStatement->setNull(preparedStmtIndex++, sqlType); }
    void setString(const sql::SQLString& value) { _preparedStatement->setString(preparedStmtIndex++, value); }
};

class MySqlCppConn
{
private:
    MySqlDbInfo _dbInfo;
    sql::Connection* _conn{nullptr};

public:
    MySqlCppConn();
    virtual ~MySqlCppConn();

    void SetDbInfo(MySqlDbInfo info);
    sql::Connection* Connect();
    sql::Connection* TryReconnect();
    sql::Connection* GetDbConnection();
    void Disconnect();

    //sql::ResultSet* ExecuteQuery(string query)

    void _test();
    void _querySample();
};

class MySqlApiConn
{
private:
    MySqlDbInfo _dbInfo;
    MYSQL* _conn{nullptr};

public:
    MySqlApiConn();
    virtual ~MySqlApiConn();

    void SetDbInfo(MySqlDbInfo info);
    MYSQL* Connect();
    MYSQL* TryReconnect();
    MYSQL* GetDbConnection();
    void Disconnect();

    void _test();
};

class MySqlDb
{
private:
    MySqlCppConn _db_cpp_conn;
    MySqlApiConn _db_api_conn;

public:
    MySqlDb() {};
    virtual ~MySqlDb() {};
    void _test();
};

#endif //__MYSQL_H__