REM wxrc -c Settings.xrc -o XRC_Settings.cpp
cd "%1"
wxrc -c Settings.xrc -o XRC_Settings_TMP.cpp
echo #include "BT_Common.h" > XRC_Settings.cpp
type XRC_Settings_TMP.cpp >> XRC_Settings.cpp