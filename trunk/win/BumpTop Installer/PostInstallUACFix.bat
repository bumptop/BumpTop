set WiRunSql=%1
set BuiltOutputPath=%2

echo Updating the MSI file...
echo Suppressing reboot...
echo cscript //nologo %WiRunSql% %BuiltOutputPath% "INSERT INTO Property (Property, Value) VALUES ('REBOOT', 'ReallySuppress')"
cscript //nologo %WiRunSql% %BuiltOutputPath% "INSERT INTO Property (Property, Value) VALUES ('REBOOT', 'ReallySuppress')"

REM echo Making custom actions asynchronous
REM echo cscript //nologo %WiRunSql% %BuiltOutputPath% "UPDATE CustomAction SET CustomAction.Type=1234 WHERE CustomAction.Type=1042"
REM cscript //nologo %WiRunSql% %BuiltOutputPath% "UPDATE CustomAction SET CustomAction.Type=1234 WHERE CustomAction.Type=1042"