import dicom
#zipfn = r'E:\StorageA\Program.Free\zip\zip30.zip'
#fnlist = dicom.zipfile_get_list(zipfn).split()
#print '-'*80
#print fnlist[4]
#print '-'*80
#print dicom.zipfile_extract_file(zipfn, fnlist[4])


dr = dicom.dicomdir()

import os
basepath = r'\lab\img\images\PET'
basepath = r'\lab\sample'
basepath = os.path.abspath(basepath)
for root, dns, fns in os.walk(basepath):
    for fn in fns:
        dr.add_dicomfile(root+'\\'+fn, root[len(basepath)+1:]+'\\'+fn)
    
#dr.add_dicomfile(r'c:\lab\sample\img001.dcm', 'img001.dcm')

dr.write_to_file('test.dcm')