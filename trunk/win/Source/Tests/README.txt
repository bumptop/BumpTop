From: Steffan Chartrand [mailto:steffan@bumptop.com] 
Sent: October-22-09 2:24 PM
To: Justin Ho
Subject: Testing Options

Run the tests by starting BumpTop with the "-automatedJSONTesting" option.
Enables tests to run by parsing JSON file. The tests themselves are defined in BT_AutomatedJSONTesting.cpp.
Skips animations
Skips dialog box pop ups for existing tests

Default directories
JSON test script: Source\Tests\sampleTestScript.json
Log file output: Source\Tests\test_log.txt
Testing environement: Source\Tests\environment

To run a custom set of tests, create a file named customTests.json in the JSON test script dir.

If the -automatedJSONTesting flag is set, the following command line arguments are valid:
1) "-skipRendering"
2) -JSONTestScriptDir <dir>
3) -JSONLogFileDir <dir>
4) -JSONTestFilesDir <dir>

Currently the test_log.txt file is updated by appending test results to the end of the file. In order to see if all tests passed, we would need to check the date and time when the tests are run and check to make sure that the string "0 FAILED" exists.