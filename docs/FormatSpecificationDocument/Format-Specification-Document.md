# Format Specification Document

Description of the Bathymetric Attributed Grid Object, version 2.0
Release 2.0.1 (2022-12-01)

The release history is [here](RevisionHistory.md).

## Contents

[Overview of the BAG Structure](FSD-BAGStructure.md)

[Encapsulation](FSD-Encapsulation.md)

[Axiomatic Definitions](FSD-AxiomaticDefs.md)

[The BAG API](FSD-BAGAPI.md)

[BAG Architecture Review Board](FSD-BAGARB.md)

[Revision Control and Code Availability](FSD-RevisionControl.md)

[How to Apply for an Extension/Bug Fix](FSD-Extensions.md)

[Glossary of Acronyms](FSD-Glossary.md)

[References](FSD-References.md)

[Appendix A](FSD-Appendices.md)

## Introduction

### Motivation

The design of the ‘Navigation Surface’ concept [2, 3] envisioned a new structure for hydrographic product creation from basic surveys.  Relying on a database to hold all the original data, processed into the form of grids of the best available representation of the true nature of the seafloor, the Navigation Surface concept extracts whatever data is required for a particular product and, through automatic manipulation and/or cartography, constructs a product suitable for a particular purpose.

Depending on task, this might be a uniform grid at the best available resolution (e.g., for flow modeling), or a hydrographic vector-based chart for safety of navigation.  In order to make this possible, the database must contain the best available data for each area, at the highest achievable resolution, and will contain data from a number of different sources, potentially from a number of different software packages.

In order to make this database possible, there is a need for a uniform file format that allows data to be passed between software packages, and between agencies involved in the collection, processing and dissemination of the data, while maintaining the integrity of the data and metadata at all times. The file format should also be sufficiently flexible to support processing of data without format conversion where possible, since format conversion is an extremely expensive process.  The Open Navigation Surface (ONS) Project has as its mandate the task of building such a data file format, and developing and maintaining the source code for a software library to read and write this format so that adoption of the technology is eased for any developer.  The source code library has been developed on the Open Source model [4] so that the source code is freely available; all of the members of the ONS Working Group (ONSWG) and their respective employers have provided their effort on this basis.

This document describes the format, the Application Programming Interface (API) associated with it, and the conduct of the ONSWG and its derivatives that maintain the BAG file format source code.  It also describes where to find the code, how to obtain access to the current source tree, how to apply for extensions to the format, and how to provide bug-fixes.  The development effort is entirely voluntary, and participation from other interested parties is actively encouraged.

### Nomenclature

The term “Navigation Surface” was coined to describe the combination of a data object representing the bathymetry and associated uncertainty, and the methods by which such objects could be manipulated, combined and used for a number of tasks, including products in support of safety of navigation.  These multiple goals have led to some uncertainty about what exactly constitutes a Navigation Surface.  To avoid any further confusion, a revised nomenclature has been designed.

In the ONS model, a unit of bathymetry is termed a Bathymetric Attributed Grid (BAG).  A single BAG object represents one contiguous area of the skin of the Earth at a single resolution, but can represent data at any stage of the process from raw grid to final product.  The name Navigation Surface (NS) is reserved for a final product BAG destined specifically for safety-of-navigation purposes.  The status of any particular BAG is distinguished solely by the certification section of metadata embedded in the file.

### Properties of the BAG

The Navigation Surface concept requires that in addition to estimation of depth, an estimate of the uncertainty associated with the depth must be computed and preserved.  In order to make the system suitable to support safety of navigation applications, there is a means to over-ride any automatically constructed depth estimates with ‘Hydrographer Privilege’, (essentially, a means to specify directly the depth determined by a human observer as being the most significant in the area - irrespective of any statistical evidence to the contrary).

There is also the requirement to provide data on the data (i.e. metadata, which describe all aspects of the data’s life, from methods of capture to processing methods, and from geospatial extents to responsible party.

Finally, there must be a means to certify that the data in the file has been inspected by someone with appropriate experience and authority, that the data has been verified as suitable for some specific purpose, and that the file has not changed since this certification was made: in essence, a digital replacement for the Hydrographer’s signature.

Means to incorporate all of these requirements in a portable, extensible, platform neutral, vendor neutral format are provided.

### Status of the Project

The ONSWG maintain both a web-site, [here](http://www.opennavsurf.org), which contains the current recommended release of the source code and supporting documentation, and a BitBucket hosted Git repository, which contains the current development version of the source code.  Details of availability of the code are given in Section 7.  There are no mechanisms for ‘nightly builds’ or other intermediate releases.

The library is designed to be built from source code, including all of the component libraries that the BAG uses.  All of the component libraries are covered under Open Source style licenses, although not all the licenses are alike. Users should ensure that they are compliant with the terms of the appropriate license before using the component libraries.  The ONS source code library itself is Open Source in the sense that you are free to modify, adapt and otherwise reuse the source code, including the construction of derivative products based on the source code.  However, if a bug is found or there is a way to improve part of the library, it is requested that this be communicated to the ONSWG (as described in Section 8).  Extensions to the library (e.g., new HDF groups (Section 8.2) or additions to metadata elements) shall not be added without permission of the BAG Architecture Review Board (Section 6).  Applications for extensions are also covered in Section 8.

### History

The idea for what became the Open Navigation Surface Project had been discussed within the US hydrographic community for some time, but first crystallized in late 2003 through comments of M. Paton at IVS3D Ltd., which were adopted by B. Calder at CCOM/JHC with the intent of acting as an independent third-party broker for the development of the library.  The ONSWG first met in early 2004, and outlined the requirements for the BAG file structure, the ethos of the project and the basic functionality of the source code library.  A presentation on the structure of the project was made at US Hydro 2005 in March, and development continued until the second ONSWG meeting in mid 2005, when the majority of the first release of the library was pulled together in a little under a week.  A second presentation, including demonstrations of the library linked into SAIC SABER, CARIS HIPS and IVS Fledermaus, was given at Shallow Survey 2005 in September and development of the library continued using e-mail and telephone conferencing until the first Candidate Release on 8 February 2006.  Following comments from users and further development, the first official release of the library was made on 8 April 2006.

### Identification

This document describes the Bathymetric Attributed Grid file format, version 2.0.1.  This version corresponds to the Git release tag “release-2.0.1”, which may be obtained from the Open Navigation Surface (ONS) Project on [GitHub](https://github.com/OpenNavigationSurface/BAG).

This release of the library supports, and has been tested on 64-bit versions of Linux, Windows 10, and macOS, Linux Mint.  The source code is written in C++14 and builds with CMake, and should therefore compile on most Unix-like systems if required.  Users who successfully compile the library on other platforms are urged to provide information on any modifications required to the project so that these platforms can be supported in future releases.

## [Next: Overview of the BAG Structure](FSD-BAGStructure.md)