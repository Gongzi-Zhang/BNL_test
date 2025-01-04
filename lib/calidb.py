#!/usr/bin/env python3
from utilities import *
from db import *

class caliDB:
    def __init__(self):
        createConnection(caliDbName)

    def query(self, cond, fields="Run"):
        # !!! check fields !!!
        sql = f'''SELECT {fields} FROM {caliTableName} WHERE {cond};'''
        cursor = executeSql(sql)
        rows = cursor.fetchall()
        if rows is None:
            logger.error(f'no result from the query: {sql}, please check it')
            return False

        result = {column[0]: [] for column in cursor.description}
        for row in rows:
            for idx, column in enumerate(cursor.description):
                result[column[0]].append(row[idx])

        return result

    def getRunValue(self, run, field):
        cond = f'Run = {run}'
        values = self.query(cond, field)
        if values:
            return values[field][0]
        else:
            return False
        
    def getRunType(self, run):
        return self.getRunValue(run, 'Type')

    def getRunFlag(self, run):
        return self.getRunValue(run, 'Flag')

    def getRunPedRun(self, run):
        return int(self.getRunValue(run, 'PedRun'))

    def getRunStartTime(self, run):
        return self.getRunValue(run, 'StartTime')

if __name__ == '__main__':
    ''' test '''
    db = caliDB()
    run = 200
    logger.info(f'run {run} has type of: {db.getRunType(run)}')
    logger.info(f'run {run} has flag of: {db.getRunFlag(run)}')
    logger.info(f'run {run} has ped run of: {db.getRunPedRun(run)}')
