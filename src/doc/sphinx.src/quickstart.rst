Quick Start
===========

A Brief Overview on DICOM
-------------------------

If you are not familiar with DICOM, read this before you go on.
There are several good webpages that introduce on DICOM.

((( TBD )))


C++ example 
-----------

This short example loads a DICOM formatted file and extract some informations
and image.

.. highlightlang:: c++

::

   #include "dicom.h"
   #include <string>
   
   int main()
   {
      dicom::dicomfile *df;
      df = dicom::open_dicomfile("img001.dcm");
      if (df) {
         // get some informations dicom file
         std::string studydesc = df->get_dataelement("StudyDescription")->to_string();
         // shorter form
         std::string studytime = df["StudyDate"];
         
         printf("Study Description = %s\n", studydsc.c_str());
         printf("Study Time = %s\n", studytime.c_str());
      
         // get images' informations in DICOM file
          
         int width, height,
         int precision, signedness, ncomponents, bytes_per_pixel;
         int nframes;
         df->get_image_info(&width, &height,
            &precision, &signedness, &ncomponents, &bytes_per_pixel,
            &nframes);
            
         // get pixeldata
            
         char *pixeldata;
         int pixeldata_length;
         df->get_pixeldata_a(&pixeldata, &pixeldata_length);
         
         if (pixeldata) {
            // do something ...
            
            free(pixeldata);
         }
   
         delete df;   
      } else {
         printf("Error message: %s\n", dicom::get_error_message());
      }
   }


Let's see what above codes do line by line.

.. highlightlang:: c++

::

   dicom::dicomfile *df;

A :cpp:class:`dicom::dicomfile` is an implementation of DICOM file object.



.. highlightlang:: c++

::

   df = dicom::open_dicomfile("img001.dcm");

:cpp:func:`dicom::open_dicomfile` reads, parses the file and
returns a DICOM file object.

Note) Returned DICOM file object need to be deleted by user.


.. highlightlang:: c++

::

   // get some informations dicom file
   std::string studydesc = df->get_dataelement("StudyDescription")->to_string();
   // shorter form
   std::string studytime = df["StudyDate"];

A DICOM file consists of several items contain informations about the DICOM file.
Each item in a DICOM file called 'data element' and you get a data element using
:cpp:func:`get_dataelement`. Item's value can be retrieved with functions
like :cpp:func:`to_string`, :cpp:func:`to_int`,
:cpp:func:`to_double_values`, according to the data type of item.
You may shorten code in second form.

.. highlightlang:: c++

::

   // get informations of image in dicom file
   get_image_info(&width, &height,
      &precision, &signedness, &ncomponents, &bytes_per_pixel,
      &nframes);   
   // get pixeldata
   df->get_pixeldata_a(&pixeldata, &pixeldata_length);

DICOM file object provide functions to extract image in the DICOM file.
:cpp:func:`get_image_info` takes informations related to image's geometry,
and :cpp:func:`get_pixeldata_a` extract image.

Note)
The suffix '_a' in the function name :cpp:func:`get_pixeldata_a` means
that function return a pointer that should be `free()` by user,
provided that the pointer is valid.


.. highlightlang:: c++

::

   printf("Error message: %s\n", dicom::get_error_message());

If an error is occured during reading/parsing a DICOM file,
:cpp:func:`open_dicomfile()` return NULL and you may get the error message
using :cpp:func:`dicom::get_error_message()`.  
After calling several functions return NULL on error,
:cpp:func:`dicom::get_error_message()` provide informations about the error.


Python example 
--------------

This is a python program does exact same thing.


.. highlightlang:: py

::

   import dicom
   
   df = dicom.open_dicomfile('img001.dcm')
   if df:
      # get some informations dicom file
      studydesc = df.get_dataelement('StudyDescription').to_string()
      # shorter form
      studytime = df["StudyDate"]
      
      print "Study Description =", studydsc
      print "Study Time =", studytime
   
      # get informations of image in dicom file
      
      ret = df.get_image_info()
      if ret:
         (width, height,
          precision, signedness, ncomponents, bytes_per_pixel,
          nframes) = ret
         
      # get pixeldata
      pixeldata = df.get_pixeldata()
      
      if pixeldata:
         pass # do something ...      
   else:
      print "Error message:", dicom.get_error_message()
   
