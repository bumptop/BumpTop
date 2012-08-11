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

# Based off the GData spreadsheet example code
# Known issue: column names with spaces or underscores appear to cause this script to barf
# For now, just use a dash ('-')
try:
  from xml.etree import ElementTree
except ImportError:
  from elementtree import ElementTree
import gdata.spreadsheet.service
import gdata.service
import atom.service
import gdata.spreadsheet
import atom
import getopt
import sys
import string
import time
from util import *

class SimpleCRUD:

    def __init__(self, email, password):
        self.gd_client = gdata.spreadsheet.service.SpreadsheetsService()
        self.gd_client.email = email
        self.gd_client.password = password
        self.gd_client.source = 'Spreadsheets GData Sample'
        self.gd_client.ProgrammaticLogin()
        self.curr_key = ''
        self.curr_wksht_id = ''
        self.list_feed = None
    
    def _OpenSpreadsheet(self, spreadsheet_title):
        # Get the list of spreadsheets
        feed = self.gd_client.GetSpreadsheetsFeed()
        
        usage_statistics = None
        for entry in feed.entry:
            if entry.title.text == spreadsheet_title:
                usage_statistics = entry
        
        if not usage_statistics: raise "couldn't find the spreadsheet", spreadsheet_title
        
        id_parts = usage_statistics.id.text.split('/')		
        self.curr_key = id_parts[-1]
        
    def _LoadWorksheets(self):
        desired_worksheets = {'sheet1' : None, 'sheet2' : None}
        
        feed = self.gd_client.GetWorksheetsFeed(self.curr_key)
        for entry in feed.entry:
            if entry.title.text in desired_worksheets:
                desired_worksheets[entry.title.text] = entry
                
        for worksheet_title, worksheet in desired_worksheets.iteritems():
            if not worksheet:
                new_worksheet = self.gd_client.AddWorksheet(worksheet_title, 1, 1, self.curr_key)
                
    def _OpenWorksheet(self, worksheet_title):
        feed = self.gd_client.GetWorksheetsFeed(self.curr_key)
        
        worksheet = None
        for entry in feed.entry:
            if entry.title.text == worksheet_title:
                worksheet = entry
                
        if not worksheet: raise "couldn't find the worksheet"
        
        id_parts = worksheet.id.text.split('/')
        self.curr_wksht_id = id_parts[-1]
        print self.curr_key
        print self.curr_wksht_id
        
    def _UpdateWorksheetData(self, worksheet_title, schema, tuplelist):
        """
        * Takes tuplelist and dumps it into a worksheet called worksheet_title.
        * The first row is filled with the schema.
        * Creates worksheet if it does not already exist.
        * Will return an exception if there are not enough columns-- must be done manually (defect of gdata api)
        """
        num_cols = len(schema)
        num_rows = len(tuplelist) + 1 # +1 for the schema, which is the first row
        
        # First, update the worksheet to have the same number of rows and columns as the tuplelist
        feed = self.gd_client.GetWorksheetsFeed(self.curr_key)
        
        worksheet = find(lambda e: e.title.text == worksheet_title, feed.entry)
                
        if not worksheet: 
            # create it if it doesn't exist
            worksheet = self.gd_client.AddWorksheet(worksheet_title, str(num_rows), str(num_cols), self.curr_key)

        
        # try to adjust the number of rows/cols
        worksheet.col_count.text = str(num_cols)
        worksheet.row_count.text = str(num_rows)
        edited_worksheet = self.gd_client.UpdateWorksheet(worksheet)
        id_parts = edited_worksheet.id.text.split('/')
        self.curr_wksht_id = id_parts[-1]
        
        # UpdateWorksheet bug seems to be fixed, just assertion to make sure this succeeds
        assert edited_worksheet.col_count.text == str(num_cols)
        assert edited_worksheet.row_count.text == str(num_rows)
        
        
        
        # do a batch update of the data using cell feed

        # seems that batch updates are limited to a certain size (perhaps time limit?) - 900 seems to do the trick
        num_rows_possible_in_one_batch = 900 / num_cols
        for row_batch_number in xrange(1+num_rows/num_rows_possible_in_one_batch):
            first_row = row_batch_number*num_rows_possible_in_one_batch
            # first, get the cell feed
            query = gdata.spreadsheet.service.CellQuery()
            query['min-col'] = str(1)
            query['max-col'] = str(num_cols)
            query['return-empty'] = 'true'
            cell_feed = self.gd_client.GetCellsFeed(self.curr_key, self.curr_wksht_id, query=query)
            

            
            batchRequest = gdata.spreadsheet.SpreadsheetsCellsFeed()        
            
            def update_cell(i, row, j, cell):
                cell_feed.entry[i*num_cols + j].cell.inputValue = str(cell)
                batchRequest.AddUpdate(cell_feed.entry[i*num_cols + j])
                
            # insert the schema
            for i, row in enumerate([schema]):
                for j, cell in enumerate(row):
                    update_cell(i, row, j, cell)
            
            # insert whatever rows will fit with the space we have
            
            print (first_row,first_row+len(tuplelist[first_row:first_row+num_rows_possible_in_one_batch]))
            print "Uploading", num_cols*len(tuplelist[first_row:first_row+num_rows_possible_in_one_batch])
            for i, row in enumerate(tuplelist[first_row:first_row+num_rows_possible_in_one_batch]):
                i = i + first_row + 1 #offset by one to take into account the schema
                for j, cell in enumerate(row):
                    update_cell(i, row, j, cell)
                    
            updated = self.gd_client.ExecuteBatch(batchRequest, cell_feed.GetBatchLink().href)
        
               
    def _PrintFeed(self, feed):
      
        for i, entry in enumerate(feed.entry):
          if isinstance(feed, gdata.spreadsheet.SpreadsheetsCellsFeed):
            print '%s %s\n' % (entry.title.text, entry.content.text)
          elif isinstance(feed, gdata.spreadsheet.SpreadsheetsListFeed):
            #print '%s %s %s\n' % (i, entry.title.text.encode('UTF-8'), entry.content.text.encode('UTF-8'))
            print '%s %s %s\n' % (i, entry.title.text, entry.content.text)
          else:
            print '%s %s\n' % (i, entry.title.text)
            

        
if __name__ == '__main__':
    user = 'feedback@bumptop.com'
    pw = 'lombardi'
    sample = SimpleCRUD(user, pw)
    sample._OpenSpreadsheet('test')
    sample._UpdateWorksheetData('Sheet1', ['col1', 'co-l2', 'co-l3', 'co-l4'],[(4, 3, 2, 1), (3, 'a', 5, 9),(3, 'a', 8, 9),(4, 'b', 9, 10),(3, 'a', 8, 9),(3, 'a', 8, 9)])
