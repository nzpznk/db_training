import zipfile

ofile = zipfile.ZipFile('submit.zip', 'w', zipfile.ZIP_STORED)

ofile.write('./SimSearcher.h')
ofile.write('./SimSearcher.cpp')
ofile.write('./TrieInvertedList.h')
ofile.write('./TrieInvertedList.cpp')

ofile.close()
