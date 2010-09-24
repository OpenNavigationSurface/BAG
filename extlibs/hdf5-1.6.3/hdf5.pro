###############################################################################
#                   IVS Application Build Project (v1.0)                      #
###############################################################################
#                                                                             
###                       DO NOT EDIT THIS SECTION                          
#                                                                             
### Retrieve the application master project which contains shared OPENNS definations,
### includes, and other parameters to support platform independent building.
#
include (../libmaster.pro)
#
INCLUDEPATH += $$INC_OPENNSALL
###############################################################################
#
###                  LIBRARY SPECIFIC SECTION (Edit Here)                          
#

INCLUDEPATH	+= ../zlib ../szip

win32 {
	DEFINES         += WIN32
}

HEADERS += hdf5.h \
        H5ACprivate.h \
        H5ACpublic.h \
        H5Apkg.h \
        H5Aprivate.h \
        H5Apublic.h \
        H5Bpkg.h \
        H5Bprivate.h \
        H5Bpublic.h \
        H5config.h \
        H5Dpkg.h \
        H5Dprivate.h \
        H5Dpublic.h \
        H5Eprivate.h \
        H5Epublic.h \
        H5FDfamily.h \
        H5FDmulti.h \
        H5FDprivate.h \
        H5FDpublic.h \
        H5Fprivate.h \
        H5Fpublic.h \
        H5Gprivate.h \
        H5Gpublic.h \
        H5HGprivate.h \
        H5HGpublic.h \
        H5HLprivate.h \
        H5HLpublic.h \
        H5Iprivate.h \
        H5Ipublic.h \
        H5MFprivate.h \
        H5MMprivate.h \
        H5MMpublic.h \
        H5Oprivate.h \
        H5Opublic.h \
        H5Pprivate.h \
        H5Ppublic.h \
        H5private.h \
        H5RCprivate.h \
        H5Rprivate.h \
        H5Rpublic.h \
        H5Sprivate.h \
        H5Spublic.h \
        H5TBprivate.h \
        H5Tpkg.h \
        H5Tprivate.h \
        H5Tpublic.h \
        H5Vprivate.h \
        H5Zprivate.h \
        H5Zpublic.h
                
SOURCES += H5.c \
           H5A.c \
           H5AC.c \
           H5B.c \
           H5D.c \
           H5Dcompact.c \
           H5Dcontig.c \
           H5Dio.c \
           H5Distore.c \
           H5Dseq.c \
           H5Dtest.c \
           H5E.c \
           H5F.c \
           H5FD.c \
           H5FDcore.c \
           H5FDfamily.c \
           H5FDgass.c \
           H5FDlog.c \
           H5FDmpio.c \
           H5FDmulti.c \
           H5FDsec2.c \
           H5FDsrb.c \
           H5FDstdio.c \
           H5FDstream.c \
           H5FL.c \
           H5FO.c \
           H5G.c \
           H5Gent.c \
           H5Gnode.c \
           H5Gstab.c \
           H5HG.c \
           H5HL.c \
           H5HP.c \
           H5I.c \
           H5MF.c \
           H5MM.c \
           H5O.c \
           H5Oattr.c \
           H5Obogus.c \
           H5Ocont.c \
           H5Odtype.c \
           H5Oefl.c \
           H5Ofill.c \
           H5Olayout.c \
           H5Omtime.c \
           H5Oname.c \
           H5Onull.c \
           H5Opline.c \
           H5Osdspace.c \
           H5Oshared.c \
           H5Ostab.c \
           H5P.c \
           H5Pdcpl.c \
           H5Pdxpl.c \
           H5Pfapl.c \
           H5Pfcpl.c \
           H5Ptest.c \
           H5R.c \
           H5RC.c \
           H5RS.c \
           H5S.c \
           H5Sall.c \
           H5Shyper.c \
           H5Smpio.c \
           H5Snone.c \
           H5Spoint.c \
           H5Sselect.c \
           H5ST.c \
           H5Stest.c \
           H5T.c \
           H5Tarray.c \
           H5TB.c \
           H5Tbit.c \
           H5Tcommit.c \
           H5Tcompound.c \
           H5Tconv.c \
           H5Tcset.c \
           H5Tenum.c \
           H5Tfields.c \
           H5Tfixed.c \
           H5Tfloat.c \
           H5Tinit.c \
           H5Tnative.c \
           H5Toffset.c \
           H5Topaque.c \
           H5Torder.c \
           H5Tpad.c \
           H5Tprecis.c \
           H5Tstrpad.c \
           H5Tvlen.c \
           H5V.c \
           H5Z.c \
           H5Zdeflate.c \
           H5Zfletcher32.c \
           H5Zshuffle.c \
           H5Zszip.c
           