import zipfile

ofile = zipfile.ZipFile('submit.zip', 'w', zipfile.ZIP_STORED)

ofile.write('./SimSearcher.h')
ofile.write('./SimSearcher.cpp')
ofile.write('./InvertedList.h')
ofile.write('./InvertedList.cpp')

ofile.close()
