# setup.py build bdist_wininst

from distutils.core import setup, Extension

# get src root info
import os
p = '/'.join(os.path.abspath('.').split(os.path.sep))
p = p[:p.rfind('/wrapper/python')]

# extract version info from dicom.i
v =  [l for l in file('dicom.i').readlines() if 'DICOMSDL_VER' in l]

# compiler's option
extra_compile_args = []

if os.name == "nt":
    extra_compile_args += ["/EHsc"]      

ext = Extension(
                "_dicom",
                sources=["dicom.i"],
                swig_opts=['-c++', '-I%s/lib'%(p)],
                include_dirs=['%s/lib'%(p)],
                library_dirs=[p],
                libraries=['dicomsdl'],
                extra_compile_args = extra_compile_args,
                language='c++'
            )

setup(
      name='dicomsdl',
      version=v[0].split()[-1] if v else 'UNKNOWN',
      description='DICOM Software Development Library',
      author='Kim, Tae-Sung',
      author_email='taesung.angel@gmail.com',
      url='http://code.google.com/p/dicomsdl/',
      py_modules=['dicom'],
      ext_modules=[ext]
    )

