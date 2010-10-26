#-*- coding: MS949 -*- 
'''
  Copyright 2010, Kim, Tae-Sung. All rights reserved.
  See copyright.txt for details
'''
__author__ = 'Kim Tae-Sung'
__id__ = '$Id$'


import dicom
import os, sys
import re, glob

_NCCNMUIDROOT_ = '1.2.826.0.1.3680043.8.417'

if len(sys.argv) == 1:
    print 'USAGE: %s /write [elmod.def] [dicomfile ...]'%(sys.argv[0])
    print \
r'''       [elmod.def] Definition file define how elements should be modified
       /write -- Write modified dicom file name with fn+'.mod'

Example of .def file
'0020000d' <= UI, 1.2.826.0.1.3680043.8.417.1000['00080020']1['00080030']1
'00100020' <= LO, string value # string
'00181130' <= DS, 78 # decimal value
'00200032' <= DS, [-175.0, -175.0, -175.0] # value list

Remark:
['TAG'] string will be replaced by actual value of element with given tag
1.2.826.0.1.3680043.8.417.700000 - root uid value for angel lab
'''  
    sys.exit(-1)
    
opt_write = True if '/write' in sys.argv else False
if opt_write: sys.argv.remove('/write')

moddeffn = sys.argv[1]
fnlist = sys.argv[2:]

if not moddeffn.endswith('.def'):
    print 'Element modification definition file should have extension ".def"'

# PROCESS DEF FILE
kvlist = []
with file(moddeffn, 'r') as f:
    for l in f.readlines():
        if not l: continue
        k, dummy, v = l.partition('<=')
        if dummy != '<=':
            print 'ERROR IN PARSING LINE', l
            continue
        vr,dummy,v = v.partition(',')
        vr = vr.strip()
        if dummy != ',' or len(vr) != 2:
            print 'ERROR IN PARSING LINE', l
            continue
        try:
            vr = eval('dicom.VR_'+vr)
        except:
            print 'ERROR IN PARSING LINE', l
            continue
        kvlist.append([k.strip(' "\' '), vr, v.split('#')[0].strip()])

# PROCESS FILE LIST ARG
morefnlist = []
for fn in fnlist[:]:
    if '*' in fn or '?' in fn:
        morefnlist += glob.glob(fn)
        fnlist.remove(fn)
fnlist += morefnlist

# PROCESS EACH FILES
replacer = re.compile(r'\[\'[0-9a-fA-F\.]*\'\]')
for fn in fnlist:
    try:
        df = dicom.dicomfile(fn)
    except:
        print 'Error while loading '+fn
        continue
    
    for tag, vr, v in kvlist:
        print fn+':'+tag+' =',
        v = replacer.sub(lambda m: df.get_dataelement(m.group()[2:-2]).to_string(), v)
        el = df.get_dataelement(tag)
        print el.to_string(),'=>',
        el = df.add_dataelement(tag)
        if vr in [dicom.VR_SL, dicom.VR_SS, dicom.VR_UL, dicom.VR_US,
                  dicom.VR_AT, dicom.VR_IS]:
            if v[0] == '[' and v[-1] == ']': # this is list
                v = map(int, v[1:-1].split(','))
                el.from_long_values(v)
            else: # a single int value
                v = int(v)
                el.from_long(v)
        elif vr in [dicom.VR_FL, dicom.VR_FD, dicom.VR_DS]:
            if v[0] == '[' and v[-1] == ']': # this is list
                v = map(float, v[1:-1].split(','))
                el.from_double_values(v)
            else: # a single int value
                v = float(v)
                el.from_double(v)
        else: # string  
            el.from_string(v)
        print el.to_string()
        
    if opt_write:
        _path, _fn = os.path.split(fn)
        if not os.path.isdir(_path+'.mod'):
            os.makedirs(_path+'.mod')
        print 'WRITING TO ', _path+'.mod\\'+_fn
        df.write_to_file(_path+'.mod\\'+_fn)
            