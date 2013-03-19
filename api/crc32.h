/*! \file crc32.h
 * \brief Declaration of CRC32 checksums for byte buffers.
 *
 * This code was sourced from the Samba project V3.0.0 on 2004-09-26.
 */

/* 
 * Copyright Francesco Ferrara, 1998 <francesco@aerra.it> 
 *
 * Used by kind permission, 14th October 1998. http://www.aerre.it/francesco
 *
 *
 * This was sourced from the Samba project, www.samba.org, V3.0.0.
 * 2004-09-26.
 */

#ifndef __CRC32_H__
#define __CRC32_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "onscrypto.h"

extern u32 crc32_calc_buffer( const char *buffer, u32 count);

#ifdef __cplusplus
}
#endif

#endif
