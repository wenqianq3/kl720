start /wait "J-Link Commander" "JLink.exe" -settingsfile .\Sample.jlinksettings -CommanderScript .\flash_ncpu.jlink
IF ERRORLEVEL 1 goto ERROR
ECHO J-Flash Program : OK!
goto END

:ERROR
ECHO J-Flash Program : Error!
pause

:END