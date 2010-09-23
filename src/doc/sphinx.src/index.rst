.. DICOM Software Development Library documentation master file, created by
   sphinx-quickstart on Mon Sep 13 17:40:21 2010.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to DICOM Software Development Library's documentation!
==============================================================

Introduction
------------
   Digital Imaging and Communications in Medicine (DICOM) is a standard for
   managing informations in medical imaging developed by American College of
   Radiology (ACR) and National Electrical Manufacturers Association (NEMA).
   It defines a file format and a communication protocol over network.

   DICOM SDL is a software developed libraries for easy and quick development
   of an application managing DICOM formatted files. DICOM SDL is written in C++
   and it allows to make programs that read, modify, write DICOM formatted files
   without in depth knowledge of DICOM.

   DICOM SDL provides an extension module for python and you may build scripts
   with python.

   DICOM SDL can
  
   * read/modify/write DICOM formatted files.
   * read/modify/write medical images in DICOM file, if file encodes in
     raw format, jpeg/jpeg2000 format.
  
   DICOM SDL is especially optimized for reading lots of DICOM formatted files
   quickly, and would be very useful for scanning and processing huge numbers of
   DICOM files.

   DICOM SDL cannot
  
   * send/receive DICOM over network.
   * read files encodes in RLE format and JPEG-LS format


Documentations
--------------

.. toctree::
   :maxdepth: 2
   
   installing
   quickstart
   coderecipes
   api


License
-------

DICOMSDL is licensed under the new BSD license.

...........

DICOMSDL includes several open source programs. Details are in copyright.txt.


To Do
-----

In version 2, codes for DICOM network capability will be added.

