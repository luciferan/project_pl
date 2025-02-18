#include "./MySQL.h"

#include <iostream>
#include <string>

MySqlConnector::MySqlConnector()
{
}

MySqlConnector::~MySqlConnector()
{
}

void MySqlConnector::_test()
{
    cout << "=== test use mysql connector" << endl;
    _test_connector();
    cout << "=== test use mysql api" << endl;
    _test_api();
}

void MySqlConnector::_test_connector()
{
    string server = "tcp://localhost:3306";
    string username = "db_user";
    string password = "db_pass";
    string database = "world";

    try {
        sql::mysql::MySQL_Driver* driver;
        sql::Connection* conn;

        driver = sql::mysql::get_driver_instance();
        conn = driver->connect(server, username, password);

        conn->setSchema(database);
        cout << "Connect to MySQL database successfully" << endl;

        string query = "SELECT count(*) FROM city";
        cout << "query: " << query << endl;
        unique_ptr<sql::Statement> stmt(conn->createStatement());
        unique_ptr<sql::ResultSet> res(stmt->executeQuery(query));

        while (res && res->next()) {
            cout << res->getInt(1) << endl;
        }
        cout << "query result end" << endl;

        delete conn;
    }
    catch (sql::SQLException& e) {
        cerr << "Error connecting yo MySQL: " << e.what() << endl;
    }
    catch (exception &e ) {
        cerr << "Exception: " << e.what() << endl;
    }
}

void MySqlConnector::_test_api()
{
    const char* server = "localhost";
    const char* username = "db_user";
    const char* password = "db_pass";
    const char* database = "world";

    MYSQL conn;
    if (mysql_init(&conn) == NULL) {
        cout << "mysql_init fail" << endl;
    }

    MYSQL* connection = mysql_real_connect(&conn, server, username, password, database, 3306, (const char*)NULL, 0);
    if (connection == NULL) {
        cout << "mysql_real_connect fail: " << mysql_error(&conn) << endl;
    } else {
        cout << "mysql connect success" << endl;

        if (mysql_select_db(&conn, database) == NULL) {
            const char* query = "SELECT count(*) FROM city";
            cout << "query: " << query << endl;

            if (0 == mysql_query(connection, query)) {
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