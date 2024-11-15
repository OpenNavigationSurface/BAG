import os

if os.name =='nt':
    if BAG_BIN := os.getenv('BAG_BIN'):
        os.add_dll_directory(BAG_BIN)
