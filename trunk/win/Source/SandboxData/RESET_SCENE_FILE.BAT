REM ###
REM ###
REM ###
REM ###
REM ###
REM ###TIMESTAMP CODE (from http://jgiors.wordpress.com/example-timestamp-batch-file/)
REM ###
REM ###
REM ###
REM ###
REM ###

for /f "tokens=1-4 delims=/ " %%i in ("%date%") do (
  set dow=%%i
  set month=%%j
  set day=%%k
  set year=%%l
)

for /f "tokens=1-4 delims=.: " %%i in ("%time%") do (
 set hour=%%i
 set minute=%%j
 set second=%%k
 set hundredth=%%l
)

if %hour% LSS 10 set hour=0%hour%

REM ###NOTE: Due to execution time, the following time
REM ###may vary a little from the previous.

REM ###-
REM ###Sample usages:
REM ###-

REM ###Today is %dow%
REM ###%year%-%month%-%day% %time%
REM ###%year%%month%%day%T%hour%%minute%%second%.%hundredth%
REM ###File_%year%-%month%-%day%_%hour%h_%minute%m_%second%s.txt
REM ###File____%year%-%month%-%day%____%time%.txt


REM ###
REM ###
REM ###Backup your scene file (a convoluted method that avoids prompts)
REM ###
REM ###

DEL /ah "%TEMP%\scene.bump"
XCOPY /h "scene.bump" "%TEMP%"
ATTRIB -h "%TEMP%\scene.bump"
RENAME "%TEMP%\scene.bump" scene%month%-%day%___%hour%.%minute%.%second%.bump
ATTRIB +h "%TEMP%\scene%month%-%day%___%hour%.%minute%.%second%.bump"
XCOPY /h "%TEMP%\scene%month%-%day%___%hour%.%minute%.%second%.bump" "%CD%\"
DEL /ah "%TEMP%\scene%month%-%day%___%hour%.%minute%.%second%.bump"

REM ###
REM ###
REM ###Copy scene.bump.ORIGINAL file to scene.bump
REM ###
REM ###
DEL /ah "%CD%\scene.bump"
DEL /ah "%TEMP%\scene.bump.ORIGINAL"
XCOPY /h "scene.bump.ORIGINAL" "%TEMP%"
ATTRIB -h "%TEMP%\scene.bump.ORIGINAL"
RENAME "%TEMP%\scene.bump.ORIGINAL" scene.bump
ATTRIB +h "%TEMP%\scene.bump"
XCOPY /h "%TEMP%\scene.bump" "%CD%\"
DEL /ah "%TEMP%\scene.bump"
