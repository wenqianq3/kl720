# utils/bin_gen.py

To compose flash image binary file from separated binary files.
This script combines .bin files in folder, `flash_bin`,  in a predefined order.

> Binary placing offset for KL720 flash should align with 128KB  (NAND) / 4KB (NOR)

## Prerequisite

```
pip install argparse numpy
```

## Command

```
$ python3 bin_gen.py <options>

usage: bin_gen.py [-h] [-p] [-f FLASH_SIZE] [-r]

optional arguments:
  -h, --help            show this help message and exit
  -p, --CPU_ONLY        FW only
  -f FLASH_SIZE, --flash_size FLASH_SIZE
                        target board flash size in MB
  -r, --nor             nor type flash
```

> Default flash size is 128 MB (NAND) / 32 MB (NOR)

## Note

**The following bin files are must**

```
flash_bin
├── boot_spl.bin      // bool spl bin file (for NAND flash)
├── boot_spl_nor.bin  // bool spl bin file (for NOR flash)
├── fw_ncpu.bin       // SCPU FW bin file (generated by Keil)
├── fw_scpu.bin       // NCPU FW bin file (generated by Keil)
├── models_720.nef    // model (or copied from plus/res/models/KL720)
```
