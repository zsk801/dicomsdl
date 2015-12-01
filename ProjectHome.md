# Introduction #

Digital Imaging and Communications in Medicine (DICOM) is a standard for managing informations in medical imaging developed by American College of Radiology (ACR) and National Electrical Manufacturers Association (NEMA). It defines a file format and a communication protocol over network.

DICOM SDL is a software developed libraries for easy and quick development of an application managing DICOM formatted files. DICOM SDL is written in C++ and it allows to make programs that read, modify, write DICOM formatted files without in depth knowledge of DICOM.

DICOM SDL provides an extension module for python and you may build scripts with python.

DICOM SDL can

  * read/modify/write DICOM formatted files.
  * read/modify/write medical images in DICOM file, if file encodes in 	  raw format, jpeg/jpeg2000 format.

DICOM SDL is especially optimized for reading lots of DICOM formatted files quickly, and would be very useful for scanning and processing huge numbers of DICOM files.

DICOM SDL cannot

  * send/receive DICOM over network.
  * read files encodes in RLE format and JPEG-LS format



# Get codes #

You can get sources and binaries from http://code.google.com/p/dicomsdl.



## Installing ##

DICOM SDL needs cmake (www.cmake.org) to compile. DICOM SDL was successfully compiled and ran on following environments

  * Microsoft Windows x86/x64 using Microsoft visual studio 2008,
  * Linux x64 (CentOS) using gcc,
  * MacOS (ppc) using xcode 3.1.2

DICOM SDL is also expected to be compiled successfully at other Linux x86/x64 and intel MacOS.



# Getting Started #

This short example loads a dicom file and extract some example and image.

```
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
		std::string studytime = (*df)["StudyDate"];

		printf("Study Description = %s\n", studydesc.c_str());
		printf("Study Time = %s\n", studytime.c_str());

		// get informations of image in dicom file

		int width, height;
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
```


Let's see what above codes do line by line.

```
dicom::dicomfile *df;
```

A dicom::dicomfile is an implementation of DICOM file object.


```
df = dicom::open_dicomfile("img001.dcm");
```

dicom::open\_dicomfile(filename) reads, parses the file and returns a DICOM file object.

Note) Returned DICOM file object need to be deleted by user.

```
// get some informations dicom file
std::string studydesc = df->get_dataelement("StudyDescription")->to_string();
// shorter form
std::string studytime = df["StudyDate"];
```

A DICOM file consists of several items contain informations about the DICOM file. Each item in a DICOM file called 'data element' and you get a data element using get\_dataelement(tagname). Item's value can be retrieved with functions like
'to\_string()', 'to\_int()', 'to\_double\_values()', according to the data type of item. You may shorten code in second form.

```
// get informations of image in dicom file
get_image_info(&width, &height,
	&precision, &signedness, &ncomponents, &bytes_per_pixel,
	&nframes);
// get pixeldata
df->get_pixeldata_a(&pixeldata, &pixeldata_length);
```

DICOM file object provide functions to extract image in the DICOM file. get\_image\_info(...) takes informations related to image's geometry, and get\_pixeldata\_a(...) extract image.

> Note) 	The suffix `_a` in the function name `get_pixeldata_a(...)` means 	that function return a pointer that should be `free()` by user, 	provided that the pointer is valid.

```
printf("Error message: %s\n", dicom::get_error_message());
```

If an error is occured during reading/parsing a DICOM file, `open_dicomfile()` return NULL and you may get the error message using `dicom::get_error_message()`. After calling several functions return `NULL` on error,
`dicom::get_error_message()` provide informations about the error.


## Python example ##

This is a python program does exact same thing.

```
import dicom

df = dicom.open_dicomfile('img001.dcm')
if df:
	# get some informations dicom file
	studydesc = df.get_dataelement('StudyDescription').to_string()
	# shorter form
	studytime = df["StudyDate"]

	print "Study Description =", studydesc
	print "Study Time =", studytime

	# get informations of image in dicom file

	ret = df.get_image_info()
	if ret:
		(width, height,
		 precision, signedness, ncomponents, bytes_per_pixel,
		 int nframes) = ret

	# get pixeldata
	pixeldata = df.get_pixeldata()

	if pixeldata:
		pass # do something ...
else:
	print "Error message:", dicom.get_error_message()
```