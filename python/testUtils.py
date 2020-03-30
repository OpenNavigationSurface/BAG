import tempfile, os, shutil

class RandomFileGuard:

    m_name = ""

    # if type is "name" a random filename will be generated, but no file opened. input param not used.
    # else a temp file will be opened. 
    #   if type is "file" the file pointed to by input param will be copied to the temp file.
    #   if type is "string" the contents of the input string param will be copied into the temp file.
    # the destructor deletes the file in each case.
    def __init__(self, type, input = ""):
        if type == "name":
            tmp_dir = tempfile._get_default_tempdir()
            name = next(tempfile._get_candidate_names())
            self.m_name = os.path.join(tmp_dir, name)
        elif type == "file":
            tempFile = tempfile.NamedTemporaryFile(delete=False, mode='w+t')
            self.m_name = tempFile.name
            if os.path.exists(input):
                shutil.copyfile(input, self.m_name)
            tempFile.close()
        elif type == "string":
            tempFile = tempfile.NamedTemporaryFile(delete=False, mode='w+t')
            self.m_name = tempFile.name
            tempFile.writelines(input)
            tempFile.close()
        else:
            self.m_name = ""
            

    def getName(self):
        return self.m_name

    #def __init__(self):
    #    tmp_dir = tempfile._get_default_tempdir()
    #    name = next(tempfile._get_candidate_names())
    #    self.m_name = os.path.join(tmp_dir, name)

    #def copyFile(self, fileToCopy):
    #    tempFile = tempfile.NamedTemporaryFile(delete=False, mode='w+t')
    #    self.m_name = tempFile.name
    #    shutil.copyfile(fileToCopy, self.m_name) 
    #    tempFile.close()

    #def copyString(self, stringToCopy):
    #    tempFile = tempfile.NamedTemporaryFile(delete=False, mode='w+t')
    #    self.m_name = tempFile.name
    #    tempFile.writelines(stringToCopy)
    #    tempFile.close()

    def __del__(self):
        #print(" RandomFileGuard.Py ")
        if os.path.exists(self.m_name):
            os.remove(self.m_name)


    #def __init__(self):
    #    self.m_tempFile = tempfile.NamedTemporaryFile(delete=False, mode='w+t')
    #    #f = open(self.m_tempFile.name, "w")
    #    #self.m_tempFile.close()

    #def copyFile(self, fileToCopy):
    #    #f = open(self.m_tempFile.name, "w")
    #    #f.seek(0)
    #    shutil.copyfile(fileToCopy, self.m_tempFile.name)
    #    #f.close()
    #    self.m_tempFile.close()

    #def copyString(self, stringToCopy):
    #    self.m_tempFile.writelines(stringToCopy)
    #    self.m_tempFile.close()

        #f = open(self.m_tempFile.name, "w")
        #f.seek(0)
        #f.write(stringToCopy)
        #f.close()
        #self.m_tempFile.close()

    #def getForWriting(self):
    #    self.m_tempFile.close()
    #    return self.m_tempFile.name


    #def getName(self):
    #    return self.m_tempFile.name

    #def __del__(self):
    #    #print(" RandomFileGuard.Py ")
    #    if os.path.exists(self.m_tempFile.name):
    #        self.m_tempFile.close()
    #        os.remove(self.m_tempFile.name)



def testRandomFile_WriteDirect():
    tmpFile = RandomFileGuard()
    print(tmpFile.getName())
    f = open(tmpFile.getName(), "w")
    tmpFile.m_tempFile.write("Test")
    tmpFile.m_tempFile.close()
    f = open(tmpFile.getName(), "r")
    print(f.read())
    f.close()

def testRandomFile_Open():
    tmpFile = RandomFileGuard()
    print(tmpFile.getName())
    f = open(tmpFile.getName(), "w")
    f.write("Test")
    f.close()
    f = open(tmpFile.getName(), "r")
    print(f.read())
    f.close()
    
#testRandomFile_Open()
#testRandomFile_WriteDirect()