/*! \file stdtypes.h
 * \brief Provide definitions of types for the project.
 *
 * This file provides a set of known types that avoid any weirdnesses in the
 * particular compiler that's being used (or at least a way to corral them all
 * in one place.
 */

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

typedef unsigned int u32;	/*!< Unsigned integers exactly 32-bits long */
typedef unsigned short u16;	/*!< Unsigned integers exactly 16-bits long */
typedef unsigned char u8;	/*!< Unsigned integers exactly 8-bits long (i.e., a byte) */
typedef int s32;			/*!< Signed integers exactly 32-bits long */
typedef short s16;			/*!< Signed integers exactly 16-bits long */
typedef signed char s8;		/*!< Signed integers exactly 8-bits long (i.e., a signed byte) */ 
typedef double f64;			/*!< Double precision floating point (IEEE754) numbers */
typedef float f32;			/*!< Single precision floating point (IEEE754) numbers */

/*! \brief Define a boolean logical enumerated type
 *
 * This defines a boolean type so that symbolic debuggers can show the names, rather than
 * just numbers.
 */

typedef enum {
	False = 0,	/*!< The False value definition */
	True		/*!< The True value definition */
} Bool;

typedef s32 bagError;

#ifdef __cplusplus__
}
#endif

#endif
