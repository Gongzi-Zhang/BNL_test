# data
    One can download the raw data from [google drive](https://drive.google.com/drive/folders/1SGtYnyDEmv8edpGVumi8tztlmNIQ3xuf?usp=sharing)

# database
    A sqlite3 database is employed locally for run management, which is not
    synced to the github. A csv file is, which can be found [here](database/db.csv)

    To create the local database, use the command:
    ```
    caliDB init
    ```
    
    To insert the database from the db file, use the commnad:
    ```
    caliDB insert --file database/db.csv
    ```

    Now, one can show the runs:
    ```
    caliDB show	# show the last 10 runs
    caliDB show --all	# show all runs
    ```

# package dependency
* sqlite3
* rename
* c++
  * spdlog
* python:
  * pandas
  * uproot
