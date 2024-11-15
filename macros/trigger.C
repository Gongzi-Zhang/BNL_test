// count number of runs for different triggers 
#include "utilities.h"
#include "cali.h"
#include "db.h"

void trigger() 
{
    caliDB db;

    map<int, map<float, map<float, map<float, int>>>> runNumber;
    map<int, map<float, map<float, map<float, int>>>> eventNumber;
    
    // for (int run=cali::run24PPStartRun; run<=cali::run24PPEndRun; run++)    // PP 
    for (int run=cali::run24AuAuStartRun; run<=cali::run24AuAuEndRun; run++)    // AuAu 
    {
	if (db.getRunType(run) != "data" || db.getRunFlag(run) != "good")
	    continue;

	int trg	 = db.getRunTrigger(run);
	if (trg == -2) cout << DEBUG << run << "\t" << db.getRunType(run) << endl;
	float T1 = db.getRunT1(run);
	float T2 = db.getRunT2(run);
	float T3 = db.getRunT3(run);
	runNumber[trg][T1][T2][T3]++;
	eventNumber[trg][T1][T2][T3] += db.getRunEventNumber(run);
    }
    for (const auto& [trg, v1] : runNumber)
    {
	for (const auto& [T1, v2] : v1)
	{
	    for (const auto& [T2, v3] : v2)
	    {
		for (const auto& [T3, c] : v3)
		{
		    cout << cali::trigger[trg] 
			 << "\t" << T1
			 << "\t" << T2
			 << "\t" << T3
			 << "\t" << c << "\t" << eventNumber[trg][T1][T2][T3] << endl;
		}
	    }
	}
    }
}
