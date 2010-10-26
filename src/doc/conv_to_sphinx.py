import os, glob

def proc(fn, newfn):
    outf = file(newfn, 'wb')
    for line in open(fn).readlines():
        line = line.replace('\t', '   ')
        outf.write(line)
    outf.close()

for fn in glob.glob('*.txt'):
    newfn = 'sphinx.src\\'+fn[:-4]+'.rst'
    proc(fn, newfn)