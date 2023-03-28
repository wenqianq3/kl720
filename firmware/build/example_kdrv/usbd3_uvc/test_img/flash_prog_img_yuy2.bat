start /wait "J-Link Commander" "..\..\..\..\utils\JLink_programmer\JLink.exe" -settingsfile "..\..\..\..\utils\JLink_programmer\Sample.jlinksettings" -CommanderScript .\flash_prog_img_yuy2.jlink
IF ERRORLEVEL 1 goto ERROR
ECHO J-Flash Program : OK!
goto END

:ERROR
ECHO J-Flash Program : Error!
pause

:END