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
    vector<int> getRuns(const char* cond);
    string getRunValue(const int run, const char* field);
    string getRunType(const int run);
    string getRunFlag(const int run);
    string getRunStartTime(const int run);
    int getPedRun(const int run);
    int getMIPRun(const int run);
    int getRunEventNumber(const int run);
    int getRunLength(const int run);
    int getRunTrigger(const int run);
    float getRunT1(const int run);
    float getRunT2(const int run);
    float getRunT3(const int run);
    float getRunT4(const int run);
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

vector<int> caliDB::getRuns(const char* cond)
{
    vector<int> runs;

    sqlite3_stmt* stmt;
    char sql[1024];
    sprintf(sql, "SELECT Run FROM %s WHERE %s", caliTableName, cond);
    const char* errMsg = 0;
    int rc = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, &errMsg);
    if (rc == SQLITE_OK)
    {
	while (sqlite3_step(stmt) == SQLITE_ROW) 
	{
	    // Get the data from the current row
	    int run = sqlite3_column_int(stmt, 0); // First column (index 0)
	    runs.push_back(run);
	}
    }
    else
    {
	const char* error = sqlite3_errmsg(db);
	cerr << ERROR << error << endl;;
    }

    sqlite3_finalize(stmt);
    return runs;
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

string caliDB::getRunStartTime(const int run)
{
    return getRunValue(run, "StartTime");
}

int caliDB::getPedRun(const int run)
{
    return stoi(getRunValue(run, "PedRun"));
}

int caliDB::getMIPRun(const int run)
{
    return stoi(getRunValue(run, "MIPRun"));
}

int caliDB::getRunEventNumber(const int run)
{
    return stoi(getRunValue(run, "Events"));
}

int caliDB::getRunLength(const int run)
{
    return stoi(getRunValue(run, "Length"));
}

int caliDB::getRunTrigger(const int run)
{
    return stoi(getRunValue(run, "Trigger"));
}

float caliDB::getRunT1(const int run)
{
    return stof(getRunValue(run, "T1"));
}

float caliDB::getRunT2(const int run)
{
    return stof(getRunValue(run, "T2"));
}

float caliDB::getRunT3(const int run)
{
    return stof(getRunValue(run, "T3"));
}

float caliDB::getRunT4(const int run)
{
    return stof(getRunValue(run, "T4"));
}
#endif
