#-*- coding: MS949 -*- 
'''
  Copyright 2010, Kim, Tae-Sung. All rights reserved.
  See copyright.txt for details
'''
__author__ = 'Kim Tae-Sung'
__id__ = '$Id$'


import sys
import dicom

if len(sys.argv) < 2:
    print 'python dumpdicomdir.py dicomdir_name'
    sys.exit()

dfo = dicom.dicomfile(sys.argv[1])
ds = dfo['OffsetOfTheFirstDirectoryRecordOfTheRootDirectoryEntity'].to_dataset()

def proc_branch(ds, prefix=''):
    while True:
        print prefix, #'%04XH'%(ds['RecordInUseFlag'].get_value()), # retired!
        print ds['DirectoryRecordType'].get_value().__repr__(),
        
        if ds['DirectoryRecordType'].get_value() == 'PATIENT':
            print ds['PatientName'].get_value().__repr__(),
            print ds['PatientID'].get_value().__repr__(),
        elif ds['DirectoryRecordType'].get_value() == 'STUDY':
            print ds['StudyDate'].get_value().__repr__(),
            print ds['StudyTime'].get_value().__repr__(),
            print ds['StudyID'].get_value().__repr__(),
            print ds['StudyDescription'].get_value().__repr__(), # optional
        elif ds['DirectoryRecordType'].get_value() == 'SERIES':            
            print ds['Modality'].get_value().__repr__(),
            print ds['SeriesNumber'].get_value().__repr__(),
        elif ds['DirectoryRecordType'].get_value() in ['IMAGE','DATASET']:
            print ds['ReferencedFileID'].get_value().__repr__(),
            print ds['InstanceNumber'].get_value().__repr__(),
        print
        
        leafds = ds['OffsetOfReferencedLowerLevelDirectoryEntity'].to_dataset()
        if leafds:
            proc_branch(leafds, prefix+'  ')
        ds = ds['OffsetOfTheNextDirectoryRecord'].to_dataset()
        if not ds: break

proc_branch(ds)