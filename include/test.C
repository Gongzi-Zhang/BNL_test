#include <iostream>
#include "utilities.h"
#include "cali.h"
#include "db.h"

using namespace std;
int main()
{

    // log macro
    cout << INFO << "hello world" << endl;
    cout << DEBUG << "debug" << endl;
    cout << ERROR << "error" << endl;
    cout << WARNING << "warning" << endl;
    cout << FATAL << "fatal" << endl;

    // db
    caliDB db = caliDB();
    const int run = 150;
    printf("run type of run %d is %s\n", run, db.getRunType(run));
    printf("run flag of run %d is %s\n", run, db.getRunFlag(run));
    printf("ped run of run %d is %d\n", run, db.getPedRun(run));
}

