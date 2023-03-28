
REM == HSIN-CHU HAPS & Taipei HAPS xtensa path ==
SET XTENSA_PATH=d:\winapp\xtensa\XtDevTools\install

REM == SD HAPS xtensa path ==
REM SET XTENSA_PATH=D:\xtensa\XtDevTools\install
SET UTILS_PATH=..\..\..\..\utils
echo "del old binary file"
del HelloWorld
del fw_ncpu_tmp.bin
del fw_ncpu.bin
del fw_ncpu_enc.bin

copy bin\vp6_asic\Debug\HelloWorld  .


call %XTENSA_PATH%\tools\RI-2019.2-win32\XtensaTools\Tools\misc\xtensaenv.bat %XTENSA_PATH%\builds\RI-2019.2-win32\vp6_asic %XTENSA_PATH%\tools\RI-2019.2-win32\XtensaTools %XTENSA_PATH%\builds\RI-2019.2-win32\vp6_asic\config vp6_asic
echo " Run xt-objcopy to generate fw_ncpu.bin"
xt-objcopy.exe -j .ResetVector.text -j .WindowVectors.text -j .Level2InterruptVector.text -j .Level3InterruptVector.text -j .DebugExceptionVector.text -j .NMIExceptionVector.text  -j .KernelExceptionVector.text  -j .UserExceptionVector.text -j .DoubleExceptionVector.text -j .clib.rodata -j .rtos.rodata  -j .rodata -j .text  -j .clib.data -j .rtos.percpu.data -j .data -j .bss -j .debug_pubnames  -j .debug_info  -j .debug_abbrev -j .debug_line -j .xt.prop  -j .xtensa.info -j .comment -j .debug_ranges -O binary .\HelloWorld fw_ncpu_tmp.bin

echo "generate ncpu secure firmware binary file"
%UTILS_PATH%\sbtenc\sbtenc.exe -e -i fw_ncpu_tmp.bin -o fw_ncpu_tmp_enc.bin -s %UTILS_PATH%\sbtenc\keys\sbtkey.bin
echo "transfer secure firmware to DFU format"
%UTILS_PATH%\dfu\gen_dfu_binary_for_win.exe -ncpu fw_ncpu_tmp_enc.bin fw_ncpu_enc.bin
copy fw_ncpu_enc.bin %UTILS_PATH%\bin_gen\flash_bin
copy fw_ncpu_enc.bin %UTILS_PATH%\JLink_programmer\bin
