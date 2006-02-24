###############################################################################
#                   IVS Application Build Project (v1.0)                      #
###############################################################################
#                                                                             
###                       DO NOT EDIT THIS SECTION                          
#                                                                             
### Retrieve the application master project which contains shared IVS definations,
### includes, and other parameters to support platform independent building.
#
include (../libmaster.pro)
#
INCLUDEPATH += $$INC_OPENNSALL
###############################################################################
#
###                  LIBRARY SPECIFIC SECTION (Edit Here)                          
#

DEFINES += BEE_STATIC

HEADERS += aes.h \
        aesopt.h \
        aes_be.h \
        aes_le.h \
        api.h \
        base64.h \
        beecrypt.h \
        blockmode.h \
        blockpad.h \
        blowfish.h \
        blowfishopt.h \
        dhaes.h \
        dldp.h \
        dlkp.h \
        dlpk.h \
        dlsvdp-dh.h \
        dsa.h \
        elgamal.h \
        endianness.h \
        entropy.h \
        fips186.h \
        gnu.h \
        hmac.h \
        hmacmd5.h \
        hmacsha1.h \
        hmacsha256.h \
        md5.h \
        memchunk.h \
        mp.h \
        mpbarrett.h \
        mpnumber.h \
        mpopt.h \
        mpprime.h \
        mtprng.h \
        pkcs1.h \
        pkcs12.h \
        rsa.h \
        rsakp.h \
        rsapk.h \
        sha1.h \
        sha1opt.h \
        sha256.h \
        timestamp.h \
        win.h
 
SOURCES += aes.c \
#        aesopt.c \
        base64.c \
        beecrypt.c \
        blockmode.c \
        blockpad.c \
        blowfish.c \
        dhaes.c \
        dldp.c \
        dlkp.c \
        dlpk.c \
        dlsvdp-dh.c \
        dsa.c \
        elgamal.c \
        endianness.c \
        entropy.c \
        fips186.c \
        hmac.c \
        hmacmd5.c \
        hmacsha1.c \
        hmacsha256.c \
        md5.c \
        memchunk.c \
        mp.c \
        mpbarrett.c \
        mpnumber.c \
        mpprime.c \
        mtprng.c \
        rsa.c \
        rsakp.c \
        rsapk.c \
        sha1.c \
        sha256.c \
        timestamp.c
        
#OBJECTS += mp-asm/blowfishopt.obj \
#        mp-asm/mpopt.obj \
#        mp-asm/sha1opt.obj
