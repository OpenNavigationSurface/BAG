import testUtils

def testRandomFile_WriteDirect():
    tmpFile = testUtils.RandomFileGuard()
    #print(tmpFile.getName())
    tmpFile.m_tempFile.write("Test")
    tmpFile.m_tempFile.close()
    f = open(tmpFile.getName(), "r")
    #print(f.read())
    f.close()

def testRandomFile_Open():
    tmpFile = testUtils.RandomFileGuard()
    #print(tmpFile.getName())
    f = open(tmpFile.getName(), "w")
    f.write("Test")
    f.close()
    f = open(tmpFile.getName(), "r")
    #print(f.read())
    f.close()
    
testRandomFile_WriteDirect()
testRandomFile_Open()



import test_compounddatatype
import test_metadata
import test_dataset
import test_descriptor
