@ECHO OFF
REM "DFU binary file convert"
SET BIN_IN=%1
SET BIN_OUT=fw_scpu.bin
SET ENC_BIN_OUT=fw_scpu_enc.bin

SET UTILS_PATH=..\..\..\..\utils

ECHO Generating DFU binary [%BIN_IN% -^> %BIN_OUT%]...
%UTILS_PATH%\dfu\gen_dfu_binary_for_win.exe -scpu .\Objects\%BIN_IN% .\Objects\%BIN_OUT%
copy .\Objects\%BIN_OUT% %UTILS_PATH%\bin_gen\flash_bin\
copy .\Objects\%BIN_OUT% %UTILS_PATH%\JLink_programmer\bin\

IF "%~2"=="gen_enc" (
	
    ECHO Generating DFU binary [%BIN_IN% -^> %ENC_BIN_OUT% ]...
    ECHO KEY: %UTILS_PATH%\sbtenc\keys\sbtkey.bin

    REM "generate scpu secure firmware binary file"
    %UTILS_PATH%\sbtenc\sbtenc.exe -e -i .\Objects\%BIN_IN% -o .\Objects\tmp_enc.bin -s %UTILS_PATH%\sbtenc\keys\sbtkey.bin
    %UTILS_PATH%\dfu\gen_dfu_binary_for_win.exe -scpu .\Objects\tmp_enc.bin .\Objects\%ENC_BIN_OUT%
    copy .\Objects\%ENC_BIN_OUT% %UTILS_PATH%\bin_gen\flash_bin\
    copy .\Objects\%ENC_BIN_OUT% %UTILS_PATH%\JLink_programmer\bin\

    REM "Del temp binary file convert"
    del .\Objects\tmp_enc.bin
)
REM "Del temp binary file convert"
del .\Objects\%BIN_IN%

