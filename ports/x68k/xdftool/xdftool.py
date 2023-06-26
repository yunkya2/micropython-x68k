#!/usr/bin/env python3

# Copyright (c) 2023 Yuichi Nakamura
# Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

import sys
import os
import time
from struct import pack, unpack

# X68k FD boot sector
FDBOOTSECT = b'`<\x90X68IPL30\x00\x04\x01\x01\x00\x02\xc0\x00\xd0\x04\xfe\x02\x00\x08\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00           FAT12   O\xfa\xff\xc0M\xfa\x01\xb8K\xfa\x00\xe0I\xfa\x00\xeaC\xfa\x01 N\x94p\x8eNO~p\xe1H\x8e@&:\x01\x02"N$:\x01\x002\x07N\x95f("N2:\x00\xfa IE\xfa\x01xp\n\x00\x10\x00 \xb1\nV\xc8\xff\xf8g8\xd2\xfc\x00 Q\xc9\xff\xe6E\xfa\x00\xe0`\x10E\xfa\x00\xfa`\nE\xfa\x01\x10`\x04E\xfa\x01(a\x00\x00\x94"JL\x99\x00\x06p#NON\x942\x07pONOp\xfeNOt\x004)\x00\x1a\xe1Z\xd4z\x00\xa4\x84\xfa\x00\x9c\x84z\x00\x94\xe2\nd\x04\x08\xc2\x00\x18HBR\x02"N&:\x00~2\x07N\x954|h\x00"N\x0cYHUf\xa6T\x89\xb5\xd9f\xa6/\x19 Y\xd1\xd9/\x08/\x112|g\xc0v@\xd6\x88N\x95"\x1f$\x1f"_J\x80f\x00\xff|\xd5\xc2S\x81e\x04B\x1a`\xf8N\xd1pFNO\x08\x00\x00\x1ef\x02p\x00Nup!NONur\x0fp"NOr\x19t\x0cp#NOa\x08r\x19t\rp#NOv,r p NOQ\xcb\xff\xf8Nu\x00\x00\x04\x00\x03\x00\x00\x06\x00\x08\x00\x1f\x00\t\x1a\x00\x00"\x00\rHuman.sys \x82\xaa \x8c\xa9\x82\xc2\x82\xa9\x82\xe8\x82\xdc\x82\xb9\x82\xf1\x00\x00%\x00\r\x83f\x83B\x83X\x83N\x82\xaa\x81@\x93\xc7\x82\xdf\x82\xdc\x82\xb9\x82\xf1\x00\x00\x00#\x00\rHuman.sys \x82\xaa \x89\xf3\x82\xea\x82\xc4\x82\xa2\x82\xdc\x82\xb7\x00\x00 \x00\rHuman.sys \x82\xcc \x83A\x83h\x83\x8c\x83X\x82\xaa\x88\xd9\x8f\xed\x82\xc5\x82\xb7\x00human   sys\x00\x00\x00\x00\x00'

# BPB parameters
BPB = {
    '':   ( 1024, 1, 1, 2, 192, 1232, 0xfe, 2,  8, 2 ),       # 2HD (1232KB)
    '/5': (  512, 1, 1, 2, 224, 2400, 0xf9, 7, 15, 2 ),       # 2HC (1200KB)
    '/8': (  512, 2, 1, 2, 112, 1280, 0xfb, 2,  8, 2 ),       # 2DD  (640KB)
    '/9': (  512, 2, 1, 2, 112, 1440, 0xf9, 3,  9, 2 ),       # 2DD  (720KB)
    '/4': (  512, 1, 1, 2, 224, 2880, 0xf0, 9, 18, 2 ),       # 2HQ (1440KB)
}

# For error handling
class DiskFull(Exception):
    def __str__(self):
        return 'Disk image full'
class RootDirFull(Exception):
    def __str__(self):
        return 'Root directory full'
class FileNotFound(Exception):
    def __init__(self, name):
        self.name = name
    def __str__(self):
        return f'No such file or directory: \'{self.name:s}\''


class DiskFat:
    def __init__(self, disk):
        """Create new FAT data"""
        self.disk = disk
        self.table = [0] * self.disk.maxcls
        self.table[0] = 0xfff
        self.table[1] = 0xfff

        if self.disk.fhin:          # read FAT data in the XDF file
            self.disk.fhin.seek(self.disk.sect2byte(self.disk.snfat))
            self.decode(self.disk.fhin.read(self.disk.sect2byte(self.disk.fatsize)))

    def flush(self):
        """Write FAT data into new XDF file"""
        self.disk.fhout.seek(self.disk.sect2byte(self.disk.snfat))
        self.disk.fhout.write(self.encode())

    def decode(self, data):
        """Decode binary FAT data into the table entries"""
        n = 0
        i = 0
        while n < self.disk.maxcls:
            self.table[n] = data[i] + ((data[i + 1] & 0xf) << 8)
            if n >= self.disk.maxcls - 1:
                break
            self.table[n + 1] = ((data[i + 1] & 0xf0) >> 4) + (data[i + 2] << 4)
            n += 2
            i += 3

    def encode(self):
        """Encode FAT data into binary"""
        data = bytearray(b'\0' * self.disk.sect2byte(self.disk.fatsize))
        n = 0
        f = True
        for c in self.table:
            if f:
                d = c
            else:
                d = d | (c << 12)
                dd = pack('<L', d)
                data[n:n + 3] = dd[:3]
                n += 3
            f = not f
        if not f:
            dd = pack('<L', d)
            data[n:n + 3] = dd[:3]
        data[0] = self.disk.mediabyte
        return data

    def getchain(self, cls):
        """Get cluster chain starting with cluster 'cls'"""
        if cls == 0:
            return []
        clist = [cls]
        while self.table[cls] != 0xfff:
            cls = self.table[cls]
            clist.append(cls)
        return clist

    def allocate(self, size=0):
        """Allocate clusters for 'size' bytes data"""
        ncls = (size // self.disk.clsbytes) + 1
        clist = []
        lastcls = -1
        try:
            for i in range(ncls):
                cls = self.table.index(0, lastcls + 1)
                clist.append(cls)
                lastcls = cls
        except ValueError as e:
            raise DiskFull
        for i in range(len(clist) - 1):
            self.table[clist[i]] = clist[i + 1]
        self.table[clist[-1]] = 0xfff
        return clist[0]

    def extend(self, cls):
        """Extend existing cluster chain to add one cluster"""
        clist = self.getchain(cls)
        try:
            c = self.table.index(0)
        except ValueError as e:
            raise DiskFull
        self.table[clist[-1]] = c
        self.table[c] = 0xfff

    def __repr__(self):
        x = 0
        r = ''
        for n in self.table:
            if (x & 0xf) == 0:
                r += f'\n{x:03x} :'
            r += f' {n:03x}'
            if (x & 0xf) == 7:
                r += ' '
            x += 1
        r += '\n'
        return r


class Dirent:
    def __init__(self, disk, data=None, name='', attr=0, cls=0, dir=None):
        """Create new directory entry"""
        self.disk = disk
        if data:        # read existing entry
            self.decode(data)
        else:           # create new entry
            self.name = name
            self.attr = attr
            self.cls = cls
            self.size = 0
            self.time = time.time()
            self.dir = dir

    def decode(self, data):
        """Decode one directory entry"""
        self.name = self.decodename(data[0:8], data[8:11], data[12:22])
        self.attr = data[11]
        (dtime, ddate, self.cls, self.size) = unpack('<3HL', data[22:32])
        self.time = time.mktime((
                        ((ddate >> 9) & 0x7f) + 1980,
                        ((ddate >> 5) & 0xf),
                        (ddate & 0x1f),
                        ((dtime >> 11) & 0x1f),
                        ((dtime >> 5) & 0x3f),
                        ((dtime & 0x1f) << 1),
                        0, 0, 0))

    def encode(self):
        """Encode one directory entry"""
        (name1, ext, name2) = self.encodename(self.name)
        data = pack('<8s3sB10s', name1, ext, self.attr, name2)
        tm = time.localtime(self.time)
        dtime = (tm[3] << 11) | (tm[4] << 5) | (tm[5] >> 1)
        ddate = ((tm[0] - 1980) << 9) | (tm[1] << 5) | tm[2]
        data += pack('<3HL', dtime, ddate, self.cls, self.size)
        return data

    def decodename(self, name1, ext, name2):
        """Decode filename in the directory entry"""
        name = name1
        if name2[0]:
            name = (name1 + name2.replace(b'\0', b' ')).rstrip(b' ')
        else:
            name = name1.rstrip(b' ')
        if name[0] == 0x05:
            name[0] = 0xe5
        if ext != b'   ':
            name += b'.' + ext.rstrip(b' ')         # file extension
        return name.decode('ShiftJIS')

    def encodename(self, name):
        """Encode filename for the directory entry"""
        bname = name.encode('ShiftJIS')
        if bname[0] == 0xe5:
            bname[0] = 0x05
        ext = b''
        if name != '.' and name != '..':
            epos = bname.rfind(b'.')
            if epos > 0 and (len(bname) - epos) <= 4:   # file extension
                ext = bname[epos + 1:]
                bname = bname[:epos]
        ext = (ext + b'   ')[:3]
        bname = bname + b' ' * 18
        name1 = bname[0:8]
        name2 = (bname[8:18].rstrip(b' ') + b'\0' * 10)[:10]
        return (name1, ext, name2)

    def isdir(self):
        return self.attr & 0x10
    
    def isvol(self):
        return self.attr & 0x08

    def readdir(self):
        if not self.isdir():
            return None
        return DiskDir(self.disk, self.cls)

    def readfile(self, dirname):
        """Read file contents from XDF image and create the file"""
        if self.name == '.' or self.name == '..':
            return
        if self.isvol() or self.isdir():
            return
        with open(dirname + self.name, 'wb') as f:
            size = self.size
            clsbytes = self.disk.clsbytes
            for c in self.disk.fat.getchain(self.cls):
                s = min(size, clsbytes)
                if s == 0:
                    break
                self.disk.fhin.seek(self.disk.clsoffset(c))
                f.write(self.disk.fhin.read(s))
                size -= s
        if 'utime' in os.__dict__:
            os.utime(dirname + self.name, times=(self.time, self.time))

    def __repr__(self):
        if self.isdir():
            s = '  <dir>   '
        elif self.isvol():
            s = '  <vol>   '
        else:
            s = f'{self.size:-8d}  '
        tm = time.localtime(self.time)
        s += f'{tm[0]:04d}-{tm[1]:02d}-{tm[2]:02d} {tm[3]:2d}:{tm[4]:02d}:{tm[5]:02d}'
        s += '  ' + self.name
        return s


class DiskDir:
    def __init__(self, disk, cls=0):
        """Create new directory data starting with cluster 'cls'"""
        self.disk = disk
        self.entlist = []
        self.cls = cls

        if self.disk.fhin:          # read diectory in the XDF file
            if self.cls == 0:           # root directory
                self.disk.fhin.seek(self.disk.sect2byte(self.disk.snrootdir))
                self.decode(self.disk.fhin.read(self.disk.nrootdirent * 32))
            else:                       # sub directory
                dirblk = b''
                for c in self.disk.fat.getchain(self.cls):
                    self.disk.fhin.seek(self.disk.clsoffset(c))
                    dirblk += self.disk.fhin.read(self.disk.clsbytes)
                self.decode(dirblk)
        else:                       # create initial directory data
            if self.cls == 0:           # root directory
                self.entmax = self.disk.nrootdirent
            else:                       # sub directory
                self.entmax = self.disk.clsbytes // 32

    def flush(self):
        """Write directory into new XDF file"""
        data = self.encode()
        if self.cls == 0:               # root directory
            self.disk.fhout.seek(self.disk.sect2byte(self.disk.snrootdir))
            self.disk.fhout.write(data)
        else:                           # sub directory
            i = 0
            for c in self.disk.fat.getchain(self.cls):
                self.disk.fhout.seek(self.disk.clsoffset(c))
                self.disk.fhout.write(data[i:i + self.disk.clsbytes])
                i += self.disk.clsbytes

    def decode(self, data):
        """Decode directory data into entry lists"""
        self.entlist = []
        i = 0
        while i < len(data):
            if data[i] == 0:
                break
            if data[i] != 0xe5:
                dirent = Dirent(self.disk, data[i : i+32])
                self.entlist.append(dirent)
            i += 32

    def encode(self):
        """Encode directory entry list into the data"""
        data = b''
        for d in self.entlist:
            data += d.encode()
        return data

    def adddirent(self, dirent):
        """Add one directory entry into the list and extend the directory data if needed"""
        self.entlist.append(dirent)
        if len(self.entlist) > self.entmax:
            if self.cls == 0:
                raise RootDirFull   # root directory cannot be extended
            self.disk.fat.extend(self.cls)
            self.entmax += self.disk.clsbytes // 32

    def mkdir(self, dirname):
        """Make new sub directory"""
        for f in self.entlist:
            if f.name == dirname:
                return f.dir        # already exists

        # create new empty directory entries
        newdir = DiskDir(self.disk, self.disk.fat.allocate())
        newdir.adddirent(Dirent(self.disk, name='.', attr=0x10, cls=newdir.cls))
        newdir.adddirent(Dirent(self.disk, name='..', attr=0x10, cls=self.cls))

        # add directory data as sub directory
        newent = Dirent(self.disk, name=dirname, attr=0x10, cls=newdir.cls)
        newent.dir = newdir
        self.adddirent(newent)
        self.disk.subdirs.append(newdir)
        return newdir

    def writefile(self, dirname, fname):
        """Write file contents into XDF image from the existing file"""
        try:
            st = os.stat(dirname + fname)
        except:
            raise FileNotFound(dirname + fname)
        size = st[6]
        newent = Dirent(self.disk, name=fname, attr=0x20, cls=self.disk.fat.allocate(size))
        newent.size = size
        newent.time = st[8]

        with open(dirname + fname, 'rb') as f:
            clsbytes = self.disk.clsbytes
            for c in self.disk.fat.getchain(newent.cls):
                s = min(size, clsbytes)
                if s == 0:
                    break
                self.disk.fhout.seek(self.disk.clsoffset(c))
                self.disk.fhout.write(f.read(s))
                size -= s
        self.adddirent(newent)

    def __iter__(self):
        return iter(self.entlist)

    def __repr__(self):
        r = ''
        for n in self.entlist:
            r += str(n) + '\n'
        return r


class XdfDisk:
    def __init__(self, fhin=None, fhout=None, format=None):
        """Create new XDF disk image"""
        self.fhin = fhin                # file handle for input (list/extract)
        self.fhout = fhout              # file handle for output (create)

        bpb = BPB['']
        if self.fhin:
            fhin.seek(0xb)
            b = fhin.read(0x11)
            if b[10] >= 0xf0:
                bpb = unpack('<HBHB2HB3H', b)
        else:
            if format in BPB:
                bpb = BPB[format]

        self.sectbytes = bpb[0]         # sector size in byte (1024)
        self.clssect = bpb[1]           # sectors per cluster (1)
        self.snfat = bpb[2]             # sector # of the FAT (1)
        self.nfat= bpb[3]               # # of FATs (2)
        self.nrootdirent = bpb[4]       # # of root directory entries (192)
        self.nsect = bpb[5]             # # of sectors (1232)
        self.mediabyte = bpb[6]         # media byte (0xfe)
        self.fatsize = bpb[7]           # sectors per FAT (2)

        # cluster size in byte
        self.clsbytes = self.sect2byte(self.clssect)
        # sector # of the root directory
        self.snrootdir = self.snfat + self.nfat * self.fatsize
        # sector # of the data area
        self.sndata = self.snrootdir + (self.nrootdirent * 32) // self.sectbytes
        # max cluster number + 1
        self.maxcls = ((self.nsect - self.sndata) // self.clssect) + 2

        if fhout:
            for i in range(self.nsect):
                fhout.write(bytes(self.sectbytes))
            fhout.seek(0)
            fhout.write(FDBOOTSECT)
            fhout.seek(0xb)
            fhout.write(pack('<HBHB2HB3H', *bpb))
            if format == '/4':
                fhout.seek(0)
                fhout.write(b'\xeb\xfe\x90')

        self.fat = DiskFat(self)
        self.rootdir = DiskDir(self)
        self.subdirs = []

    def sect2byte(self, sect):
        return self.sectbytes * sect

    def clsoffset(self, cls):
        return self.sect2byte(self.sndata) + self.clsbytes * (cls - 2)

    def listxdf(self, dir, name):
        subdirs = []
        print(name + ':')
        for ent in dir:
            print(ent)
            if ent.name == '.' or ent.name == '..':
                continue
            if ent.isdir():
                subdirs.append(ent)
        for ent in subdirs:
            print()
            self.listxdf(ent.readdir(), name + '/' + ent.name)

    def extractxdf(self, dir, dirname, extfiles):
        for ent in dir:
            if ent.name == '.' or ent.name == '..':
                continue
            if ent.isdir():
                extfilessub = extfiles
                if not extfiles or ((dirname + ent.name) in extfiles) \
                                or ((dirname + ent.name + '/') in extfiles):
                    extfilessub = []        # extract whole sub directory
                else:
                    for f in extfiles:
                        if f.startswith(dirname + ent.name + '/'):
                            break           # extract specified files in sub directory
                    else:
                        continue
                try:
                    os.mkdir(dirname + ent.name)
                except FileExistsError as e:
                    pass
                self.extractxdf(ent.readdir(), dirname + ent.name + '/', extfilessub)
            else:
                if not extfiles or ((dirname + ent.name) in extfiles):
                    print(dirname + ent.name)
                    ent.readfile(dirname)

    def createxdf(self, dir, dirname, files):
        if files == [] or files == ['']:
            files = os.listdir(dirname)
        for f in files:
            if f == '.' or f == '..':
                continue
            if '/' in f:            # add specified files in sub directory
                i = f.find('/')
                df = f[:i]
                ff = f[i+1:]
                d = dir.mkdir(df)
                self.createxdf(d, dirname + df + '/', [ff])
            else:                   # add one file or whole sub directory
                try:
                    st = os.stat(dirname + f)
                except:
                    raise FileNotFound(dirname + f)
                if st[0] & 0o40000:     # directory
                    d = dir.mkdir(f)
                    self.createxdf(d, dirname + f + '/', [])
                else:
                    print(dirname + f)
                    dir.writefile(dirname, f)

    def flush(self):
        """Write FAT and directory data into new XDF file"""
        self.fat.flush()
        self.rootdir.flush()
        for d in self.subdirs:
            d.flush()


def usage():
    print(f'usage: {sys.argv[0]:s} t <xdf file>                         : List XDF contents')
    print(f'       {sys.argv[0]:s} x <xdf file> [<files>...]            : Extract XDF file')
    print(f'       {sys.argv[0]:s} c [<format>] <xdf file> [<files>...] : Create XDF file')
    print( 'format: (none)=2HD(1232KB) /5=2HC(1200KB) /8=2DD(640KB) /9=2DD(720KB) /4=2HQ(1440KB)')
    sys.exit(1)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        usage()

    try:
        if sys.argv[1] == 't' or sys.argv[1] == 'list':
            with open(sys.argv[2], 'rb') as f:
                d = XdfDisk(fhin=f)
                d.listxdf(d.rootdir, '.')
        elif sys.argv[1] == 'x' or sys.argv[1] == 'extract':
            extfiles = sys.argv[3:]
            with open(sys.argv[2], 'rb') as f:
                d = XdfDisk(fhin=f)
                d.extractxdf(d.rootdir, '', extfiles)
        elif sys.argv[1] == 'c' or sys.argv[1] == 'create':
            if sys.argv[2] in BPB:
                if len(sys.argv) < 3:
                    usage()
                format = sys.argv[2]
                fname = sys.argv[3]
                extfiles = sys.argv[4:]
            else:
                format = ''
                fname = sys.argv[2]
                extfiles = sys.argv[3:]
            with open(fname, 'wb') as f:
                d = XdfDisk(fhout=f, format=format)
                if extfiles:
                    d.createxdf(d.rootdir, '', extfiles)
                d.flush()
        else:
            usage()
    except (DiskFull, RootDirFull, FileNotFound) as e:
        print(sys.argv[0] + ': error: ' + str(e))
        sys.exit(1)

    sys.exit(0)
