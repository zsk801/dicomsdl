import dicom

def ex_dicom_to_pil(fn):
    dfo = dicom.dicomfile(fn)
    im = dfo.to_pil_image()
    im.show()
    
def ex_dicom_to_numpy(fn):
    dfo = dicom.dicomfile(fn)
    a = dfo.to_numpy_array()
    