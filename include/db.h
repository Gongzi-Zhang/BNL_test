#ifndef __CALI_DB__
#define __CALI_DB__

#include <iostream>
#include <sstream>
#include <cstring>
#include <string>
#include <sqlite3.h>
#include "utilities.h"
#include "cali.h"

using namespace std;

const std::string caliDbName = string(cali::CALIROOT) + "/database/BNL_test.db";
const char* caliTableName = "runs";

class caliDB {
  private:
    sqlite3 *db;

  public:
    caliDB();
    ~caliDB();
    bool createConnection();
    bool closeConnection();
    string getRunValue(const int run, const char* field);
    string getRunType(const int run);
    string getRunFlag(const int run);
    int getPedRun(const int run);
    int getMIPRun(const int run);
};

caliDB::caliDB() 
{
    int rc = sqlite3_open(caliDbName.c_str(), &db);
    if (rc != SQLITE_OK)
    {
	cerr << FATAL << "Can't connect to the database" << endl;
	exit(4);
    }
}

caliDB::~caliDB() 
{
    sqlite3_close(db);
}

string caliDB::getRunValue(const int run, const char* field)
{
    sqlite3_stmt* stmt;
    char sql[1024];
    sprintf(sql, "SELECT %s FROM %s WHERE Run = %d", field, caliTableName, run);
    const char* errMsg = 0;
    int rc = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, &errMsg);
    if (rc != SQLITE_OK)
    {
	const char* error = sqlite3_errmsg(db);
	cerr << ERROR << error << endl;;
	sqlite3_finalize(stmt);
	return "";
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW)
    {
	sqlite3_finalize(stmt);
	return "";
    }

    const unsigned char* value = sqlite3_column_text(stmt, 0);
    stringstream ss;
    ss << value;

    sqlite3_finalize(stmt);

    return ss.str();
}

string caliDB::getRunType(const int run)
{
    return getRunValue(run, "Type");
}

string caliDB::getRunFlag(const int run)
{
    return getRunValue(run, "Flag");
}

int caliDB::getPedRun(const int run)
{
    return stoi(getRunValue(run, "PedRun"));
}

int caliDB::getMIPRun(const int run)
{
    return stoi(getRunValue(run, "MIPRun"));
}
#endif
