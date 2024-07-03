# operation
* Add a run to the database manually:
```
bin/add_run /path/to/ptrg/run_Info.txt [run_type]
```

* pedestal run:
Take at least one ptrg run every day and add it to the database/analyze it manually
```
bin/add_run /path/to/ptrg/run_Info.txt ptrg
```
Remember to add all runs before it to the database manually before adding the ptrg run.
    * Update the PedRun value in the [config file](data/config.cfg)
* cosmic
When taking cosmic run, use 
    * Bunch Trigger Source: TLOGIC; 
    * Trigger Logic: MAJ64
    * Majority Level: 2
and
    * Preset time: 360 m


* Trigger tile threshold:

Whenever you modify the trigger tile threshold value or the trigger logic, add the old runs to the database first:
```
bin/add_run /path/to/data/run_Info.txt [data|cosmic]
```
then update the corresponding information in the [config file](data/config.cfg), 
which will affect all following runs.

# data
One can download the raw data from [google drive](https://drive.google.com/drive/folders/1SGtYnyDEmv8edpGVumi8tztlmNIQ3xuf?usp=sharing)

# database
A sqlite3 [database](database/BNL_test.db) is employed locally for run management. 
Meanwhile, a [csv file](database/db.csv) is also created for quick search.

To create the local database, use the command:
```
caliDB init
```

To insert/update the database from the db file, use the commnad:
```
caliDB insertf --file database/db.csv
```
Now, one can show the runs:
```
caliDB show	# show the last 10 runs
caliDB show -e 20	# show the last 20 runs
caliDB show --all	# show all runs
caliDB show -f	# display all fields
```
# analysis
To convert the raw txt file into a root file:
```
bin/convert run_number
```

To make QA plots:
```
bin/QA run_number
```

# package dependency
* root (install it manually to ensure python support)
* sqlite3
* rename
* gnuplot: required by Janus
* c++
  * sqlite3: libsqlite3-dev
  * nlohmann-json3
* python:
  * pandas
  * matplotlib
  * scipy
  * mplhep
  * uproot
  * datetime
