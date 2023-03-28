/* Please be aware this feature so far only can support windows OS */

1. Hardware : set to manual boot mode or boot from USB directly

2. connect UART0 by Teraterm or Putty / BR115200, connect USB 3.0

3. press reset button

4. (manual boot mode) enter '3', then you can see "Kneron Beethoven" (VID3231 PID0720) from Deveice Manager, Teraterm message as below
	BOOT MODE: Manual
	1. SPI-NOR
	2. SPI-NAND
	3. USB
	4. UART(Xmodem)
	Please select boot mode[1-4]: 3


	USB is enabled, waiting for download...

	Device is linked.

5. open zadig and selece device "Kneron Beethoven" (VID3231 PID0720), driver select "WinUSB (v6.1.7600.16385)" and install it.

6. copy fw_scpu.bin and fw_ncpu.bin to "beethoven_sw\firmware\utils\usb_minion\app\bin\"

7. open Windows Command Prompt under beethoven_sw\firmware\utils\usb_minion\app\

8. execute "kl720_usb_dfw.exe", and if it works well, 3 options will be displayed at the end

	1. start dfu-util-static to download usb_minion.bin..
	Cannot connec to KL720 USB DFU device (0x3231 0x0720)
	If the device is connected, please check if the WinUSB driver has been installed
	========bcdDevice 0x101========
	Connected to KL720 USB DFU  device (0x3231 0x0720) at Super-Speed.
	
	2. usb_minion takes the handle..
	wait for KL720 DFW USB-MINION device (0x3231 0x0720)
	If the device is connected, please check if the WinUSB driver has been installed
	========bcdDevice 0xBA========
	Connected to KL720 DFW USB-MINION device (0x3231 0x0999) at High-Speed.
	
	For 1) Flash PROGRAMMING, please set transfer size = 131072(0x20000) .
	For 2) USB DFW BOOT, please set transfer size = 983040(0xF0000) .
	Please select one of following items:
	1) Flash PROGRAMMING
	2) USB DFW BOOT
	3) EXIT
	Select:

9. If your project is designed with external SPI Flash, you can select option 1) Flash PROGRAMMING and update/verify SCPU/NCPU FW to Flash automatically

10. If your project is designed without external SPI Flash, you can select option 2) USB DFW BOOT to do DFW SCPU/NCPU FW to chip internal memory spcae and boot up automatically

11. option 3) EXIT to close kl720_usb_dfw.exe
 
