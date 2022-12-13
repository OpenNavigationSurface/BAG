import os
import shutil
import tempfile
import re


WHITESPACE_REGEX = re.compile(r'\s+')


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

    def __del__(self):
        if os.path.exists(self.m_name):
            os.remove(self.m_name)


def replace_whitespace(input: str, *, replace_str: str = '') -> str:
    """
    Replace interior or exterior whitespace from a string
    :param input: String whose whitespace should be replaced
    :param replace_str: String to replace whitespace with;
        defaults to empty string (i.e., removal of whitespace)
    :return: String with whitespace replaced.
    """
    return WHITESPACE_REGEX.sub(replace_str, input)
