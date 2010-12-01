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

###############################################################################
#
###                  LIBRARY SPECIFIC SECTION (Edit Here)                          
#

INCLUDEPATH	+= ../zlib-1.2.5 ../szip

win32 {
    CONFIG += staticlib
}

CONFIG -= qt
DEFINES -= UNICODE

#Use 1.6 definitions
DEFINES += H5_USE_16_API

# Input
HEADERS += hdf5.h H5api_adpt.h H5overflow.h H5pubconf.h H5public.h H5version.h \
        H5Apublic.h H5ACpublic.h \
        H5Cpublic.h H5Dpublic.h \
        H5Epubgen.h H5Epublic.h H5Fpublic.h H5FDpublic.h H5FDcore.h H5FDdirect.h \
        H5FDfamily.h H5FDlog.h H5FDmpi.h H5FDmpio.h H5FDmpiposix.h \
        H5FDmulti.h H5FDsec2.h  H5FDstdio.h \
        H5Gpublic.h  H5Ipublic.h H5Lpublic.h \
        H5MMpublic.h H5Opublic.h H5Ppublic.h H5Rpublic.h H5Spublic.h \
        H5Tpublic.h H5Zpublic.h \
        H5config.h H5pubconf.h
        
SOURCES +=  \
        H5.c H5checksum.c H5dbg.c H5system.c H5timer.c H5trace.c \
        H5A.c H5Abtree2.c H5Adense.c H5Adeprec.c H5Aint.c H5Atest.c \
        H5AC.c H5B.c H5Bcache.c H5Bdbg.c \
        H5B2.c H5B2cache.c H5B2dbg.c H5B2hdr.c H5B2int.c H5B2stat.c H5B2test.c \
        H5C.c H5CS.c \
        H5D.c H5Dbtree.c H5Dchunk.c H5Dcompact.c H5Dcontig.c H5Ddbg.c \
        H5Ddeprec.c H5Defl.c H5Dfill.c H5Dint.c \
        H5Dio.c H5Dlayout.c \
        H5Dmpio.c H5Doh.c H5Dscatgath.c H5Dselect.c H5Dtest.c \
        H5E.c H5Edeprec.c H5Eint.c \
        H5F.c H5Faccum.c H5Fdbg.c H5Ffake.c H5Fio.c H5Fmount.c H5Fmpi.c H5Fquery.c \
        H5Fsfile.c H5Fsuper.c H5Fsuper_cache.c H5Ftest.c \
        H5FD.c H5FDcore.c  \
        H5FDdirect.c H5FDfamily.c H5FDint.c H5FDlog.c H5FDmpi.c H5FDmpio.c \
        H5FDmpiposix.c H5FDmulti.c H5FDsec2.c H5FDspace.c H5FDstdio.c \
        H5FL.c H5FO.c H5FS.c H5FScache.c H5FSdbg.c H5FSsection.c H5FSstat.c H5FStest.c \
        H5G.c H5Gbtree2.c H5Gcache.c \
        H5Gcompact.c H5Gdense.c H5Gdeprec.c H5Gent.c \
        H5Gint.c H5Glink.c \
        H5Gloc.c H5Gname.c H5Gnode.c H5Gobj.c H5Goh.c H5Groot.c H5Gstab.c H5Gtest.c \
        H5Gtraverse.c \
        H5HF.c H5HFbtree2.c H5HFcache.c H5HFdbg.c H5HFdblock.c H5HFdtable.c \
        H5HFhdr.c H5HFhuge.c H5HFiblock.c H5HFiter.c H5HFman.c H5HFsection.c \
        H5HFspace.c H5HFstat.c H5HFtest.c H5HFtiny.c \
        H5HG.c H5HGcache.c H5HGdbg.c \
        H5HL.c H5HLcache.c H5HLdbg.c H5HLint.c \
        H5HP.c H5I.c H5L.c H5Lexternal.c H5lib_settings.c \
        H5MF.c H5MFaggr.c H5MFdbg.c H5MFsection.c \
        H5MM.c H5MP.c H5MPtest.c \
        H5O.c H5Oainfo.c H5Oalloc.c H5Oattr.c \
        H5Oattribute.c H5Obogus.c H5Obtreek.c H5Ocache.c H5Ochunk.c \
        H5Ocont.c H5Ocopy.c H5Odbg.c H5Odrvinfo.c H5Odtype.c H5Oefl.c \
        H5Ofill.c H5Oginfo.c \
        H5Olayout.c \
        H5Olinfo.c H5Olink.c H5Omessage.c H5Omtime.c \
        H5Oname.c H5Onull.c H5Opline.c H5Orefcount.c \
        H5Osdspace.c H5Oshared.c H5Ostab.c \
        H5Oshmesg.c H5Otest.c H5Ounknown.c \
        H5P.c H5Pacpl.c H5Pdapl.c H5Pdcpl.c \
        H5Pdeprec.c H5Pdxpl.c H5Pfapl.c H5Pfcpl.c H5Pfmpl.c \
        H5Pgcpl.c H5Pint.c \
        H5Plapl.c H5Plcpl.c H5Pocpl.c H5Pocpypl.c H5Pstrcpl.c H5Ptest.c \
        H5R.c H5Rdeprec.c \
        H5RC.c \
        H5RS.c \
        H5S.c H5Sall.c H5Sdbg.c H5Shyper.c H5Smpio.c H5Snone.c H5Spoint.c \
        H5Sselect.c H5Stest.c \
        H5SL.c \
        H5SM.c H5SMbtree2.c H5SMcache.c H5SMmessage.c H5SMtest.c \
        H5ST.c \
        H5T.c H5Tarray.c H5Tbit.c H5Tcommit.c H5Tcompound.c H5Tconv.c \
        H5Tcset.c H5Tdbg.c H5Tdeprec.c H5Tenum.c H5Tfields.c \
        H5Tfixed.c \
        H5Tfloat.c H5Tnative.c H5Toffset.c H5Toh.c \
        H5Topaque.c \
        H5Torder.c \
        H5Tpad.c H5Tprecis.c H5Tstrpad.c H5Tvisit.c H5Tvlen.c H5TS.c H5V.c H5WB.c H5Z.c  \
        H5Zdeflate.c H5Zfletcher32.c H5Znbit.c H5Zshuffle.c H5Zszip.c  \
        H5Zscaleoffset.c H5Ztrans.c \
        H5Tinit.c
        

win32 {
    HEADERS += H5FDwindows.h
    SOURCES += H5FDwindows.c
}

