# operation
* Take at least one ptrg run every day and add it to the database/analyze it manually
```
scripts/add_run.sh /path/to/ptrg/run_Info.txt ptrg
```
  * Update the PedRun value in the [config file](data/config.cfg)

* Trigger tile threshold:

Whenever you modify the trigger tile threshold value, add the old runs to the database first:
```
scripts/add_run.sh /path/to/data/run_Info.txt [data|cosmic]
```
then update the trigger tile threshold values in the [config file](data/config.cfg), 
which will affect all following runs.

# data
One can download the raw data from [google drive](https://drive.google.com/drive/folders/1SGtYnyDEmv8edpGVumi8tztlmNIQ3xuf?usp=sharing)

# database
A sqlite3 database is employed locally for run management, which is not
synced to the github. A csv file is, which can be found [here](database/db.csv)

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
* root
* sqlite3
* rename
* c++
  * sqlite3: libsqlite3-dev
* python:
  * pandas
  * uproot
  * datetime
  * copy
  * json
