#ifndef __CALI_DB__
#define __CALI_DB__

#include <string>
#include <sqlite3.h>
#include "cali.h"

const std::string caliDbName = std::string(cali::CALIROOT) + "/database/BNL_test.db";
const char* caliTableName = "runs";

class caliDB {
  private:
    sqlite3 *db;

  public:
    caliDB();
    ~caliDB();
    bool createConnection();
    bool closeConnection();
    const char* getRunValue(const int run, const char* field);
    const char* getRunType(const int run);
    const char* getRunFlag(const int run);
    int getPedRun(const int run);
};
#endif
