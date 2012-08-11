# Copyright 2011 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sqlite3
from util import *


## very simple object for dealing with a list of tuples. converts any number to type float; otherwise,
## the object remains as type text. you can perform destructive or non-destructive selects
class InMemorySQLTable(object):

    def _get_new_table_id(self):
        i = 0
        while i in self.table_ids: i += 1
        self.table_ids[i] = True
        return i

    def _release_table_id(self, i):
        if i in self.table_ids: del self.table_ids[i]

    @staticmethod
    def fromtuplelist(tuple_list, col_names = None, db_name = None, are_schema_columns_numeric = None):
        obj = InMemorySQLTable()
        if not db_name: db_name = ':memory:'
        obj.conn = sqlite3.connect(db_name)
        obj.cursor = obj.conn.cursor()
        obj.table_ids = {}
        
        if are_schema_columns_numeric == None:
            are_schema_columns_numeric = [isnumeric(v) for v in tuple_list[0]]

        def get_sql_type(is_numeric):
            return ("TEXT", "FLOAT")[is_numeric]
            

        if not col_names: col_names = ["col%d" % x for x in xrange(len(are_schema_columns_numeric))]
        
        schema = [col_name + " " + get_sql_type(is_numeric) for (col_name, is_numeric) in zip(col_names, are_schema_columns_numeric)]

        obj.table_id = obj._get_new_table_id()
        sql_query = 'CREATE TABLE table%d (%s)' % (obj.table_id, ', '.join(schema))
        #print sql_query

        obj.cursor.execute(sql_query)

        for tuple in tuple_list:
            # convert:
            #    string -> "the string in quotes"
            #    number -> string
            new_tuple = []
            for (is_numeric, value) in zip(are_schema_columns_numeric, tuple):
                if is_numeric:
                    new_tuple.append(str(value))
                else:
                    new_tuple.append('"%s"' % value)
                
            sql_query = "INSERT INTO table%d VALUES (%s)" % (obj.table_id, ', '.join(new_tuple))
            #print sql_query
            obj.cursor.execute(sql_query)

        return obj

    @staticmethod
    def fromsqltable(in_memory_sql_table, table_id):
        obj = InMemorySQLTable()
        obj.conn = in_memory_sql_table.conn
        obj.table_id = table_id
        obj.cursor = in_memory_sql_table.cursor
        obj.table_ids = in_memory_sql_table.table_ids
        return obj
        

    ## select NEW_COLUMNS from table where WHERE
    def select_and_replace(self, new_columns, where = '', from_str = None):
        # execute the query
        obj = self.select(new_columns, where, from_str)

        # delete the table that was being selected from
        self.cursor.execute("DROP TABLE table%d" % self.table_id)
        self._release_table_id(self.table_id)
        
        self.table_id = obj.table_id

    def select(self, new_columns, where = '', from_str = None):
        new_table_id = self._get_new_table_id()
        new_columns = new_columns.replace("#TABLE#", "table%d" % self.table_id)
        if from_str: from_str = from_str.replace("#TABLE#", "table%d" % self.table_id)
        if where: where = where.replace("#TABLE#", "table%d" % self.table_id)
        
        # construct the query
        sql_query = "CREATE TABLE table%d AS SELECT %s FROM " % (new_table_id, new_columns)
        if from_str == None: 
            sql_query += "table%d t" % self.table_id
        else:
            sql_query += from_str
        if where: sql_query += " WHERE %s" % where
        
        #print sql_query

        # execute it
        self.cursor.execute(sql_query)

        # return the new table generated
        return InMemorySQLTable.fromsqltable(self, new_table_id)
        
    def update(self, values, where = ''):
        sql_query = "UPDATE table%d SET %s" % (self.table_id, ', '.join([_var + ' = ' + _val for _var, _val in values.iteritems()]))
        if where: sql_query += " WHERE %s" % where
        self.cursor.execute(sql_query)
        

    # return a list of tuples, i.e. [(row1val1, row1val2, row1val3), (row2val1, row2val2, row2val3)]
    def totuplelist(self):
        sql_query = "SELECT * FROM table%d" % self.table_id
        self.cursor.execute(sql_query)
        return [tuple for tuple in self.cursor]
        
    def tovalue(self):
        return self.totuplelist()[0][0]
        

