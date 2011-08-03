rmdir /s/q html latex
makedoxyfile.py
doxygen Doxyfile
"c:\Program Files (x86)\HTML Help Workshop\hhc.exe" html\index.hhp
copy html\*.chm .
@REM cd latex & pdflatex refman.tex & makeindex refman.idx & pdflatex refman.tex & cd ..
