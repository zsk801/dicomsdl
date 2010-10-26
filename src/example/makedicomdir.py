#-*- coding: MS949 -*- 
'''
  Copyright 2010, Kim, Tae-Sung. All rights reserved.
  See copyright.txt for details
'''
__author__ = 'Kim Tae-Sung'
__id__ = '$Id$'


import sys
import dicom

if len(sys.argv) < 3:
    print 'python makedicomdir.py basedirpath dicomdir_name'
    sys.exit()

try:
    ddobj = dicom.dicomfile()
    dicom.build_dicomdir(ddobj, sys.argv[1])
    ddobj.write_to_file(sys.argv[2])
    print "DICOMDIR file is successfully built and "\
          "is written to '%s'."%(sys.argv[2]) 
except RuntimeError, e:
    print 'Error in build DICOMDIR'
    print e