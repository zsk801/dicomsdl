/*
 * dicomxml.cxx
 *
 *  Created on: 2011. 8. 3.
 *      Author: Angel
 */


#include "dicom.h"
#include "expat.h"

namespace dicom { //------------------------------------------------------

int Depth;

static void XMLCALL
start(void *data, const char *el, const char **attr)
{
  int i;

  for (i = 0; i < Depth; i++)
    printf("  ");

  printf("%s", el);

  for (i = 0; attr[i]; i += 2) {
    printf(" %s='%s'", attr[i], attr[i + 1]);
  }

  printf("\n");
  Depth++;
}

static void XMLCALL
end(void *data, const char *el)
{
  Depth--;
}

DLLEXPORT void test_func(char *s)
{
	XML_Parser p = XML_ParserCreate(NULL);

	XML_SetElementHandler(p, start, end);

	int len = strlen(s);

	if (XML_Parse(p, s, len, 1) == XML_STATUS_ERROR) {
		printf("Parse error at line %lu:\n%s\n",
			  XML_GetCurrentLineNumber(p),
			  XML_ErrorString(XML_GetErrorCode(p)));
	}

	XML_ParserFree(p);
}

} // namespace dicom -----------------------------------------------------
