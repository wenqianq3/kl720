@ECHO OFF

SET model_name=models_720.nef
SET jlink_script=flash_model.jlink

IF NOT EXIST .\bin\%model_name% (
    ECHO ERROR! %model_name% is not found in bin folder
    EXIT /b 1
)

REM extract fw_info.bin and all_models.bin from NEF file
..\nef_utility\nef_utility_win.exe -X .\bin\models_720.nef
IF ERRORLEVEL 1 (
    ECHO ERROR! Failed on extracting data from NEF file
    EXIT /b 1
)
COPY .\output\* "bin" >NUL

start /wait "J-Link Commander" "JLink.exe" -settingsfile .\Sample.jlinksettings -CommanderScript .\%jlink_script% -ExitOnError 1
IF ERRORLEVEL 1 goto ERROR
ECHO J-Flash Program : OK!

REM delete temp files
RD /q /s output
DEL .\bin\fw_info.bin
DEL .\bin\all_models.bin

goto END

:ERROR
ECHO J-Flash Program : Error!
pause

:END
