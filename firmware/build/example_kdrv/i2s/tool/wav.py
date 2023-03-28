from khost import CMD_ID_GO, khost_t
import random
import sys
import time
import wave
import sys 

argc = len(sys.argv)
argv = sys.argv
print('Number of arguments: {}'.format(argc))
print('Argument(s) passed: {}'.format(argv))

if argc != 2:
    print('usage:')
    print('python wav.py -r|-p, -r: recording test. -p: play test.')
    sys.exit(0)

if argv[1] != '-r' and argv[1] != '-p':
    print('usage:')
    print('python wav.py -r|-p, -r: recording test. -p: play test.')
    sys.exit(0)

x = khost_t()
x.connect()
if x.uid == 0:
    raise ValueError("No USB device connected")
print('connect to device:' + str(x.uid))
CMD_ID_GO_2 = 0x07


# test on play wav file
if  argv[1] == '-p':
    binarycontent = bytearray()
    try:
        with open("mozart_stereo.wav", "rb") as f:
            byte = f.read(1)
            binarycontent += byte
            while byte:
                byte = f.read(1)
                binarycontent += byte
    except IOError:
        print('Error While Opening the file!')  
    print('wav file len: ' + str(len(binarycontent)) )
    x.mem_write(0x80000000, binarycontent, len(binarycontent))
    x.cmd_header_write(CMD_ID_GO,0,0)

# test on recording wav file
if  argv[1] == '-r':
    x.cmd_header_write(CMD_ID_GO_2,0,0)
    time.sleep(12)
    data_size = (32 * 1024 * 2 * 2) * 10
    raw = x.mem_read(0x80000000, data_size)
    with wave.open('record.wav','wb') as f:
        f.setnchannels(2)
        f.setsampwidth(2)
        f.setframerate(32000)
        f.writeframesraw(bytes(raw))
x.disconnect()
