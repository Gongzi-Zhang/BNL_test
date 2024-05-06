#!/usr/bin/env python3
import os
import sys
import logging
import sqlite3
import pandas as pd

CALIROOT=os.environ['CALIROOT']
gDB = f'{CALIROOT}/database/BNL_test.db'
gTABLE = 'runs'
gCONN = ''
logger = logging.getLogger()
logger.setLevel(logging.INFO)

handler = logging.StreamHandler(sys.stdout)
formatter = logging.Formatter('%(levelname)s - %(message)s')
handler.setFormatter(formatter)
logger.addHandler(handler)

FIELDS = [ 'Id', 'Type', 'Flag', 
    'StartTime', 'StopTime', 'Length', 
    'Geometry', 'Channels', 
    'Trigger', 'T1', 'T2', 'T3', 'T4', 
    'Events', 'LG', 'HG', 'Ped','Size', 
    'PedRun',
    'Note' ]
TextFields = ['Type', 'Flag', 'StartTime', 'StopTime', 'Geometry', 'Size', 'Note']
FreFields = [ 'Id', 'Type', 'Flag', 
    'StartTime', 'StopTime', 'Length', 
    'Trigger', 
    'Events', 'Size', 
    'Note' ]
TYPES = ['ptrg', 'cosmic', 'data', 'junk']
FLAGS = ['good', 'bad', 'susp']

FIELD_WIDTH = {
    'Id': 4, 
    'Type': 6, 
    'Flag': 4,
    'StartTime': 14, 
    'StopTime': 14, 
    'Length': 5,    # elapsed time in h
    'Geometry': 8,
    'Channels': 3,  # number of good channels
    'Trigger': 3,   # trigger logic
    'T1': 5,
    'T2': 5,
    'T3': 5,
    'T4': 5,
    'Events': 7,    # number of events
    'LG': 2,
    'HG': 2,
    'Ped': 3,
    'Size':   4,    # raw data file size in GB
    'PedRun': 4,
    'Note': 30, 
    }

FIELD_TITLE = {
    'Length':   'Len',
    'Channels': 'Chs',
    'Trigger':  'Trg',
    'PedRun': 'PRun'
    }


def check_field(field):
    if field in FIELDS:
        return True
    print(f'''ERROR\tunknown field '{field}'. Allowed fields: {FIELDS}''')
    return False

def check_value(field, value):
    if 'Type' == field:
        if value in TYPES:
            return True
        else:
            print(f'ERROR\tInvalid run type: {value}. Allowed types: {TYPES}')
    elif 'Channels' == field:
        if 1 <= value and value <= 300:
            return True
        else:
            print(f'ERROR\tInvalid channel number: {value}. Allowed range [1, 300]')

    return False

''' create a database connection to a SQLite database '''
def create_connection(db_file):
    global gCONN
    try:
        gCONN = sqlite3.connect(db_file)
        logger.debug(f'connect to sqlite db: {db_file}')
    except sqlite3.Error as e:
        print(e)
        return False
    return True

def close_connection():
    logger.debug(f'close connection to sqlite db: {gDB}')
    gCONN.close()

''' execute the sql statement. Return a cursor object '''
def execute_sql(sql, values=None):
    try:
        c = gCONN.cursor()
        if values is None:
            return c.execute(sql)
        else:
            return c.execute(sql, values)
    except sqlite3.Error as e:
        print(e)
        return None

''' query table existance in the db '''
def query_table(table):
    sql = f'''SELECT name FROM sqlite_master WHERE type='table' AND name = '{table}';'''
    logger.debug(sql)
    result = execute_sql(sql)
    if result.fetchone():
        return True
    else:
        return False

''' formatted output '''
def print_sep_line(fields):
    line = '+'
    for f in fields:
        width = len(f)
        if f in FIELD_WIDTH:
            width = FIELD_WIDTH[f]
        line += '-'*(width+2)
        line += '+'
    print(line)

def print_header(fields):
    header = '|'
    for f in fields:
        width = len(f)
        if f in FIELD_WIDTH:
            width = FIELD_WIDTH[f]
        if f in FIELD_TITLE:
            f = FIELD_TITLE[f]
        header += ' {value:<{width}} '.format(value=f, width=width)
        header += '|'
    print(header)

def print_record(record):
    if not record:
        return

    row = '|'
    for f in record:
        width = len(f)
        if f in FIELD_WIDTH:
            width = FIELD_WIDTH[f]
        row += ' {value:<{width}} '.format(value=str(record[f]), width=width)
        row += '|'
    print(row)

''' print query results '''
def show_query(cursor):
    if not cursor:
        return False

    fields = [des[0] for des in cursor.description]

    print_sep_line(fields)
    print_header(fields)
    print_sep_line(fields)
    for row in cursor.fetchall():
        print_record(dict(zip(fields, row)))
        print_sep_line(fields)

''' show all tables in the db '''
def show_tables():
    sql = f'''SELECT name AS tables FROM sqlite_master WHERE type='table';'''
    logger.debug(sql)
    result = execute_sql(sql)
    show_query(result)

''' drop a table '''
def drop_table(table):
    if not query_table(table):
        print(f'''table '{table}' doesn't exist''')
        return False
    yesno = input(f'''are you sure you want to drop table '{table}': y[es], n[o]\n''')
    if 'y' == yesno:
        yesno2 = input(f'''confirm dropping table '{table}': y[es], n[o]\n''')
        if 'y' == yesno2:
            sql = f'''DROP TABLE IF EXISTS {table};'''
            logger.debug(sql)
            if execute_sql(sql):
                gCONN.commit()
                return True
        else:
            print('cancel dropping')
            return False
    else:
        print('cancel dropping')
        return False

''' table specific '''
''' create a new table '''
def create_table():
    sql = f''' CREATE TABLE IF NOT EXISTS {gTABLE} (
                Id integer PRIMARY KEY,
                Type text,
                Flag text,
                StartTime text,
                StopTime text,
                Length real,
                Geometry text,
                Channels integer,
                Trigger integer,
                T1 float,
                T2 float,
                T3 float,
                T4 float,
                Events integer,
                LG integer,
                HG integer,
                Ped Integer,
                Size text,
                PedRun integer,
                Note text
            );'''
    logger.debug(sql)
    if not execute_sql(sql):
        return False
    gCONN.commit()
    return True

''' query data in the table: return all records as a list'''
def query_records(conditions='1=1', col="*"):
    sql = f'''SELECT {col} FROM {gTABLE} WHERE {conditions};'''
    logger.debug(sql)
    return execute_sql(sql)

def insert_record(record):
    if 'Id' in record:
        result = query_records(f"Id={record['Id']}")
        if result.fetchone():
            logger.warning(f'''record for Id={record['Id']} already exist, will skip it''')
            print_record(record)
            return False

    valid = True
    for f in ('Type', 'Channels'):
        if f in record:
            valid &= check_value(f, record[f])
    if not valid:
        logger.error(f'invalid value in the following record, will not insert it')
        print(record)
        return False

    columns = ', '.join(record.keys())
    placeholders = ':' + ', :'.join(record.keys())
    sql = f'''INSERT INTO {gTABLE}({columns}) VALUES({placeholders});'''
    logger.debug(sql)
    execute_sql(sql, record)
    gCONN.commit()

    return True

''' insert records from a csv file to a table '''
def insert_records(filename):
    if not os.path.exists(filename):
        logger.error(f'file does not exist: {filename}')
        return False
    for i, row in pd.read_csv(filename).iterrows():
        insert_record(row.to_dict())
    return True

''' update a record in the table '''
def update_record(kvalue, field, value):
    if field == 'Id':
        logger.warning("Can't update primary key: Id")
        return False
    if field in TextFields:
        sql = f'''UPDATE {gTABLE} SET {field} = '{value}' WHERE Id = {kvalue};'''
    else:
        sql = f'''UPDATE {gTABLE} SET {field} = {value} WHERE Id = {kvalue};'''
    logger.debug(sql)
    if execute_sql(sql):
        gCONN.commit()
        return True

''' delete a record in a table using primary key values '''
def delete_record(kvalue):
    conditions = f'Id = {kvalue}'
    result = query_records(conditions)
    if not result:
        print('WARNING\tindicated record does not exist in table {gTABLE}')
        return False
    show_query(result)
    yesno = input(f'''are you sure you want to delete above record in table '{gTABLE}': y[es], n[o]\n''')
    if 'y' == yesno:
        yesno2 = input(f'confirm deleting record (Id = {kvalue}) in table {gTABLE}: y[es], n[o]\n')
        if 'y' == yesno2:
            sql = f'DELETE FROM {gTABLE} WHERE Id = {kvalue};'
            logger.debug(sql)
            if execute_sql(sql):
                logger.info('successfully delete the record')
                gCONN.commit()
                return True
        else:
            print('cancel deletion')
            return False
    else:
        print('cancel deletion')
        return False

''' insert records to a table '''
def insert_to_table():
    mode = int(input('please select the insert mode: 1 [csv file], 2 [manual input]\n'))
    if 1 == mode:
        filename = input('please input the file path: ')
        if not insert_records(filename):
            return False
    elif 2 == mode:
        print(f'''please input the following fields for table '{gTABLE}':''')
        values = {}
        values['Id'] = int(input('Run id: '))
        values['Type'] = input(f'Type {TYPES}: ').strip() or 'data'
        values['Flag'] = input(f'Flag {FLAGS}: ').strip() or 'good'
        values['StartTime'] = input('StartTime: ').strip()
        values['StopTime'] = input('StopTime: ').strip()
        values['Length'] = float(input('Length/h: '))
        values['Geometry'] = input('Geometry: ').strip()
        values['Channels'] = int(input('#Channels: '))
        values['Trigger'] = int(input('Trigger Logic: '))
        values['T1'] = float(input('T1: ') or 0.005)
        values['T2'] = float(input('T2: ') or 0.005)
        values['T3'] = float(input('T3: ') or 0.005)
        values['T4'] = float(input('T4: ') or 0.005)
        values['Events'] = int(input('#Events: '))
        values['Size'] = input('Size: ').strip()
        values['LG'] = int(input('LG: ') or 30)
        values['HG'] = int(input('HG: ') or 55)
        values['Ped'] = int(input('Ped: ') or 160)
        values['PedRun'] = int(input('PedRun: '))
        values['Note'] = input('Note: ').strip()
        if not insert_record(values):
            return False
    else:
        print(f'ERROR\tunrecognised mode {mode}')
        return False
    return True

''' update a record in the table '''
def update():
    kvalue = int(input(f'select the record `Id` that you want to update: '))
    conditions = f'Id = {kvalue}'
    result = query_records(conditions)
    if not result.fetchone():
        print(f'''ERROR\tindicated record (Id = {kvalue}) does not exist in table '{gTABLE}' ''')
        return False

    print('record before updating:')
    show_query(query_records(conditions))

    field_prompt = f'0[quit]'
    for i in range(1, len(FIELDS)):
        field_prompt += f', {i}[{FIELDS[i]}]'
    index = int(input(f'which field you want to update: {field_prompt}: '))
    if index < 0 or index >= len(FIELDS):
        print(f'ERROR\tinvalid index {index}')
        return False
    if index == 0:
        print(f'quit the update')
        return True
    field = FIELDS[index]
    value = input(f'''updated value for {field}: ''')
    update_record(kvalue, field, value)

    print('record after updating:')
    show_query(query_records(conditions))

def export_records(fname):
    if os.path.exists(fname):
        print(f'ERROR\t{fname} already exists, please backup it')
        return False
    db_df = pd.read_sql_query(f'SELECT * from {gTABLE};', gCONN)
    db_df.to_csv(fname, index=False)

''' use the function carefully '''
def do_query():
    sql = input('input the sql query: ').strip()
    show_query(execute_sql(sql))


if __name__ == '__main__':
    if not create_connection(gDB):
        print('Error! cannot create the database connection.')
        exit()

    ''' command loop '''
    command = None
    while command != 'q':
        line = input('c[reate table], S[how tables], s[how], i[nsert], u[pdate], e[xport to csv], d[elete record], E[xecute query], D[rop table], q[uit]\n').strip()
        val = line.split()
        command = val[0]

        if command == 'c':
            create_table()
        elif command == 'S':
            show_tables()
        elif command == 's':
            show_query(query_records())
        elif command == 'i':
            insert_to_table()
        elif command == 'u':
            update()
        elif command == 'e':
            fname = input('what is the output file name: ')
            export_records(fname)
        elif command == 'd':
            kvalue = int(input('input the Id you want to delete: '))
            delete_record(kvalue)
        elif command == 'E':
            do_query()
        elif command == 'D':
            drop_table(gTABLE)

    close_connection()
