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
import sys

from create_stats_graphs_helpers import *


should_get_invite_stats = True
should_sync_stats_files_from_lunar_pages = True
upload_to_google_spreadsheets = True
   
   


bumptop_vars_and_names =	"""
d_bt_build, FLOAT, BumpTop build
d_bt_launchOnStartup, FLOAT, % of users launching on startup
d_bt_interaction_actors_addedToPile, FLOAT, # of times actors were added to pile
d_bt_interaction_actors_fs_types_documents, FLOAT, # of documents
d_bt_interaction_actors_fs_types_executables, FLOAT, # of executables
d_bt_interaction_actors_fs_types_folders, FLOAT, # of folders
d_bt_interaction_actors_fs_types_images, FLOAT, # of images
d_bt_interaction_actors_highlightedImage, FLOAT, # of times actors were zoomed in on
d_bt_interaction_actors_launchedFile, FLOAT, # of time actors were launched
d_bt_interaction_actors_pinned, FLOAT, # of times actors were pinned
d_bt_interaction_actors_removedFromPile, FLOAT, # of times actors were removed from pile
d_bt_interaction_actors_unpinned, FLOAT, # of times actors were unpinned
d_bt_interaction_dragAndDrop_fromExplorer, FLOAT, # of drag/drops from explorer
d_bt_interaction_dragAndDrop_toActor, FLOAT, # of drag/drops to actor
d_bt_interaction_dragAndDrop_toExplorer, FLOAT, # of drag/drops from explorer
d_bt_interaction_dragAndDrop_toPile, FLOAT, # of drag/drops to pile
d_bt_interaction_markingmenu_executedCommandByClick, FLOAT, Marking menu: # commands executed by click
d_bt_interaction_markingmenu_executedCommandByHotkey, FLOAT, Marking menu: # commands executed by hotkey
d_bt_interaction_markingmenu_invokedByClick, FLOAT, Marking menu: # times invoked by click
d_bt_interaction_markingmenu_invokedByLasso, FLOAT, Marking menu: # commands invoked by lasso
d_bt_interaction_piles_created, FLOAT, # of times piles created
d_bt_interaction_piles_destroyed, FLOAT, # of times piles destroyed
d_bt_interaction_piles_fanoutView, FLOAT, # of times piles viewed in fanout
d_bt_interaction_piles_folderized, FLOAT, # of times piles folderized
d_bt_interaction_piles_gridView, FLOAT, # of times piles viewed in gridview
d_bt_interaction_piles_pilized, FLOAT, # of times piles "pilized" (?)
d_bt_resetLayout, FLOAT, # of reset layouts
d_bt_shellExt_foldersBumped, FLOAT, shellext: # of folders bumped
d_bt_shellExt_foldersUnbumped, FLOAT, shellext: # of folders unbumped
d_bt_window_activatedFromClick, FLOAT, window: # of activations by click
d_bt_window_activatedFromNonClick, FLOAT, window: # of activations by non-click
d_bt_window_activatedFromDoubleClick, FLOAT, window: # of activations from double click
d_bt_window_activatedFromSystemTray, FLOAT, window: # of activations from system tray
d_bt_window_activatedFromShowDesktop, FLOAT, window: # of activations via Show Desktop
d_bt_window_activeTime, FLOAT, active time (min)
d_bt_window_focusedTime, FLOAT, focused time (min)
d_bt_window_instantiations, FLOAT, # of instantiations
d_bt_window_interactiveTime, FLOAT, interactive time (min)
d_bt_window_slideshowTime, FLOAT, slideshow time (min)
d_bt_workingDirectory, TEXT, working directory
d_hardware_cpuArch, TEXT, cpuArch
d_hardware_cpuFeatures, TEXT, cpuFeatures
d_hardware_cpuNameString, TEXT, cpuNameString
d_hardware_cpuNumProcessors, FLOAT, cpuNumProcessors
d_hardware_cpuSpeed, FLOAT, cpuSpeed
d_hardware_cpuType, TEXT, cpuType, EnumTimeSeries
d_hardware_osBuildNumber, TEXT, osBuildNumber
d_hardware_osServicePack, TEXT, osServicePack
d_hardware_osSrvPckMajorVer, FLOAT, osSrvPckMajorVer
d_hardware_osSrvPckMinorVer, FLOAT, osSrvPckMinorVer
d_hardware_osVersion, FLOAT, osVersion
d_hardware_ramPhysicalAvailable, FLOAT, ramPhysicalAvailable (MB)
d_hardware_ramPhysicalTotal, FLOAT, ramPhysicalTotal (MB)
d_hardware_ramVirtualAvailable, FLOAT, ramVirtualAvailable (MB)
d_hardware_ramVirtualTotal, FLOAT, ramVirtualTotal (MB)
d_hardware_vidRenderer, TEXT, vidRenderer
d_bt_interaction_clipboard_copy, FLOAT, # of copies
d_bt_interaction_clipboard_paste, FLOAT, # of pastes
d_bt_interaction_clipboard_cut, FLOAT, # of cuts
d_bt_interaction_actors_custom_printer_tossedTo, FLOAT, # of times tossed to printer
d_bt_interaction_actors_custom_printer_launched, FLOAT, # of times printer launched (?)
d_bt_interaction_actors_custom_printer_enabled, FLOAT, printer enabled?
d_bt_interaction_actors_custom_printer_droppedOn, FLOAT, # of times dropped on printer
d_bt_interaction_actors_custom_email_tossedTo, FLOAT, # of times tossed to email
d_bt_interaction_actors_custom_email_launched, FLOAT, # of times email launched (?)
d_bt_interaction_actors_custom_email_enabled, FLOAT, email enabled?
d_bt_interaction_actors_custom_email_droppedOn, FLOAT, # of times dropped on email
d_bt_interaction_layout_laidOutInGrid, FLOAT, # of times laid out in grid
d_bt_interaction_piles_pileByType, FLOAT, # of times piled
d_bt_interaction_piles_leafed, FLOAT, # of times piles leafed
d_hardware_isTabletPC, FLOAT, Is Tablet PC?
d_bt_interaction_piles_gridScrolled, FLOAT, # of times grid scrolled
d_bt_crashes, FLOAT, # of crashes
d_bt_interaction_search_searchSubstring, FLOAT, # of times search substring
d_bt_interaction_search_findAsYouType, FLOAT, # of times find as you type
d_bt_performance_maxAverageFps, FLOAT, max avg FPS
d_bt_performance_averageFps, FLOAT, avg FPS
d_bt_theme_name, TEXT, theme name
d_bt_theme_schema, TEXT, theme schema
d_bt_theme_version, TEXT, theme version
d_bt_theme_url, TEXT, theme url
d_bt_hwManParticipantID, FLOAT, participant id
d_bt_settings_instantiations, FLOAT, # of settings instantiations
d_bt_interaction_actors_fs_types_postitnotes, FLOAT, # of postit notes
d_bt_interaction_actors_fs_types_postitnotesCreated, FLOAT, # of postit notes created
d_bt_theme_timesChanged, FLOAT, # of times theme changed
"""


charts = """
TimeSeries : num_users
TimeSeries : num_users_invited num_users_activated
CustomTimeSeries : num_users_in_last_week
TimeSeries : d_bt_window_slideshowTime d_bt_window_activeTime d_bt_window_focusedTime d_bt_window_interactiveTime
Histogram : d_bt_window_focusedTime
Histogram : d_bt_window_interactiveTime
Histogram : d_bt_window_activeTime
Histogram : d_bt_window_slideshowTime
CustomHistogram : num_stats_reports
TimeSeries : d_bt_crashes
Histogram : d_bt_crashes
TimeSeries : d_bt_performance_averageFps d_bt_performance_maxAverageFps
TimeSeries : d_bt_interaction_actors_highlightedImage d_bt_interaction_actors_launchedFile 
Histogram : d_bt_interaction_actors_highlightedImage
Histogram : d_bt_interaction_actors_launchedFile 
TimeSeries : d_bt_interaction_actors_addedToPile d_bt_interaction_actors_removedFromPile
Histogram : d_bt_interaction_actors_addedToPile
Histogram : d_bt_interaction_actors_removedFromPile
TimeSeries : d_bt_interaction_actors_pinned d_bt_interaction_actors_unpinned
Histogram : d_bt_interaction_actors_pinned
Histogram : d_bt_interaction_actors_unpinned
TimeSeries : d_bt_interaction_actors_fs_types_documents d_bt_interaction_actors_fs_types_executables d_bt_interaction_actors_fs_types_folders d_bt_interaction_actors_fs_types_images d_bt_interaction_actors_fs_types_postitnotes
Histogram : d_bt_interaction_actors_fs_types_documents
Histogram : d_bt_interaction_actors_fs_types_executables
Histogram : d_bt_interaction_actors_fs_types_folders
Histogram : d_bt_interaction_actors_fs_types_images
Histogram : d_bt_interaction_actors_fs_types_postitnotes
TimeSeries : d_bt_interaction_actors_fs_types_postitnotesCreated
TimeSeries : d_bt_interaction_dragAndDrop_fromExplorer d_bt_interaction_dragAndDrop_toExplorer d_bt_interaction_dragAndDrop_toActor d_bt_interaction_dragAndDrop_toPile
Histogram : d_bt_interaction_dragAndDrop_fromExplorer 
Histogram : d_bt_interaction_dragAndDrop_toExplorer 
Histogram : d_bt_interaction_dragAndDrop_toActor
Histogram : d_bt_interaction_dragAndDrop_toPile
TimeSeries : d_bt_interaction_markingmenu_executedCommandByClick d_bt_interaction_markingmenu_executedCommandByHotkey
Histogram : d_bt_interaction_markingmenu_executedCommandByClick 
Histogram : d_bt_interaction_markingmenu_executedCommandByHotkey
TimeSeries : d_bt_interaction_markingmenu_invokedByClick d_bt_interaction_markingmenu_invokedByLasso
Histogram : d_bt_interaction_markingmenu_invokedByClick
Histogram : d_bt_interaction_markingmenu_invokedByLasso
TimeSeries : d_bt_interaction_piles_created d_bt_interaction_piles_destroyed
Histogram : d_bt_interaction_piles_created 
Histogram : d_bt_interaction_piles_destroyed
TimeSeries : d_bt_interaction_piles_folderized d_bt_interaction_piles_pilized
Histogram : d_bt_interaction_piles_folderized 
Histogram : d_bt_interaction_piles_pilized
TimeSeries : d_bt_interaction_piles_fanoutView d_bt_interaction_piles_gridView d_bt_interaction_piles_leafed
Histogram : d_bt_interaction_piles_fanoutView
Histogram : d_bt_interaction_piles_gridView
Histogram : d_bt_interaction_piles_leafed
TimeSeries : d_bt_interaction_search_searchSubstring d_bt_interaction_search_findAsYouType
Histogram : d_bt_interaction_search_searchSubstring
Histogram : d_bt_interaction_search_findAsYouType
TimeSeries : d_bt_resetLayout 
Histogram : d_bt_resetLayout 
TimeSeries : d_bt_interaction_piles_pileByType
Histogram : d_bt_interaction_piles_pileByType
TimeSeries : d_bt_window_instantiations
Histogram : d_bt_window_instantiations
TimeSeries : d_bt_shellExt_foldersBumped d_bt_shellExt_foldersUnbumped
Histogram : d_bt_shellExt_foldersBumped 
Histogram : d_bt_shellExt_foldersUnbumped
StackedArea : d_bt_window_activatedFromClick d_bt_window_activatedFromNonClick
TimeSeries : d_bt_window_activatedFromDoubleClick d_bt_window_activatedFromSystemTray d_bt_window_activatedFromShowDesktop
Histogram : d_bt_window_activatedFromDoubleClick 
Histogram : d_bt_window_activatedFromSystemTray
TimeSeries : d_bt_interaction_clipboard_copy d_bt_interaction_clipboard_paste d_bt_interaction_clipboard_cut
Histogram : d_bt_interaction_clipboard_copy
Histogram : d_bt_interaction_clipboard_paste
Histogram : d_bt_interaction_clipboard_cut
TimeSeries : d_bt_interaction_actors_custom_printer_tossedTo d_bt_interaction_actors_custom_printer_launched d_bt_interaction_actors_custom_printer_enabled d_bt_interaction_actors_custom_printer_droppedOn  
Histogram : d_bt_interaction_actors_custom_printer_tossedTo 
Histogram : d_bt_interaction_actors_custom_printer_launched 
Histogram : d_bt_interaction_actors_custom_printer_enabled
Histogram : d_bt_interaction_actors_custom_printer_droppedOn  
TimeSeries : d_bt_interaction_actors_custom_email_tossedTo d_bt_interaction_actors_custom_email_launched d_bt_interaction_actors_custom_email_enabled d_bt_interaction_actors_custom_email_droppedOn  
Histogram : d_bt_interaction_actors_custom_email_tossedTo 
Histogram : d_bt_interaction_actors_custom_email_launched
Histogram : d_bt_interaction_actors_custom_email_enabled
Histogram : d_bt_interaction_actors_custom_email_droppedOn
TimeSeries : d_bt_interaction_layout_laidOutInGrid
Histogram : d_bt_interaction_layout_laidOutInGrid
Barchart : d_bt_build
Barchart : d_hardware_cpuArch
Barchart : d_hardware_cpuFeatures
Barchart : d_hardware_cpuNameString
Barchart : d_hardware_cpuNumProcessors
Barchart : d_hardware_cpuSpeed
Barchart : d_hardware_cpuType
Barchart : d_hardware_osBuildNumber
Barchart : d_hardware_osServicePack
Barchart : d_hardware_osSrvPckMajorVer
Barchart : d_hardware_osSrvPckMinorVer
Barchart : d_hardware_osVersion
Barchart : d_hardware_ramPhysicalAvailable
Barchart : d_hardware_ramPhysicalTotal
Barchart : d_hardware_ramVirtualAvailable
Barchart : d_hardware_ramVirtualTotal
Barchart : d_hardware_vidRenderer
TimeSeries : d_hardware_isTabletPC
Barchart : d_bt_theme_name
Barchart : d_bt_theme_schema
Barchart : d_bt_theme_version
Barchart : d_bt_theme_url
Barchart : d_bt_hwManParticipantID
TimeSeries : d_bt_theme_timesChanged
Histogram : d_bt_theme_timesChanged
"""
charts = [line.strip().split(' : ') for line in charts.strip().split('\n')]

human_readable_names = bumptop_vars_and_names_to_human_readable_names(bumptop_vars_and_names)
human_readable_names['num_users'] = '# of users who submitted data'
human_readable_names['collected_date'] = 'collected_date'
human_readable_names['release'] = 'release'
human_readable_names['num_users_invited'] = '# of users invited'
human_readable_names['num_users_activated'] = '# of users who activated'

human_readable_names['num_stats_reports'] = '# of stats reports'
human_readable_names['num_days_from_first_to_last_stat_report'] = '# of days from first to last stat report'

human_readable_names['num_users_in_last_week'] = '# of unique users in the previous 7 day period'

human_readable_names['num_stats_reports_vs_days_after_activation'] = '# of stats reports vs. # of days after activation' 

bumptop_vars = [_line.split(', ')[0] for _line in bumptop_vars_and_names.strip().split('\n')]
bumptop_vars_sql_types = [_line.split(', ')[1] for _line in bumptop_vars_and_names.strip().split('\n')]

bumptop_vars_are_numeric = dict(zip(bumptop_vars, [_type != 'TEXT' for _type in bumptop_vars_sql_types]))



hwmanfmode = len(sys.argv) > 1 and sys.argv[1] == '-hwmanfmode'

if hwmanfmode: print 'hwmanfmode'
    
if hwmanfmode:
    sync_stats_locally('toshiba')
else:
    sync_stats_locally('')

### Update the list of stats files
if should_sync_stats_files_from_lunar_pages:
    try:
        if hwmanfmode:
            sync_stats_from_lunar_pages_hwmanfmode()
        else:
            sync_stats_from_lunar_pages()
    except WindowsError:
        pass
    
### Get a list of the stats files (in the stats directory)
   
if hwmanfmode:
    stats_files = get_list_of_stats_files('stats/toshiba/')
else:
    stats_files = get_list_of_stats_files('stats/')
print len(stats_files), "stat files"


### Parse the stat files
parsed_stat_files = [parse_stats_file(s) for s in stats_files]
parsed_stat_files = filter(lambda x: x != None, parsed_stat_files)
if hwmanfmode: parsed_stat_files = filter(lambda x: x.data.has_key('d_bt_hwManParticipantID') and x.data['d_bt_hwManParticipantID'] < 69.0, parsed_stat_files)

stats_raw_table = parsed_stats_files_to_stats_raw_table(parsed_stat_files, bumptop_vars, bumptop_vars_sql_types)


######
####### Filtering out bad data
#######

for filter_fn in ['all_times_under_24_hours',
                  'less_than_100_crashes_reported',
                  'times_not_negative',
                  'submitted_after_july_2008',
                  'uploaded_less_than_one_week_after_collected',
                  'collected_today_or_earlier']:
    l = len(stats_raw_table.tuplelist)
    stats_raw_table.tuplelist = filter(eval(filter_fn)(stats_raw_table.schema), stats_raw_table.tuplelist)
    print "Filtered out %d rows because failed test: %s " % (l - len(stats_raw_table.tuplelist), filter_fn)
    


######
###### Create SQL table
######

are_types_numeric = []
for i, var in enumerate(stats_raw_table.schema):
    if bumptop_vars_are_numeric.has_key(var):
        are_types_numeric.append(bumptop_vars_are_numeric[var])
    else:
        are_types_numeric.append(isnumeric(stats_raw_table.tuplelist[0][i]))

stats_raw = InMemorySQLTable.fromtuplelist(stats_raw_table.tuplelist, stats_raw_table.schema, are_schema_columns_numeric = are_types_numeric)
stats_raw.select("release")


###
### Filter out BumpTop employees
###
winson_machine = '{7B5DF2F6-7FF2-4004-AB3E-5B37A0152B7B}'
mike_machine = '{3BF21C63-D8CA-4EEC-B9B9-35F18570B67E}'
dell_machine = '{2A96F283-EB83-4DCD-9512-CBC626A0FEBA}'
anand_machine = '{35A4943B-217D-4476-9281-4AD86C78C6D7}'
henry_machine = '{F706A784-F7EE-40B9-9559-922E03EC2C74}'
henry_machine2 = '{556D7405-814F-4258-84F6-A630A20D2674}'


stats_raw.select_and_replace('*',where=user_id_is_not([winson_machine, mike_machine, dell_machine, anand_machine, henry_machine, henry_machine2]))
stats_raw.select("release")

distinct_users = int(stats_raw.select("COUNT(DISTINCT user_id)").tovalue())
num_days = int(stats_raw.select("COUNT(DISTINCT collected_date)").tovalue())

print distinct_users, "users over", num_days, "days"

###
### Collect info about the number of people that have installed / authorized BumpTop
###

if should_get_invite_stats:
    invite_stats = get_invite_stats()
    
time_series_vars = []
for chart in charts:
    if chart[0] in ['TimeSeries', 'StackedArea']:
        vars = chart[1].split()
        for var in vars:
            if chart[1].startswith('d_'):
                time_series_vars.append(var)

###
print "Create a table for the time-series charts"
###

release_dates = [('Beta 9', '2008-07-14'), ('Beta 10', '2008-07-29'), ('Beta 11', '2008-08-19'), ('Beta 12', '2008-09-04'), ('Beta 13', '2008-09-23'), ('Beta 14', '2008-10-16'), ('Beta 15', '2008-10-31'),('Beta 16', '2008-11-22'),('Beta 17', '2008-12-10')]

stats_daily_avg_per_stat = stats_raw_to_daily_avg(stats_raw, time_series_vars, release_dates)


stats_global_totals_per_stat = stats_raw_to_global_total(stats_raw, time_series_vars)
stats_global_totals_per_stat['num_users'] = distinct_users


if hwmanfmode:
    histogram_time_range_end = None
    histogram_time_range_num_days = 0
else:
    histogram_time_range_end = date.today()
    histogram_time_range_num_days = 7


histogram_vars = 		[chart[1] for chart in charts if chart[0] == 'Histogram']

stats_weekly_totals_per_user = stats_raw_to_weekly_totals_per_user(stats_raw, histogram_vars, histogram_time_range_end, histogram_time_range_num_days)

barchart_vars = [chart[1] for chart in charts if chart[0] == 'Barchart']
    
if hwmanfmode:
    html_stats_page = HTMLStatsPage('graphs_hwmanf.html', 'https://spreadsheets.google.com/a/bumptop.com/tq?key=p-mYzcIPOGBmjIg-TbZgiKA&gid=13')
else:
    html_stats_page = HTMLStatsPage('graphs.html', 'https://spreadsheets.google.com/a/bumptop.com/tq?key=p-mYzcIPOGBmjIg-TbZgiKA&gid=11')

html_stats_page.addToHeader("""
<title>BumpTop Stats</title>

<style type="text/css">
BODY
   {
   font-family:sans-serif;
   }
</style>
""")

#<img src ="http://bumptop.com/imgs/logos/BumpTop75ds.png" align ="left"> 
html_stats_page.addToBody("""
<img src ="bigbrother.jpg" align ="left"> 
<h1>Bump-a-Lytics%s</h1>
<small>BumpTop's Bad Ass Analytics.  We're watching you!</small>
<p>&nbsp;</p>
<h3>Launchpad</h3>
<ul>
<li><a href="http://bumptop.com/stats/screenshots">Screenshots!</a></li>
<li><a href="http://bumptop.crowdsound.com/">Feature Voting (CrowdSound)</a></li>
<li><a href="http://bumptop.com/forums">Forums (Get Satisfaction)</a></li>
<li><a href="http://flickr.com/groups/bumptops/">Public Gallery (Flickr)</a></li>
<li><a href="http://bumptop.com/download/?invite_code=e7ee58bb">Beta HQ</a></li>
<li><a href="http://bumptop.com/admin">Inviter</a></li>
</ul>

<IFRAME SRC="http://spreadsheets.google.com/ccc?key=p-mYzcIPOGBmjIg-TbZgiKA" WIDTH="0" HEIGHT="0" FRAMEBORDER="0" onload="onGoogleDocsIFrameLoadCallback();"></IFRAME>
<IFRAME SRC="http://spreadsheets.google.com/a/bumptop.com/ccc?key=p-mYzcIPOGBmjIg-TbZgiKA" WIDTH="0" HEIGHT="0" FRAMEBORDER="0" onload="onGoogleDocsForDomainIFrameLoadCallback();"></IFRAME>
<hr>
Last updated: %s<br>
""" % (["", " - hwmanf"][hwmanfmode], datetime.now()))

stats = {}
for var in ['stats_raw', 'stats_daily_avg_per_stat', 'stats_weekly_totals_per_user', 'stats_global_totals_per_stat', 'invite_stats', 'release_dates']:
    if var != 'invite_stats' or should_get_invite_stats:
        stats[var] = eval(var)

for i, (chart_type, var_names) in enumerate(charts):
    var_names = var_names.strip().split()
    if intersection(var_names, ['num_users_invited', 'num_users_activated']) and not should_get_invite_stats:
        continue
    if chart_type in ['TimeSeries', 'CustomTimeSeries']:
        chart = TimeSeries(stats, var_names, html_stats_page, human_readable_names, i)
        chart.emit()
    elif chart_type in ['Histogram', 'CustomHistogram']:
        chart = Histogram(stats, var_names[0], html_stats_page, human_readable_names, i, histogram_time_range_end, histogram_time_range_num_days)
        chart.emit()
    elif chart_type == 'Barchart':
        chart = Barchart(stats, var_names[0], html_stats_page, human_readable_names, i)
        chart.emit()
    elif chart_type == 'StackedArea':
        chart = StackedArea(stats, var_names, html_stats_page, human_readable_names, i)
        chart.emit()
        
html_stats_page.writeToFile()

###
### Upload the time-series data to Google spreadsheets
###

#################################

if upload_to_google_spreadsheets:

    user = 'feedback@bumptop.com'
    pw = 'lombardi'
    print "Connecting to Google Spreadsheets..."
    spreadsheet = SimpleCRUD(user, pw)
    print "Opening spreadsheet 'Usage Statistics'..."
    spreadsheet._OpenSpreadsheet('Usage Statistics')
    if hwmanfmode: 
        sheet_name = 'time series toshiba'
    else:
        sheet_name = 'time series'
    print "Updating sheet '%s'..." % sheet_name
    spreadsheet._UpdateWorksheetData(sheet_name, [human_readable_names[_t] for _t in stats_daily_avg_per_stat.schema], stats_daily_avg_per_stat.sqlitetable.totuplelist())
