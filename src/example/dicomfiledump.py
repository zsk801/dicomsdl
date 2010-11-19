#-*- coding: MS949 -*- 
'''
  Copyright 2010, Kim, Tae-Sung. All rights reserved.
  See copyright.txt for details
'''
__author__ = 'Kim Tae-Sung'
__id__ = '$Id$'

import sys
import glob
import dicom

def extractimage():
    pass
#def extractimage(filename, outputfilename):
#    dcmfile = dicom.file(filename)
#    imgelement = dcmfile.elementAt('0x7fe00010')
#    if not imgelement.isvalid():
#        imgelement = dcmfile.elementAt('00540410.0.7fe00010')
#    if not imgelement.isvalid():
#        print "NO IMAGE DATA IS FOUND "\
#              "- I.E. NO ELEMENT WITH 0x7FE00010 or 00540410.0.7FE00010 TAG"
#        return
#        
#    if imgelement.vr == dicom.VR_PX:
#        img = imgelement.asImageSequence().itemAt(1)
#    else:
#        img = imgelement.asString()
#        
#    fout = file(outputfilename, 'wb')
#    fout.write(img)
#    fout.close()
#    
#    print "WRITE IMAGE ELEMENT VALUE IN (%s) TO (%s)"\
#            %(filename,outputfilename)

def extract_image(fn, dfo):
    imgelement = dfo.get_dataelement(0x7fe00010)
    if not imgelement.is_valid():
        imgelement = dfo.get_dataelement('00540410.0.7fe00010')
        
    if not imgelement.is_valid():
        print fn+" : NO IMAGE DATA ELEMENT IS FOUND; "\
              "I.E. NO ELEMENT WITH 0x7FE00010 OR 00540410.0.7FE00010 TAG"
        return
    
    if imgelement.vr == 'PX':
        print 'PXXXXXXXXXXXXXXXX'
        pass
    else:
        pass
    

        
def processfile_3(fn, opt):
    dfo = dicom.open_dicomfile(fn, dicom.OPT_LOAD_CONTINUE_ON_ERROR)
    
    if opt['opt_ignore']:
        if dfo and not dicom.get_error_message():
            print dfo.dump_string(fn+' : ').rstrip()
    else:
        if dfo:
            if not opt['opt_quite']:
                print dfo.dump_string(fn+' : ').rstrip()
                
            errmsg = dicom.get_error_message()
            if errmsg:
                print fn+' : ** ERROR IN LOADING A DICOM FILE; '+errmsg
                
        else:
            print fn+' : ** NOT A DICOM FILE'
            
    if dfo and opt['opt_extract']:
        extract_image(fn, dfo)

    
def processfile_2(fn, opt):
    # expand filenames in zipped file
    
    if fn.endswith('zip'): # .dicomzip or .zip
        s = dicom.zipfile_get_list(fn)
        if s:
            for zfn in s.splitlines():
                processfile_3('zip:///'+fn+':'+zfn, opt)
    else:
        processfile_3(fn, opt)

def processfile_1(fn, opt):
    # expand filenames containing wild card
    
    if '*' in fn or '?' in fn:
        for f in glob.glob(fn):
            processfile_2(f, opt)
    else:
        processfile_2(fn, opt)

if len(sys.argv) == 1:
    print 'USAGE: %s [dicomfile.dcm or zipped dicomfile.zip] [-q] [-e] [-w]'%(sys.argv[0])
    print '   -i - ignore damaged files and non-DICOM formatted files'
    print '   -e - extract image data in dicom file into [filename+.raw].' 
    print '   -q - don\'t display dump.'
     
else:
    opt = {}
    opt['opt_ignore'] = True if '-i' in sys.argv else False
    if opt['opt_ignore']: sys.argv.remove('-i')
    opt['opt_extract'] = True if '-e' in sys.argv else False
    if opt['opt_extract']: sys.argv.remove('-e')
    opt['opt_quite'] = True if '-q' in sys.argv else False
    if opt['opt_quite']: sys.argv.remove('-q')
    
    for f in sys.argv[1:]:
        processfile_1(f, opt)

    
#    for fn in fnlist:
#        if ':::' in fn:
#            zipfn, entry = fn.split(':::')[:2]
#            zf = zipfile.ZipFile(zipfn)
#            dcmstr = zf.read(entry)            
#            zf.close()
#            try:
#                dcm = dicom.file(len(dcmstr), dcmstr)
#            except RuntimeError, e:
#                print 'ERROR IN {0}: {1}'.format(fn, e)
#                continue
#        else:
#            try:
#                dcm = dicom.file(fn)
#            except RuntimeError, e:
#                print 'ERROR IN {0}: {1}'.format(fn, e)
#                continue
#            
#        if not opt_quite:
#            if opt_multifile:
#                dummy, _fn = os.path.split(fn)
#                for l in dcm.dump().splitlines():
#                    print _fn, ':', l
#            else:
#                print dcm.dump()
#        if opt_extract:
#            extractimage(fn, fn+'.raw')
#    
