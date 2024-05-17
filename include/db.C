#include <iostream>
#include <cstring>
#include "utilities.h"
#include "db.h"

caliDB::caliDB() 
{
    int rc = sqlite3_open(caliDbName.c_str(), &db);
    if (rc != SQLITE_OK)
    {
	std::cerr << FATAL << "Can't connect to the database" << std::endl;
	exit(4);
    }
}

caliDB::~caliDB() 
{
    sqlite3_close(db);
}

const char* caliDB::getRunValue(const int run, const char* field)
{
    sqlite3_stmt* stmt;
    char sql[1024];
    sprintf(sql, "SELECT %s FROM %s WHERE Id = %d", field, caliTableName, run);
    const char* errMsg = 0;
    int rc = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, &errMsg);
    if (rc != SQLITE_OK)
    {
	const char* error = sqlite3_errmsg(db);
	std::cerr << ERROR << error << std::endl;;
	sqlite3_finalize(stmt);
	return NULL;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW)
    {
	sqlite3_finalize(stmt);
	return NULL;
    }

    return (const char*)sqlite3_column_text(stmt, 0);
}

const char* caliDB::getRunType(const int run)
{
    return getRunValue(run, "Type");
}

const char* caliDB::getRunFlag(const int run)
{
    return getRunValue(run, "Flag");
}

int caliDB::getPedRun(const int run)
{
    return atoi(getRunValue(run, "PedRun"));
}
