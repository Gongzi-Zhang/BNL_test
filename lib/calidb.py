#!/usr/bin/env python3
from utilities import *
from db import *

class caliDB:
    def __init__(self):
        createConnection(caliDbName)

    def getRunValue(self, run, field):
        sql = f'''SELECT {field} FROM {caliTableName} WHERE Run = {run}'''
        cursor = executeSql(sql)
        row = cursor.fetchone()
        if row is None:
            logger.error(f'run {run} not found in the db')
            return False
        else:
            return row[0]
        
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
