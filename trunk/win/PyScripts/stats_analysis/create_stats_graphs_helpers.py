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

from parse_stats import StatsParser
from datetime import datetime, timedelta, date
import os
from util import *
from inmemorysqltable import InMemorySQLTable
from googlespreadsheets import SimpleCRUD
import math
import time
import subprocess
import xml.parsers.expat

class StatsFile: pass

SCRIPTS_DIR = "/home/47095/users/.home/domains/s47095.gridserver.com/scripts/"

# runs a process with a certain timeout, and if it's not done in that time,
# restarts it. done because ftpsync.pl seems to hang sometimes, but if you 
# keep running it and killing it when it hangs it will eventually finish
def run_and_restart(cmd, timeout_in_seconds):

    assert timeout_in_seconds > 1
    timeout = timedelta(seconds=timeout_in_seconds)
    
    done = False
    
    while not done:
        print "restarting..."
        p = subprocess.Popen(cmd)
        start_time = datetime.now()
    
        while datetime.now() - start_time < timeout:
            if p.poll() != None:
                done = True # the process is done
                break
            time.sleep(0.1) # otherwise, wait a bit
            
def sync_stats_from_lunar_pages():
    run_and_restart(["ftpsync.pl", "-g", "-f", SCRIPTS_DIR + "stats", "ftp://omexca2:Flames285@omexcards.com/public_html/bumptop/download/stats"], 120)

def sync_stats_from_lunar_pages_hwmanfmode():
    run_and_restart(["ftpsync.pl", "-g", "-f", SCRIPTS_DIR + "stats/toshiba", "ftp://omexca2:Flames285@omexcards.com/public_html/bumptop/download/stats/toshiba"], 120)

    
def sync_stats_locally(extra_dir):
    today = date.today()
    subprocess.call("mkdir %sstats_upload_inbox/stats_dir_uploaded_%d_%d_%d/" % (SCRIPTS_DIR, today.month, today.day, today.year), shell=True)
    
    if extra_dir: subprocess.call("mkdir %sstats_upload_inbox/stats_dir_uploaded_%d_%d_%d/%s" % (SCRIPTS_DIR, today.month, today.day, today.year, extra_dir), shell=True)

    subprocess.call("find %sstats_upload_inbox/%s -maxdepth 1 -name 'screenshot_*.jpg' -exec mv '{}' %s../html/stats/screenshots/%s \;" % (SCRIPTS_DIR, extra_dir, SCRIPTS_DIR, extra_dir), shell=True)
    subprocess.call("find %sstats_upload_inbox/%s -maxdepth 1 -name 'stats_*.xml' -exec mv '{}' %sstats_upload_inbox/stats_dir_uploaded_%d_%d_%d/%s \;" % (SCRIPTS_DIR, extra_dir, SCRIPTS_DIR, today.month, today.day, today.year, extra_dir), shell=True)
    
    # copy to a directory in the stats folder
    subprocess.call("mkdir %sstats/stats_dir_uploaded_%d_%d_%d/" % (SCRIPTS_DIR, today.month, today.day, today.year), shell=True)
    
    if extra_dir: subprocess.call("mkdir %sstats/stats_dir_uploaded_%d_%d_%d/%s" % (SCRIPTS_DIR, today.month, today.day, today.year, extra_dir), shell=True)
    
    print "Syncing local directory... ",
    subprocess.call("rsync -r %sstats_upload_inbox/stats_dir_uploaded_%d_%d_%d/%s %sstats/stats_dir_uploaded_%d_%d_%d/%s" % (SCRIPTS_DIR, today.month, today.day, today.year, extra_dir, SCRIPTS_DIR, today.month, today.day, today.year, extra_dir), shell=True)
    print "done"

def get_list_of_stats_files(base_dir):
    def is_stats_filename(filename):
        return filename.startswith('stats_') and filename.endswith('.xml') and (len(filename.split('_')) == 7 or len(filename.split('_')) == 6)
        
    stats_files = [base_dir + _f for _f in os.listdir(base_dir) if is_stats_filename(_f)]

    # check in directories as well
    stats_dirs = [_d for _d in os.listdir(base_dir) if _d.startswith('stats_dir_')]
    for dir in stats_dirs:
        stats_files += [base_dir + "%s/%s" % (dir, _f) for _f in os.listdir(base_dir + dir) if is_stats_filename(_f)]
        
    return stats_files
            
def parse_stats_file(filename):
    
    f = open(filename, 'r')
    data = f.read()
    f.close()
    
    if not data: 
        print "File read error:", filename
        return None
    
    s = StatsParser()
    d = None
    try:
        d = s.parse(data)
    except xml.parsers.expat.ExpatError:
        print "Excluding %s due to parse error" % filename
        return None
        
    for key in d:
        if 'time' in key.lower():
            try:
                d[key] /= 60.0
            except TypeError:
                print "Excluding %s because %s is not a number (value = %s)" % (filename, key, d[key])
                return None
        if '.ram' in key.lower():
            d[key] /= 1048576.0 # bytes to megabytes

    
    split_filename = os.path.splitext(os.path.basename(filename))[0].split('_') # drop the .xml

    
    if d:
        s = StatsFile()
        s.__dict__["filename"] = filename
        s.__dict__["user_id"] = split_filename[1]
        s.__dict__["dir_hash"] = split_filename[2]
        s.__dict__["collected_date"] = date(year = int(split_filename[3]),
                                            month = int(split_filename[4]),
                                            day = int(split_filename[5]))
        if len(split_filename) >= 7:
            s.__dict__["upload_time"] = datetime.fromtimestamp(int(split_filename[6]))
        else:
            s.__dict__["upload_time"] = datetime(int(split_filename[3]), int(split_filename[4]), int(split_filename[5]))
        
        s.__dict__["data"] = {}
        for key, value in d.iteritems():
            s.data[make_name_nice_for_sql(key)] = value
        
        
        d = s
    
    return d
    
class Table: pass

def make_name_nice_for_sql(var_name):
    return var_name.replace('.', '_')
    
def bumptop_vars_and_names_to_human_readable_names(bumptop_vars):
    return dict([(make_name_nice_for_sql(_line.split(', ')[0]), _line.split(', ')[2]) for _line in bumptop_vars.strip().split('\n')])

def parsed_stats_files_to_stats_raw_table(parsed_stat_files, bumptop_vars, bumptop_var_sql_types):
    stats_tuplelist = []
    
    attributes_to_include = ['user_id', 'dir_hash', 'collected_date', 'upload_time']
    for p in parsed_stat_files:
        row = [getattr(p, _var) for _var in attributes_to_include]
        for (_var, _var_type) in zip(bumptop_vars, bumptop_var_sql_types):
            try:
                row.append(p.data[_var])
            except KeyError:
                if _var_type == 'FLOAT':
                    row.append(0)
                else:
                    row.append("")
        row.append('') # empty cell for the release annotations e.g. Beta 11
        stats_tuplelist.append(row) 
        
        for var_name in p.data.iterkeys():
            if var_name not in bumptop_vars:
                print "Unknown field detected: %s" % var_name
    table = Table()
    table.__dict__['schema'] = attributes_to_include + bumptop_vars + ['release']
    table.__dict__['tuplelist'] = stats_tuplelist
    return table
    
# time should not exceed 24 hours in a day
def all_times_under_24_hours(schema):
    def inner(row):
        for name, value in zip(schema, row):
            if name != 'upload_time' and 'time' in name.lower():
                if value > 60.0*24:
                    return False
        return True
    return inner
    
def less_than_100_crashes_reported(schema):
    def inner(row):
        num_crashes = row[find("d_bt_crashes", schema)]
        return num_crashes < 100
    return inner
    
def times_not_negative(schema):
    def inner(row):
        for name, value in zip(schema, row):
            if name != 'upload_time' and 'time' in name.lower():
                if value < 0:
                    return False
        return True
    return inner
    
def find(find_value, seq):
    for i, value in enumerate(seq):
        if value == find_value:
            return i
    return -1
    
def submitted_after_july_2008(schema):
    def inner(row):
        collected_date = row[find("collected_date", schema)]
        if collected_date < date(year=2008, month=7, day=14):
            #print "Filtering out row (user_id = %s, dir_hash = %s, collected_date = %s) because collected before july 14 (%s)" % tuple(row[0:3]+[collected_date])
            return False
        return True
    return inner
    
def uploaded_less_than_one_week_after_collected(schema):
    def inner(row):
        uploaded_time = row[find("upload_time", schema)]
        collected_date = row[find("collected_date", schema)]
        if uploaded_time.date() > collected_date + timedelta(days=7) or collected_date > uploaded_time.date():
            #print "Filtering out row (user_id = %s, dir_hash = %s, collected_date = %s) because uploaded more than 7 days after collected (%s, %s)" % tuple(row[0:3]+[uploaded_time, collected_date])
            return False
        return True
    return inner

def collected_today_or_earlier(schema):
    def inner(row):
        collected_date = row[find("collected_date", schema)]
        if collected_date > date.today():
            print "Filtering out row (user_id = %s, dir_hash = %s, collected_date = %s) because collected date is in the future" % tuple(row[0:3])
 
        return collected_date <= date.today()
    return inner
    
def user_id_is_not(user_ids):
    sql = ["user_id != '%s'" % user_id for user_id in user_ids]
    " AND ".join(sql)
    return "(%s)" % " AND ".join(sql)
    
def fill_in_missing_days(cursor, base_date):
    invited_users = []
    for (the_date, num_users) in cursor:
        base_date += timedelta(days=1)
        while the_date != base_date:
             invited_users.append(("%04d-%02d-%02d" % (base_date.year, base_date.month, base_date.day), 0L))   
             base_date += timedelta(days=1)
        invited_users.append(("%04d-%02d-%02d" % (the_date.year, the_date.month, the_date.day), num_users))
    return invited_users

def get_invite_stats():
    import MySQLdb
    import datetime

    conn = MySQLdb.connect (host = "internal-db.s47095.gridserver.com",
                           user = "db47095_usradmin",
                           passwd = "Flames285",
                           db = "db47095_bumptopusers")

    cursor = conn.cursor ()

    cursor.execute ("SELECT DATE(sent_time), count(1) FROM invite_codes WHERE sent_time IS NOT NULL AND DATE(sent_time) != DATE('0000-00-00') GROUP BY 1;")
    invited_users = fill_in_missing_days(cursor, datetime.date(2008, 7, 7))

    

    cursor.execute("SELECT COUNT(*) FROM invite_codes WHERE sent_time IS NULL;")
    num_old_invited_users = [tuple for tuple in cursor]
    
    invited_users = [("%04d-%02d-%02d" % (2008,7,7), num_old_invited_users[0][0])] + invited_users


    cursor.execute("SELECT DATE(first_auth_time), count(1) FROM invite_codes WHERE first_auth_time IS NOT NULL AND DATE(first_auth_time) != DATE('0000-00-00') GROUP BY 1;")
    authorized_users = fill_in_missing_days(cursor, datetime.date(2008, 7, 7))

    cursor.execute("SELECT COUNT( * ) FROM invite_codes WHERE first_auth_time IS NULL AND md5_hashes IS NOT NULL ;")
    num_old_authorized_users = [tuple for tuple in cursor]
    authorized_users = [("%04d-%02d-%02d" % (2008,7,7), num_old_invited_users[0][0])] + authorized_users


    cursor.close ()
    conn.close ()

    return { "invites_sent": invited_users, "invites_authorized": authorized_users }
    
def is_n_days_before(n, the_date, date_var_name = 'collected_date'):
    if type(the_date) is not str:
        the_date = "'%s'" % the_date
    return "julianday(%s) - 7 < julianday(%s) AND julianday(%s) <= julianday(%s)" % (the_date, date_var_name, date_var_name, the_date)
    
def stats_raw_to_daily_avg(stats_raw, time_series_vars, release_dates):
    stats_daily_avg_cols	=	['collected_date',	'COUNT(DISTINCT user_id)']
    stats_daily_avg_schema	= 	['collected_date',	'num_users']

    stats_daily_avg_cols += 	["SUM(%s) / COUNT(DISTINCT user_id) AS %s" % (_var, _var) for _var in time_series_vars] + ['release']
    stats_daily_avg_schema += 	time_series_vars + ['release']
    
    stats_daily_avg_cols +=		['(SELECT COUNT(DISTINCT t2.user_id) FROM #TABLE# t2 WHERE %s) AS num_users_in_last_week' % is_n_days_before(7, 't.collected_date', date_var_name='t2.collected_date')]
    stats_daily_avg_schema +=	['num_users_in_last_week']



    stats_daily_avg_per_stat = stats_raw.select(", ".join(stats_daily_avg_cols), where="1 GROUP BY collected_date")

    #####
    #####
    print "Add release annotations"
    #####
    #####
    for release, release_date in release_dates:
        stats_daily_avg_per_stat.update({'release':'"%s"' % release}, where="collected_date = '%s'" % release_date)
    
    table = Table()
    table.__dict__['schema'] = stats_daily_avg_schema
    table.__dict__['sqlitetable'] = stats_daily_avg_per_stat
    return table
    
def stats_raw_to_global_total(stats_raw, time_series_vars):
    totals_cols = ["SUM(%s) AS %s" % (_var, _var) for _var in time_series_vars]
    

    stats_global_totals_per_stat = stats_raw.select(", ".join(totals_cols)).totuplelist()[0]

    return_val = dict(zip(time_series_vars, list(stats_global_totals_per_stat)))
    return return_val
    
def stats_raw_to_weekly_totals_per_user(stats_raw, histogram_vars, time_range_end = None, time_range_num_days = None):
    histogram_cols = 		['user_id']
    histogram_schema = 		['user_id']

    histogram_cols += 		["SUM(%s) AS %s" % (_var, _var) for _var in histogram_vars]
    histogram_schema += 	histogram_vars
    
    histogram_cols +=		['COUNT(*) AS num_stats_reports', 'julianday(MAX(collected_date)) - julianday(MIN(collected_date)) AS num_days_from_first_to_last_stat_report']
    histogram_schema += 	['num_stats_reports', 'num_days_from_first_to_last_stat_report' ]
    
    if time_range_end:
        where_q = is_n_days_before(time_range_num_days, time_range_end)
    else:
        where_q = "1"
    


    return stats_raw.select(", ".join(histogram_cols), where=where_q + " GROUP BY user_id")
    
def create_histogram_table(var_name, stats_weekly_totals_per_user):
    min_val, max_val = stats_weekly_totals_per_user.select("MIN(%s), MAX(%s)" % (var_name, var_name)).totuplelist()[0]
    min_val = math.floor(min_val)
    max_val = math.ceil(max_val)
    if min_val == 0 and max_val == 0: max_val = 1
    
    bucket_width = math.ceil((max_val - min_val) / 30.0)
    if not (bucket_width * 30 + min_val > max_val): bucket_width += 1.0
    
    histogram = []
    bucket = min_val - bucket_width
    while bucket < max_val:
        histogram.append((int(bucket + bucket_width), stats_weekly_totals_per_user.select("COUNT(*)", where="%d < %s AND %s <= %d" % (int(bucket), var_name, var_name, int(bucket + bucket_width))).tovalue()))	
        bucket += bucket_width
        
    return histogram
    
def create_table_num_stats_reports_vs_days_after_activation(stats_raw):
    users_min_max_dates = stats_raw.select("user_id, MAX(collected_date) AS max_date, MIN(collected_date) AS min_date", where='1 GROUP BY user_id')
    
    n = stats_raw.select("MAX(user_range)", 
                          from_str="(SELECT julianday(max_date) - julianday(min_date) AS user_range FROM table%d GROUP BY user_id)" % users_min_max_dates.table_id).tovalue()
    table = []
    for i in xrange(int(n) + 1):
        print "querying for num stats reports %d days after activation" % i
        num_stats_reports_i_days_later = stats_raw.select("COUNT(DISTINCT user_id)", 
                                                           where="julianday(collected_date) = %d + julianday((SELECT t2.min_date FROM table%d t2 WHERE t2.user_id = t.user_id))" % (i, users_min_max_dates.table_id)).tovalue()
        table.append((i, num_stats_reports_i_days_later))
    return table


def create_barchart_table(var_name, stats_raw):
    return stats_raw.select("%s, COUNT(DISTINCT user_id)" % var_name, where="1 GROUP BY %s" % var_name).totuplelist()
    
def create_vidrenderer_barchart_table(stats_raw):
    categories = ["MOBILITY RADEON", "GeForce", "ATI Mobility", "ATI", "Intel", "GDI Generic", "Quadro FX", "Radeon", "Sapphire Radeon", "Quadro NVS"]
    results = []
    data = stats_raw.select("user_id, d_hardware_vidRenderer", where="1 GROUP BY user_id").totuplelist()
    for category in categories:
        count = len(data)
        data = filter(lambda x: not x[1].lower().startswith(category.lower()), data)
        count = count - len(data)
        
        results.append((category + "*", count))
        
    for datum in data:
        results.append((datum[1], 1))
    return results

class HTMLStatsPage:
    def __init__(self, filename, spreadsheet_url):
        self._text_jsInitFunction = []
        self._text_jsFunctions = []
        self._text_body = []
        self._text_header = []
        self.spreadsheet_url = spreadsheet_url
        self.filename = filename
        
    def addToJsInitFunction(self, code):
        self._text_jsInitFunction.append(code)
        
    def addToJsFunctions(self, code):
        self._text_jsFunctions.append(code)
        
    def addToBody(self, code):
        self._text_body.append(code)
        
    def addToHeader(self, code):
        self._text_header.append(code)
        
    def writeToFile(self):
        try:
            os.chdir('../html/stats')
        except WindowsError:
            pass
            
        f=open(self.filename, 'w')
        f.write("""
<html>
  <head>
    <script type="text/javascript" src="http://www.google.com/jsapi"></script>
    <script type="text/javascript">
    
      var googleLibraryLoaded = false;
      var googleDocsIFrameLoaded = false;
      var googleDocsForDomainIFrameLoaded = false;
      
      function sleep(naptime){
          naptime = naptime * 1000;
          var sleeping = true;
          var now = new Date();
          var alarm;
          var startingMSeconds = now.getTime();
          while(sleeping){
             alarm = new Date();
             alarmMSeconds = alarm.getTime();
             if(alarmMSeconds - startingMSeconds > naptime){ sleeping = false; }
          }
      }
      google.load("visualization", "1", {packages:['annotatedtimeline', 'columnchart', 'barchart', 'areachart']});
      google.setOnLoadCallback(onGoogleVizLibLoadCallback); // Set callback to run when API is loaded
      
      function onGoogleVizLibLoadCallback() {
        googleLibraryLoaded = true;
        loadTable();
      }
      
      function onGoogleDocsIFrameLoadCallback()
      {
        googleDocsIFrameLoaded = true;
        loadTable();
      }
      
      function onGoogleDocsForDomainIFrameLoadCallback()
      {
        googleDocsForDomainIFrameLoaded = true;
        loadTable();
      }
      
        function loadTable() {
            if (googleDocsIFrameLoaded && googleDocsForDomainIFrameLoaded && googleLibraryLoaded)
            {
                var query = new google.visualization.Query('%s');
                query.send(verifyLoggedOn); 
            }
        }
        
        function verifyLoggedOn(queryResponse) {
            if (queryResponse.isError())
            {
                alert(queryResponse.getDetailedMessage());
            }
            else
            {
                // we've read the spreadsheet successfully, so we must be logged in
                loadTables();
            }

            
        }
        
        function loadTables() {
        """ % self.spreadsheet_url)
        for line in self._text_jsInitFunction:
            f.write(line)
            f.write('\n')
            
        f.write("}")
        
        for line in self._text_jsFunctions:
            f.write(line)
            f.write('\n')
            
        f.write("</script>\n")
        
        for line in self._text_header:
            f.write(line)
            f.write('\n')
            
        f.write("</head>\n");
        f.write("<body>\n");
        
        for line in self._text_body:
            f.write(line)
            f.write('\n')
            
        f.write("""	  
</body>
</html>
""")    
        f.close()
    
class TimeSeries(object):
    def __init__(self, stats, var_names, stats_html_page, human_readable_names, id_number):
        self.stats = stats
        self.var_names = var_names
        self.stats_html_page = stats_html_page
        self.human_readable_names = human_readable_names
        self.id_number = id_number
        self.include_annotations = True
        
    def emit(self):
        stats_daily_avg_per_stat_schema = self.stats['stats_daily_avg_per_stat'].schema
        
        self.stats_html_page.addToBody("<hr><h2>")
        if all([var_name in stats_daily_avg_per_stat_schema for var_name in self.var_names]):
            alphabet = [chr(ord('A') + x) for x in xrange(26)]
            # create spreadsheet style column names, ie. A, B, ..., Y, Z, AA, AB, AC, ..., AZ, BA, BB, ...
            col_letters = [ ([''] + alphabet)[x / 26] + alphabet[x % 26] for x in xrange(len(stats_daily_avg_per_stat_schema))]

            var_name_to_col_name = dict(zip(stats_daily_avg_per_stat_schema, col_letters))


            self.stats_html_page.addToJsInitFunction("var query = new google.visualization.Query('%s');" % self.stats_html_page.spreadsheet_url)
            letter_col_names = [var_name_to_col_name[_c] for _c in self.var_names]
            if self.include_annotations: letter_col_names += [var_name_to_col_name['release']]
            self.stats_html_page.addToJsInitFunction("query.setQuery('select A, %s');" % ', '.join(letter_col_names))
            self.stats_html_page.addToJsInitFunction("query.send(drawTimeSeriesChart_%s);" % self.id_number)
            self.stats_html_page.addToJsInitFunction("sleep(0.1)")
            
            self.stats_html_page.addToJsFunctions("""
            function drawTimeSeriesChart_%s(queryResponse) {
              
                if (queryResponse.isError())
                {
                    alert('Error in query: ' + response.getMessage() + ' ' + response.getDetailedMessage());
                }
                else
                {
                    var data = queryResponse.getDataTable();
                    
                    %s
                }
              }
            """ % (self.id_number, self.drawCommand()))
            for var in self.var_names:
                self.stats_html_page.addToBody(self.human_readable_names[var])
                try:
                    self.stats_html_page.addToBody(" (total: %s)<br>" % self.stats['stats_global_totals_per_stat'][var])
                except KeyError:
                    pass
        elif all([var_name in ['num_users_invited', 'num_users_activated'] for var_name in self.var_names]):
            tuplelist_sent = self.stats['invite_stats']["invites_sent"]
            tuplelist_authorized = self.stats['invite_stats']["invites_authorized"]
            data = []
            total_invites_sent = 0
            total_invites_authorized = 0
            for i, (tuple_sent, tuple_authorized) in enumerate(zip(tuplelist_sent, tuplelist_authorized)):
                assert tuple_sent[0] == tuple_authorized[0] # make sure the dates are the same
                total_invites_sent += int(tuple_sent[1])
                total_invites_authorized += int(tuple_authorized[1])
                year, month, day = [int(x) for x in tuple_sent[0].split('-')]
                data.append("\tdata.setValue(%d, %d, new Date(%d, %d, %d));" % (i, 0, year, month-1, day))
                data.append("\tdata.setValue(%d, %d, %f);" % (i, 1, total_invites_sent))
                release = ''
                for release_name, release_date,  in self.stats['release_dates']:
                    if release_date == tuple_sent[0]: release = release_name
                data.append('\tdata.setValue(%d, %d, "%s");' % (i, 2, release))
                data.append("\tdata.setValue(%d, %d, %f);" % (i, 3, total_invites_authorized))

            self.stats_html_page.addToJsInitFunction("drawTimeSeriesChart_%s();" % self.id_number)
            self.stats_html_page.addToJsFunctions("""
            function drawTimeSeriesChart_%s() {
                var data = new google.visualization.DataTable();
                data.addColumn('date', 'date');
                data.addColumn('number', 'invites_sent');
                data.addColumn('string', 'release');
                data.addColumn('number', 'invites_authorized');
                data.addRows(%d);
                %s
                
                var visualizationObject  = new google.visualization.AnnotatedTimeLine(document.getElementById('timeseries_chart_%s_div'));
                visualizationObject.draw(data, {displayAnnotations: true});


            }
            """% (self.id_number, len(tuplelist_sent), '\n'.join(data), self.id_number))
            for var in self.var_names:
                self.stats_html_page.addToBody("%s<br>" % self.human_readable_names[var])
        else:
            raise "unhandled variable"

        self.stats_html_page.addToBody("""
        </h2>
        <div id="timeseries_chart_%s_div" style="height: 300px; width: 1500px;"></div>
        """ % self.id_number)
        
    def drawCommand(self):
        return """
        var visualizationObject  = new google.visualization.AnnotatedTimeLine(document.getElementById('timeseries_chart_%s_div'));
        visualizationObject.draw(data, {displayAnnotations: true});""" % self.id_number
        
class StackedArea(TimeSeries):

    def __init__(self, stats, var_names, stats_html_page, human_readable_names, id_number):
        super(StackedArea, self).__init__(stats, var_names, stats_html_page, human_readable_names, id_number)
        self.include_annotations = False
    def drawCommand(self):
        return """
        var visualizationObject  = new google.visualization.AreaChart(document.getElementById('timeseries_chart_%s_div'));
        visualizationObject.draw(data, {isStacked: true});""" % self.id_number

        
    

class Histogram:
    def __init__(self, stats, var_name, stats_html_page, human_readable_names, id_number, time_range_end = None, time_range_num_days = None):
        self.stats = stats
        self.var_name = var_name
        self.stats_html_page = stats_html_page
        self.human_readable_names = human_readable_names
        self.id_number = id_number
        self.time_range_end = time_range_end
        self.time_range_num_days = time_range_num_days
        
    def emit(self):
        if self.var_name == 'num_stats_reports_vs_days_after_activation':
            histogram = create_table_num_stats_reports_vs_days_after_activation(self.stats['stats_raw'])
        else:
            histogram = create_histogram_table(self.var_name, self.stats['stats_weekly_totals_per_user'])
        
        # convert it to javascript/html
        data = []
        for i, tuple in enumerate(histogram):
            data.append("\tdata.setValue(%d, %d, '%s');" % (i, 0, tuple[0]))
            data.append("\tdata.setValue(%d, %d, %f);" % (i, 1, tuple[1]))

        self.stats_html_page.addToJsInitFunction("drawHistogramChart_%s();\n" % self.id_number)
        self.stats_html_page.addToJsFunctions("""
        function drawHistogramChart_%s() {
            var data = new google.visualization.DataTable();
            data.addColumn('string', '%s');
            data.addColumn('number', 'number of users');
            data.addRows(%d);
            %s
            
            var chart = new google.visualization.ColumnChart(document.getElementById('histogram_chart_%s_div'));
            chart.draw(data, {width: 1300, height: 300, is3D: false});
        }
        """% (self.id_number, self.var_name, len(histogram), '\n'.join(data), self.id_number))

        if self.var_name == 'num_stats_reports_vs_days_after_activation':
            title = self.human_readable_names[self.var_name]
        else:
            title = "%s"  % self.human_readable_names[self.var_name]
            if self.time_range_end:
                title += " (users binned by summed usage per user from %s to %s)" % (self.time_range_end - timedelta(self.time_range_num_days), self.time_range_end)
            
        self.stats_html_page.addToBody("""<hr>
        <h2>%s</h2>
        <div id="histogram_chart_%s_div" style="height: 300px; width: 1300px;"></div>
        """ % (title, self.id_number))

class Barchart:
    def __init__(self, stats, var_name, stats_html_page, human_readable_names, id_number):
        self.stats = stats
        self.var_name = var_name
        self.stats_html_page = stats_html_page
        self.human_readable_names = human_readable_names
        self.id_number = id_number
        
    def emit(self):
        if self.var_name == 'd_hardware_vidRenderer':
            barchart_table = create_vidrenderer_barchart_table(self.stats['stats_raw'])
        else:
            barchart_table = create_barchart_table(self.var_name, self.stats['stats_raw'])
            
        data = []
        for i, tuple in enumerate(barchart_table):
            data.append("\tdata.setValue(%d, %d, '%s');" % (i, 0, tuple[0]))
            data.append("\tdata.setValue(%d, %d, %f);" % (i, 1, tuple[1]))
        
        self.stats_html_page.addToJsInitFunction("drawBarChart_%s();\n" % self.id_number)
        self.stats_html_page.addToJsFunctions("""
        function drawBarChart_%s() {
            var data = new google.visualization.DataTable();
            data.addColumn('string', '%s');
            data.addColumn('number', 'number of users');
            data.addRows(%d);
            %s
            
            var chart = new google.visualization.BarChart(document.getElementById('bar_chart_%s_div'));
            chart.draw(data, {width: 1000, height: 300, is3D: false});
        }
        """% (self.id_number, self.var_name, len(barchart_table), '\n'.join(data), self.id_number))
        
        self.stats_html_page.addToBody("""
        <hr>
        <h2>%s</h2>
        <div id="bar_chart_%s_div" style="height: 300px; width: 1000px;"></div>
        """ % (self.human_readable_names[self.var_name], self.id_number))
        
