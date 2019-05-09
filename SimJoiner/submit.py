import zipfile

ofile = zipfile.ZipFile('submit.zip', 'w', zipfile.ZIP_STORED)

ofile.write('./SimJoiner.h')
ofile.write('./SimJoiner.cpp')
# ofile.write('./TrieInvertedList.h')
# ofile.write('./TrieInvertedList.cpp')

ofile.close()
