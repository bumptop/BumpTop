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
import subprocess

from create_stats_graphs_helpers import *


get_invite_stats = True
should_sync_stats_files_from_lunar_pages = False
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
CustomHistogram : num_days_from_first_to_last_stat_report
CustomHistogram : num_stats_reports_vs_days_after_activation
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
TimeSeries : d_bt_interaction_actors_fs_types_documents d_bt_interaction_actors_fs_types_executables d_bt_interaction_actors_fs_types_folders d_bt_interaction_actors_fs_types_images
Histogram : d_bt_interaction_actors_fs_types_documents
Histogram : d_bt_interaction_actors_fs_types_executables
Histogram : d_bt_interaction_actors_fs_types_folders
Histogram : d_bt_interaction_actors_fs_types_images
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
    
#if hwmanfmode:
#    sync_stats_locally('toshiba')
#else:
#    sync_stats_locally('')

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

stats_raw_table = parsed_stats_files_to_stats_raw_table(parsed_stat_files, bumptop_vars, bumptop_vars_sql_types)


######
####### Filtering out bad data
#######

for filter_fn in ['all_times_under_24_hours',
                 'times_not_negative',
                 'submitted_after_july_2008',
                 'uploaded_less_than_one_week_after_collected']:
    l = len(stats_raw_table.tuplelist)
    stats_raw_table.tuplelist = filter(eval(filter_fn)(stats_raw_table.schema), stats_raw_table.tuplelist)
    print "Filtered out %d rows because failed test: %s " % (l - len(stats_raw_table.tuplelist), filter_fn)
    


######
###### Create SQL table
######

if hwmanfmode:
	_db_name = 'statshwmanfmode.sqlite3'
else:
	_db_name = 'stats.sqlite3'
	
if os.path.exists(_db_name): os.remove(_db_name)
	
are_types_numeric = []
for i, var in enumerate(stats_raw_table.schema):
    if bumptop_vars_are_numeric.has_key(var):
        are_types_numeric.append(bumptop_vars_are_numeric[var])
    else:
        are_types_numeric.append(isnumeric(stats_raw_table.tuplelist[i]))


stats_raw = InMemorySQLTable.fromtuplelist(stats_raw_table.tuplelist, stats_raw_table.schema, db_name = _db_name, are_schema_columns_numeric = are_types_numeric)
stats_raw.select("release")
    

print "length of stats_raw_table: ", len(stats_raw_table.tuplelist)


candidates = stats_raw.select("count(*) as the_count, dir_hash, collected_date, d_hardware_osBuildNumber, d_hardware_cpuNumProcessors, d_hardware_cpuFeatures", 
				from_str="(SELECT DISTINCT user_id, dir_hash, collected_date, d_hardware_osBuildNumber, d_hardware_cpuNumProcessors, d_hardware_cpuFeatures FROM #TABLE# ) GROUP BY dir_hash, collected_date, d_hardware_osBuildNumber, d_hardware_cpuNumProcessors,d_hardware_cpuFeatures ORDER BY the_count DESC limit 190;")


for cand in candidates.totuplelist():
	num_occurences = cand[0]
	dir_hash = cand[1]
	collected_date = cand[2]
	
	print "|".join([str(x) for x in cand])
	if num_occurences > 1:

		
		files_to_delete = stats_raw.select("user_id, dir_hash, collected_date", 
									where="""dir_hash = '%s' AND 
											 collected_date = '%s' AND 
											 d_bt_window_activeTime != (SELECT MAX(d_bt_window_activeTime) FROM #TABLE# WHERE dir_hash = '%s' AND collected_date = '%s')""" % (dir_hash, collected_date, dir_hash, collected_date))
		
		for file_to_delete in files_to_delete.totuplelist():
			user_id = file_to_delete[0]
			collected_date = str(collected_date).replace('-', '_').replace('_0','_')
			print 'find %s -name "stats_%s_%s_%s*" -exec rm {} \\;' % (('stats', 'stats/toshiba')[hwmanfmode], user_id, dir_hash, collected_date)
			subprocess.call('find %s -name "stats_%s_%s_%s*" -exec rm {} \\;' % (('stats', 'stats/toshiba')[hwmanfmode], user_id, dir_hash, collected_date), shell=True)



