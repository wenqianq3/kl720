@ECHO OFF
REM "DFU binary file convert"
SET BIN_IN=%1
SET BIN_OUT=fw_scpu.bin
SET UTILS_PATH=..\..\..\..\..\utils

ECHO Generating DFU binary [%BIN_IN% -^> %BIN_OUT%]...
%UTILS_PATH%\dfu\gen_dfu_binary_for_win.exe -scpu .\Objects\%BIN_IN% .\Objects\%BIN_OUT%
copy .\Objects\%BIN_OUT% %UTILS_PATH%\bin_gen\flash_bin\
copy .\Objects\%BIN_OUT% %UTILS_PATH%\JLink_programmer\bin\
REM "Del temp binary file convert"
del .\Objects\%BIN_IN%

