# setup.py build bdist_wininst

from distutils.core import setup, Extension

ext = Extension(
                "_dicom",
                sources=["dicom.i"],
                swig_opts=['-c++'],
                include_dirs=['../../lib'],
                library_dirs=['../..'],
                libraries=['dicomsdk'],
                language='c++'
            )

setup(
      name='dicomsdl',
      version='0.01',
      description='DICOM Software Development Library',
      author='Kim, Tae-Sung',
      author_email='taesung.angel@gmail.com',
      url='http://code.google.com/p/dicomsdl/',
      py_modules=['dicom'],
      ext_modules=[ext]
    )
