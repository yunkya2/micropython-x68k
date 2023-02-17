import x68k
from struct import pack,unpack
from ctypes import addressof

buf=bytearray(96)
x68k.dos(x68k.d.GETDPB,pack('hl',0,addressof(buf)))

dpb=unpack('2BH2BH2B4HLBbLH64s',buf)
print('Drive : {:c}'.format(dpb[0]+ord('A')))
print('Unit : {}'.format(dpb[1]))
print('Bytes/sector : {}'.format(dpb[2]))
print('Sectors/cluster : {}'.format(dpb[3]+1))
print('Sector # of the first cluster : {}'.format(dpb[4]))
print('Sector # of FAT : {}'.format(dpb[5]))
print('# of FAT : {}'.format(dpb[6]))
print('# of sectors in FAT : {}'.format(dpb[7]))
print('# of root directory entries : {}'.format(dpb[8]))
print('Sector # of the data area : {}'.format(dpb[9]))
print('# of the total clusters : {}'.format(dpb[10]))
print('Sector # of the root directory : {}'.format(dpb[11]))
print('Pointer to the device driver : 0x{:x}'.format(dpb[12]))
print('Media byte : 0x{:02x}'.format(dpb[13]))
print('DPB in use flag : {}'.format(dpb[14]))
print('Next DPB pointer : 0x{:x}'.format(dpb[15]))
print('Cluster # of the current directory : {}'.format(dpb[16]))
print('Current directory : {:s}'.format(dpb[17]))
