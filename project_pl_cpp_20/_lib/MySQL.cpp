#include "./MySQL.h"

#include <iostream>
#include <string>

//-------------------------------------------------------------------------------
MySqlCppConn::MySqlCppConn()
{
}

MySqlCppConn::~MySqlCppConn()
{
    Disconnect();
}

void MySqlCppConn::SetDbInfo(MySqlDbInfo info)
{
    _dbInfo = std::move(info);
}

sql::Connection* MySqlCppConn::Connect()
{
    try {
        if (!_conn) {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
            _conn = driver->connect(_dbInfo.server, _dbInfo.username, _dbInfo.password);
            _conn->setSchema(_dbInfo.database);
            cout << "Connect to MySQL database successfully" << endl;
        } else if (!_conn->isValid()) {
            if (!_conn->reconnect()) {
                cerr << "Error reconnecting fail" << endl;
                cerr << "try connect()" << endl;

                _conn = TryReconnect();
            }
        }
    }
    catch (sql::SQLException& e) {
        cerr << "Error connecting to MySQL: " << e.what() << endl;
    }
    catch (exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }

    return _conn;
}

sql::Connection* MySqlCppConn::TryReconnect()
{
    for (int cnt = 0; cnt < 5; ++cnt) {
        cout << "MySQL reconnect try " << cnt + 1 << endl;

        if (_conn && !_conn->isValid()) {
            if (_conn->reconnect()) {
                cout << "Reconnect to MySQL" << endl;
                return _conn;
            }
        }
    }

    cout << "MySQL reconnect fail" << endl;
    Disconnect();

    return nullptr;
}

sql::Connection* MySqlCppConn::GetDbConnection()
{
    return Connect();
}

void MySqlCppConn::Disconnect()
{
    if (_conn) {
        _conn->close();
        delete _conn;
        _conn = nullptr;
    }
}

//sql::ResultSet* MySqlCppConn::ExecuteQuery(string query)
//{
//    sql::ResultSet* res{nullptr};
//
//    try {
//        sql::Connection* conn = GetDbConnection();
//        unique_ptr<sql::Statement> stmt(conn->createStatement());
//        res = stmt->executeQuery(query);
//    }
//    catch (sql::SQLException& e) {
//        cerr << "Error connecting yo MySQL: " << e.what() << endl;
//    }
//    catch (exception& e) {
//        cerr << "Exception: " << e.what() << endl;
//    }
//
//    return res;
//}

void MySqlCppConn::_test()
{
    string server = "tcp://192.168.35.227:3306";
    string username = "user_pl";
    string password = "pass_pl";
    string database = "world";

    //try {
    //    sql::mysql::MySQL_Driver* driver;
    //    sql::Connection* conn;

    //    driver = sql::mysql::get_driver_instance();
    //    conn = driver->connect(server, username, password);

    //    conn->setSchema(database);
    //    cout << "Connect to MySQL database successfully" << endl;

    //    string query = "SELECT count(*) FROM city";
    //    cout << "query: " << query << endl;
    //    unique_ptr<sql::Statement> stmt(conn->createStatement());
    //    unique_ptr<sql::ResultSet> res(stmt->executeQuery(query));

    //    while (res && res->next()) {
    //        cout << res->getInt(1) << endl;
    //    }
    //    cout << "query result end" << endl;

    //    delete conn;
    //}
    //catch (sql::SQLException& e) {
    //    cerr << "Error connecting yo MySQL: " << e.what() << endl;
    //}
    //catch (exception &e ) {
    //    cerr << "Exception: " << e.what() << endl;
    //}

    SetDbInfo({server, username, password, database});
    Connect();
    _querySample();
}

void MySqlCppConn::_querySample()
{
    try {
        string query = "SELECT count(*) FROM city";
        cout << "query: " << query << endl;

        //sql::Connection* conn = GetDbConnection();
        //unique_ptr<sql::Statement> stmt(conn->createStatement());
        //unique_ptr<sql::ResultSet> res(stmt->executeQuery(query));

        DbConnection dbCon(GetDbConnection(), true);
        sql::ResultSet* res = dbCon.ExecuteQuery(query);
        while (res && res->next()) {
            cout << res->getInt(1) << endl;
        }

        //conn->close();
    }
    catch (sql::SQLException& e) {
        cerr << "Error connecting yo MySQL: " << e.what() << endl;
        cerr << "SQLState: " << e.getSQLState() << endl;
    }
    catch (exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }

    try {
        string query = "SELECT name, Population FROM city WHERE CountryCode = ?";
        cout << "query: " << query << endl;

        //sql::Connection* conn = GetDbConnection();
        //unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(query));
        //stmt->setString(1, "kor");
        //unique_ptr<sql::ResultSet> res(stmt->executeQuery());

        DbConnection dbConn(GetDbConnection(), true);
        dbConn.GetStatement(query);
        dbConn.setString("kor");
        sql::ResultSet* res = dbConn.ExecuteQuery();
        while (res && res->next()) {
            cout << res->getString(1) << " " << res->getInt(2) << endl;
        }

        //conn->close();
    }
    catch (sql::SQLException& e) {
        cerr << "Error connecting yo MySQL: " << e.what() << endl;
        cerr << "SQLState: " << e.getSQLState() << endl;
    }
    catch (exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }
}

//-------------------------------------------------------------------------------
MySqlApiConn::MySqlApiConn()
{
}

MySqlApiConn::~MySqlApiConn()
{
    Disconnect();
}

void MySqlApiConn::SetDbInfo(MySqlDbInfo info)
{
    _dbInfo = std::move(info);
}

MYSQL* MySqlApiConn::Connect()
{
    MYSQL conn;
    if (mysql_init(&conn) == NULL) {
        cout << "mysql_init fail" << endl;
    }

    MYSQL* connection = mysql_real_connect(&conn, _dbInfo.server.c_str(), _dbInfo.username.c_str(), _dbInfo.password.c_str(), _dbInfo.database.c_str(), 3306, (const char*)NULL, 0);
    if (connection == NULL) {
        cout << "mysql_real_connect fail" << mysql_error(&conn) << endl;
    } else {
        cout << "mysql connect success" << endl;

        if (NULL == mysql_select_db(&conn, _dbInfo.database.c_str())) {
            _conn = connection;
        } else {
            cout << "mysql database select fail" << endl;
            mysql_close(connection);
        }
    }

    return _conn;
}

MYSQL* MySqlApiConn::TryReconnect()
{
    if (_conn) {
        mysql_close(_conn);
        delete _conn;
        _conn = nullptr;
    }

    for (int cnt = 0; cnt < 5; ++cnt) {
        cout << "MySQL connect try " << cnt + 1 << endl;

        _conn = Connect();
        if (_conn && mysql_ping(_conn)) {
            cout << "mysql connect success" << endl;
            return _conn;
        }
    }

    cout << "mysql connect fail" << endl;
    Disconnect();

    return nullptr;
}

MYSQL* MySqlApiConn::GetDbConnection()
{
    if (_conn) {
        if (mysql_ping(_conn)) {
            _conn = TryReconnect();
        }
    } else {
        _conn = Connect();
    }

    return _conn;
}

void MySqlApiConn::Disconnect()
{
    if (_conn) {
        mysql_close(_conn);
        delete _conn;
        _conn = nullptr;
    }
}

void MySqlApiConn::_test()
{
    string server = "192.168.35.227";
    string username = "user_pl";
    string password = "pass_pl";
    string database = "world";

    MYSQL conn;
    if (mysql_init(&conn) == NULL) {
        cout << "mysql_init fail" << endl;
    }

    MYSQL* connection = mysql_real_connect(&conn, server.c_str(), username.c_str(), password.c_str(), database.c_str(), 3306, (const char*)NULL, 0);
    if (connection == NULL) {
        cout << "mysql_real_connect fail: " << mysql_error(&conn) << endl;
    } else {
        cout << "mysql connect success" << endl;

        if (mysql_select_db(&conn, database.c_str()) == NULL) {
            string query = "SELECT count(*) FROM city";
            cout << "query: " << query << endl;

            if (0 == mysql_real_query(connection, query.c_str(), query.length())) {
                MYSQL_RES* sql_result = mysql_store_result(connection);
                MYSQL_ROW sql_row;
                while ((sql_row = mysql_fetch_row(sql_result)) != NULL) {
                    cout << sql_row[0] << endl;
                }
                mysql_free_result(sql_result);
            }
            cout << "query result end" << endl;
        }

        mysql_close(connection);
    }
}

//-------------------------------------------------------------------------------
void MySqlDb::_test()
{
    cout << "=== test use mysql connector" << endl;
    _db_cpp_conn._test();
    cout << "=== test use mysql api" << endl;
    _db_api_conn._test();
}