HOW TO COMPILE
--------------

Microsoft VisualStudio

1. edit 'config_cmake.bat' and run it.
2. do 'nmake.exe install'

GCC (Linux and OSX)

1. edit 'config_cmake.sh' and run it.
2. do 'make install'



Note on build rpm file in Linux
-------------------------------

When you see an error like

error: Installed (but unpackaged) file(s) found:
   ... .../site-packages/dicom.pyo
   
a workaround resolution is modifying '{pythondir}/distutils/command/build_rpm.py'

1. find lines like this
   ('install', 'install_script',
    ("%s install "
     "--root=$RPM_BUILD_ROOT"
     
2. modify line like this
   ('install', 'install_script',
    ("%s install -O1 "
     "--root=$RPM_BUILD_ROOT"

ref. https://bugzilla.redhat.com/show_bug.cgi?id=530685