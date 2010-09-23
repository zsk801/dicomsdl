Objects and Functions
=====================

Functions
---------

Opening and Closing a DICOM file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. cpp:function:: dicomfile* open_dicomfile(char *filename, opttype opt=0L)

   `open_dicomfile` read and parse DICOM file with filename, and return
   the pointer of dicomfile object. Returned dicomfile pointer need to be
   :cpp:func:`close_dicomfile()`. opt is option for reading;
   options need to be `or`. Return `NULL` on error, but if
   option :c:data:`OPT_READ_CONTINUE_ON_ERROR` is given, partially
   parsed object will be returned and :cpp:func:`get_error_message()`
   will return error message.

.. cpp:function:: dicomfile* open_dicomfile_from_memory(char *data, int datasize, opttype opt=0L)

   `open_dicomfile_from_memory` read DICOM file in the memory.
   opt is option for reading. options need to be `or`. 
   Returned dicomfile pointer need to be :cpp:func:`close_dicomfile()`.
   Return `NULL` on error, but if option OPT_READ_CONTINUE_ON_ERROR is given,
   partially parsed object will be returned and :cpp:func:`get_error_message()`
   will return error message.

.. cpp:function:: void close_dicomfile(dicomfile *df)

    destroy dicomfile object.

.. cpp:type:: opttype

   options for open_dicomfile and open_dicomfile_from_memory

   .. c:var:: opttype OPT_READ_PARTIAL_FILE
   
      Default, Read the part of a DICOM file, before the given `tag`.
      `tag` value should be given using `or`. By skipping the latter portion,
      loading a large numbers of DICOM file will be much faster. Following
      code will load all data elements with tag same or less than (0054,0040).
      
      .. highlightlang:: c++
      
      ::
      
         dicomfile *df = open_dicomfile("somefile.dcm", OPT_READ_PARTIAL_FILE | 0x00540400);
   
   .. c:var:: opttype OPT_READ_CONTINUE_ON_ERROR
   
      :cpp:func:`open_dicomfile()` and :cpp:func:`open_dicomfile_from_memory()`
      continue reading on error and partially
      loaded dicomfile object, rather than returning `NULL`.
      
   .. c:var:: opttype OPT_LOAD_DONOT_COPY_DATA
   
      Only for :cpp:func:`open_dicomfile_from_memory()`.   
      When :cpp:func:`open_dicomfile_from_memory()` load a dicomfile object
      from DICOM file image, it copies whole image and keeps it internally.
      Whenever you access data element's value, accessing function will
      reference internally kept DICOM file image.
      When `OPT_LOAD_DONOT_COPY_DATA` option is given,
      :cpp:func:`open_dicomfile_from_memory()` will not copy the image. That
      means if you try access data element's value after destroy original
      DICOM file image, application will crash. So, if you use optioin
      `OPT_LOAD_DONOT_COPY_DATA` original file image should be maintained
      until dicomfile object is deleted. 

      .. highlightlang:: c++
      
      ::
      
         char *fileimage;
         // .. loading fileimage from somewhere
         dicomfile *df = open_dicomfile_from_memory(fileimage, OPT_LOAD_DONOT_COPY_DATA);
         delete fileimage; // deleting original file image
         df->get_dataelement("PatientName")->to_string(); // CRASH!!!


.. cpp:function:: char *get_error_message()

   return error message, if previous operation had an error.

class dicomfile
---------------


class dataset
-------------

class dataelement
-----------------

class sequence
--------------

class pixelsequence
-------------------