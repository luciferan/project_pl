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

using namespace std;

class MySqlConnector
{
private:

public:
    MySqlConnector();
    virtual ~MySqlConnector();

    void _test();
    void _test_connector();
    void _test_api();
};

#endif //__MYSQL_H__