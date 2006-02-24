/*
 * $Id: stdtypes.h,v 1.2 2005/10/26 17:35:55 openns Exp $
 * $Log: stdtypes.h,v $
 * Revision 1.2  2005/10/26 17:35:55  openns
 * Updates after testing following the 2nd ONSWG meeting.  Checked in from
 * Webb McDonald's implementation of 2005-09-07.
 *
 *
 * File:	stdtypes.h
 * Purpose:	Definitions for standard types
 * Date:	2004-10-23
 *
 * This is free source code, courtesy of the OpenNavigationSurface project.  Visit
 * the website http://www.opennavsurf.org
 *
 */

#ifndef __STDTYPES_H__
#define __STDTYPES_H__

#ifdef __cplusplus__
extern "C" {
#endif

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef int s32;
typedef short s16;
typedef signed char s8;
typedef double f64;
typedef float f32;

typedef enum {
	False = 0,
	True
} Bool;


#ifdef __cplusplus__
}
#endif

#endif
