#!/usr/bin/python3

import sys
import os
import csv
import argparse
import struct 

parser = argparse.ArgumentParser()

parser.add_argument('-b', '--bin')
parser.add_argument('-d', '--dcd')
parser.add_argument('-i', '--imx', default = "load.imx")
parser.add_argument('-g', '--img', default = "load.img")
parser.add_argument('-a', '--addr', default = "0x80010000")

args = parser.parse_args()

args.addr = int(args.addr, 16)

if args.bin is not None:
    if not os.path.exists(args.bin):
        print('bin file : %s is not exist' % args.bin)
    print('bin file : %s' % args.bin)

    if args.dcd is None:
        args.dcd = os.path.join(os.path.split(os.path.realpath(sys.argv[0]))[0], "dcdConfig.csv")
        print('use default dcd config file : %s' % args.dcd)
    else:
        args.dcd = os.path.realpath(args.dcd)
        print('dcd config file : %s' % args.dcd)

    #read dcd file and generate dcdConfig structural body
    with open(args.dcd, 'r') as d:
        csvReader = csv.reader(d)
        dcdConfig = list(csvReader)
        d.close()

    #build imx file
    with open(args.imx, 'wb') as f:
        #write ivt
        f.write(struct.pack('<B', 0xD1))
        f.write(struct.pack('>H', 32))
        f.write(struct.pack('<B', 0x40))
        f.write(struct.pack('<I', args.addr))
        f.write(struct.pack('<I', 0x00000000))
        f.write(struct.pack('<I', args.addr - (len(dcdConfig) + 318) * 8 - 4))
        f.write(struct.pack('<I', args.addr - (len(dcdConfig) + 318) * 8 - 16))
        f.write(struct.pack('<I', args.addr - (len(dcdConfig) + 318) * 8 - 48))
        f.write(struct.pack('<I', 0x00000000))
        f.write(struct.pack('<I', 0x00000000))

        #write boot data
        f.write(struct.pack('<I', args.addr - (len(dcdConfig) + 318) * 8 - 1072))
        f.write(struct.pack('<I', ((os.path.getsize(args.bin) + 0x1000 - 1) & ~(0x1000 - 1)) + 4096))
        f.write(struct.pack('<I', 0x00000000))

        #write DCD
            #dcd header
        f.write(struct.pack('<B', 0xD2))
        f.write(struct.pack('>H', len(dcdConfig) * 8 + 8))
        f.write(struct.pack('<B', 0x40))
            #dcd wrute header
        f.write(struct.pack('<B', 0xCC))
        f.write(struct.pack('>H', len(dcdConfig) * 8 + 4))
        f.write(struct.pack('<B', 0x04))
            #dcd wrute data
        for d in dcdConfig:
            f.write(struct.pack('>I', int(d[0], 16)))
            f.write(struct.pack('>I', int(d[1], 16)))

        #padding data
        for i in range(0x9EC):
            f.write(struct.pack('<B', 0x00))

        #write bin data
        with open(args.bin, 'rb') as b:
            while(True):
                raw = b.read(1024)
                if len(raw) == 0:
                    b.close()
                    break
                else:
                    f.write(raw)

        #padding data
        for i in range(((os.path.getsize(args.bin) + 0x1000 - 1) & ~(0x1000 - 1)) - os.path.getsize(args.bin)):
            f.write(struct.pack('<B', 0x00))

        f.close
    
    #build img file
    with open(args.img, 'wb') as g:
        #Header write 1K data
        for i in range(0x400):
            g.write(struct.pack('<B', 0x00))

        #write imx data
        with open(args.imx, 'rb') as f:
            while(True):
                raw = f.read(1024)
                if len(raw) == 0:
                    f.close()
                    break
                else:
                    g.write(raw)

    print('generate %s and %s finished!' %(args.imx, args.img))
    g.close()

