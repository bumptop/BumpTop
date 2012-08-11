@echo off
echo Processing Source...
..\Source\bin\lupdate.exe ..\Source\BumpTop.pro
echo Processing Settings...
pushd "..\BumpTop Settings\project"
python extractSettingsStrings.py 
popd
pushd ..\Source\bin
python processHtmlSources.py
python processJavaScriptSources.py
python mergeWebStringsIntoTS.py
popd
echo --
echo Generated translation files:
echo   ..\BumpTop Settings\project\Settings_^<lang^>.ts
echo   ..\Source\Languages\BumpTop_^<lang^>.ts
echo   ..\Source\Languages\BumpTop_Web_en.ts
