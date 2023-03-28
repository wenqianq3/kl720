REM "DFU binary file convert"
SET UTILS_PATH=..\..\..\..\..\..\utils
%UTILS_PATH%\dfu\gen_dfu_binary_for_win.exe -scpu .\Objects\fw_scpu_tmp.bin .\Objects\fw_scpu.bin
copy .\Objects\fw_scpu.bin %UTILS_PATH%\bin_gen\flash_bin\
copy .\Objects\fw_scpu.bin %UTILS_PATH%\JLink_programmer\bin\
REM "Del temp binary file convert"
del .\Objects\fw_scpu_tmp.bin

