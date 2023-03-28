'''
 @file bin_gen.py
 @brief Generate flash final bin file from other seperate bin files.
'''

import argparse
import io
import os
import numpy as np
import platform
import shutil

FW_ONLY=0
BIN_DIR="./flash_bin"
OUTPUT_FW_FILE="flash_image.bin"
MODEL_NAME="models_720.nef"
BOARD_FLASH_SIZE=0x8000000  # 128 MB (for nand flash)
BLOCK_SIZE=0x20000          # 128KB (for nand flash)

i_offset=0
i_size_limit=1
i_bin_file=2
i_is_required=3
flash_bin_info_nand = [
               #  * size_limit=0 means allow MAX size within offset interval
               #   offset,     size_limit,   bin_file,          required
               #  ---------------------------------------------------------
                  [0x00000000, 32*1024,     'boot_spl.bin',     True],
                  [0x00020000, 32*1024,     'boot_spl.bin',     True],    # backup

                  [0x00040000, 128*1024,    'fw_scpu.bin',      True],
                  [0x00060000, 2*1024*1024, 'fw_ncpu.bin',      True],
                  [0x00260000, 4*1024,      'boot_cfg0.bin',    False],

                  [0x00280000, 128*1024,    'fw_scpu1.bin',     False],   # backup
                  [0x002A0000, 2*1024*1024, 'fw_ncpu1.bin',     False],   # backup
                  [0x004A0000, 4*1024,      'boot_cfg1.bin',    False],   # backup

                  [0x004C0000, 0,           'reserved.bin',     False],   # reserved for user defined data

                  [0x00540000, 4*1024,      'fw_info.bin',      True],
                  [0x00560000, 0,           'all_models.bin',   True]
                 ]

flash_bin_info_nor = [
               #  * size_limit=0 means allow MAX size within offset interval
               #   offset,     size_limit,   bin_file,          required
               #  ---------------------------------------------------------
                  [0x00000000, 32*1024,     'boot_spl_nor.bin', True],

                  [0x00008000, 128*1024,    'fw_scpu.bin',      True],
                  [0x00028000, 2*1024*1024, 'fw_ncpu.bin',      True],
                  [0x00228000, 4*1024,      'boot_cfg0.bin',    False],

                  [0x00229000, 128*1024,    'fw_scpu1.bin',     False],   # backup
                  [0x00249000, 2*1024*1024, 'fw_ncpu1.bin',     False],   # backup
                  [0x00449000, 4*1024,      'boot_cfg1.bin',    False],   # backup

                  [0x0044A000, 0,           'reserved.bin',     False],   # reserved for user defined data

                  [0x004CA000, 4*1024,      'fw_info.bin',      True],
                  [0x004CB000, 0,           'all_models.bin',   True]
                 ]

def file_read_binary(src):
     ''' Load depth file '''
     with open(src, "rb") as f:
         data = f.read()
         return data

def write_data_to_binary_file(ofile, data):
    with io.open(ofile, 'wb') as fo:
        fo.write(bytearray(data))

def is_offset_block_size_aligned(bin_table, block_size):
    for bin_file in bin_table:
        if bin_file[i_offset] % block_size != 0:
            print('Error! Offset of', bin_file[i_bin_file], 'is not aligned with flash block size')
            return False
    return True;

def bin_gen(bin_info, ofile):
    print('')
    print('Generating flash binary file...\n')
    print('Target flash size = {:.0f} MB'.format(BOARD_FLASH_SIZE/1024/1024))
    print(' --- Bin file information list ---\n')

    if is_offset_block_size_aligned(flash_table, BLOCK_SIZE) == False:
        return False

    model_size = 0
    total_bin_size = 0
    platform_os = platform.system()

    if platform_os == 'Windows':
        cmd = r'..\nef_utility\nef_utility_win.exe -X flash_bin' + '\\' + MODEL_NAME
        nef_file = r'flash_bin' + '\\' + MODEL_NAME
        fw_info_path = r'output\fw_info.bin'
        all_models_path = r'output\all_models.bin'
        fw_info_path_new = r'flash_bin\fw_info.bin'
        all_models_path_new = r'flash_bin\all_models.bin'
    elif platform_os == 'Linux':
        cmd = r'../nef_utility/nef_utility_linux -X flash_bin/' + MODEL_NAME
        nef_file = r'flash_bin/' + MODEL_NAME
        fw_info_path = r'output/fw_info.bin'
        all_models_path = r'output/all_models.bin'
        fw_info_path_new = r'flash_bin/fw_info.bin'
        all_models_path_new = r'flash_bin/all_models.bin'

	# calcualte image size and fill 0xff
    if FW_ONLY == 0:
	    # split NEF into fw_info.bin + all_models.bin
        if os.path.isfile(nef_file):
            os.system(cmd)
            shutil.move(fw_info_path, fw_info_path_new)
            shutil.move(all_models_path, all_models_path_new)
        else:
            print('Error!', MODEL_NAME, 'not exist in flash_bin folder ')
            return False

        i_allmodel = -1
        model_file = BIN_DIR + "/" + bin_info[i_allmodel][i_bin_file]
        if os.path.isfile(model_file):
            model_size = os.path.getsize(model_file)
        else:
            print('Error! model file is not available')
            return False

        total_bin_size = bin_info[i_allmodel][i_offset] + model_size
    else:
	    # also clean others in SPL/SCPU/NCPU area
        total_bin_size = bin_info[-1][i_offset] + bin_info[-1][i_size_limit]

    bin_data = np.zeros(total_bin_size, dtype=np.uint8)
    bin_data.fill(0xff)

    # concate binary files
    for index, bin_file in enumerate(bin_info):
        abort=False
        file_status_str=''
        bin_start_addr = bin_file[i_offset]
        file_name = BIN_DIR + "/" + bin_file[i_bin_file]

        if os.path.isfile(file_name) == 0:
            # check must files
            if bin_file[i_is_required] == True:
                file_status_str='  not existed, ERROR!'
                abort=True
            else:
                file_status_str='  not existed, skip!'
        else:
            rdata = file_read_binary(file_name)
            rdata = list(rdata)
            rdata = np.array(rdata)

            # check if item data is larger than size_limit
            if bin_info[index][i_size_limit] == 0:
                if index == len(bin_info) - 1:
                    size_limit = BOARD_FLASH_SIZE - bin_info[index][i_offset]
                else:
                    size_limit = bin_info[index+1][i_offset] - bin_info[index][i_offset]
                # update size_limit in bin_info table
                bin_info[index][i_size_limit] = size_limit
            else:
                size_limit = bin_info[index][i_size_limit]

            if len(rdata) > size_limit:
                file_status_str='  size over limit, ERROR!'
                abort=True
            else:
                bin_data[bin_start_addr:bin_start_addr + len(rdata)] = rdata

        size_in_kb = int(bin_info[index][i_size_limit]/1024)
        print('{:<30s}  =>  address: 0x{:08X}, size_limit: 0X{:08X} ( ~{:>6d} KB) {:s}' \
                .format(bin_info[index][i_bin_file],
                        bin_info[index][i_offset],
                        bin_info[index][i_size_limit],
                        size_in_kb, file_status_str))
        if abort==True:
            return False

    write_data_to_binary_file(ofile, bin_data)
    print('\n --------------------------------\n')

    if FW_ONLY == 0 and os.path.isfile(nef_file):
        os.rmdir("output")
        os.remove(fw_info_path_new)
        os.remove(all_models_path_new)

    if os.path.isfile(OUTPUT_FW_FILE):
        full_path_name=os.path.abspath(OUTPUT_FW_FILE)
        print(' {:s} is generated'.format(full_path_name))
        return True
    else:
        print('Error!!')
        return False

if __name__ == '__main__':
    ### please make sure image directory and model path area correct

    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--CPU_ONLY', action='store_true', help='FW only', default=False)
    parser.add_argument('-f', '--flash_size', type=int, help='target board flash size in MB')
    parser.add_argument('-r', '--nor', action='store_true', help='nor type flash', default=False)

    args = parser.parse_args()

    if (args.CPU_ONLY):
        FW_ONLY = 1

    take_user_flash_size=False
    if (args.flash_size):
        BOARD_FLASH_SIZE=args.flash_size*1024*1024
        take_user_flash_size=True

    if (args.nor):
        flash_table = flash_bin_info_nor
        BLOCK_SIZE = 0x1000 # 4 KB
        if take_user_flash_size == False:
            BOARD_FLASH_SIZE = 0x2000000  # 32 MB

        if FW_ONLY == 1:
            flash_table = flash_table[:3]
    else:
        flash_table = flash_bin_info_nand
        if FW_ONLY == 1:
            flash_table = flash_table[:4]

    ret = bin_gen(flash_table, OUTPUT_FW_FILE)

    if ret:
        print("\n[done]")
    else:
        print("\n[Failed]")

