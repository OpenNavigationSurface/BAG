# BAG Format Extensions

## List of Extensions

**Table 6.1. List of Extensions to the BAG format**

| Extension name                                                                | Notes                                                                               |
|:------------------------------------------------------------------------------|:------------------------------------------------------------------------------------|
| [A Variable Resolution Grid Extension for BAG Files](FSD-Extension-VRGrid.md) | Allow for piece-wise refinement of BAG cells to a higher resolution where necessary |


## How to Apply for an Extension/Bug Fix

The BAG FSD, of which this chapter is a part, is intended to be a living document, evolving as the requirements for the BAG format change. Over time, it is expected that extensions to the HDF groups in the BAG will be required, and new elements of other groups might be required. This section describes how to apply for an extension or bug fix.

### Nomination Process

Any requests for extension shall be considered by the [BAG Architecture Review Board](FSD-BAGARB.md) as a group.  All communication shall be by e-mail only, using the ``navsurf_dev@ccom.unh.edu`` address. Originators should include details appropriate to their request as described below and be ready to answer any subsequent questions that might be required.

A ‘receipt notice’ e-mail shall be returned to the originator immediately, and a reply to the request shall be returned as quickly as possible. The decision-making process shall be as defined in Section 6.

### Request for an Extension HDF Group

Requests for an additional HDF group to be added to the base structure of the BAG must be accompanied by a full description of the data structure to be encoded. The request must be accompanied by a supporting document, e.g., an academic paper, user manual with appropriate details or a URI, and by preference code to read/write the data format. If the location of the section within the BAG structure is important, a recommendation for location may also be submitted.

The submission format may be plain text, Adobe PDF, or Microsoft Word. Other formats may be supported; Please check with the BAG-ARB before sending a document that is not in one of those three formats.

Since the FSD and the BAG format are open source, it is very important that the submission must be able to be published. This includes the source code submitted in support of the request. By sending the request to the group, the submitter explicitly agrees that:

*    They are the owner of any intellectual property associated with the information in the request, and/or have the appropriate authority to transfer the associated intellectual property.
*    The information in the request is not covered by any restrictions (e.g., security constraints, commercial secrets) that would prevent it from being used in the Open Navigation Surface project.
*    There are no limitations on the publication, dissemination or other transmission of the data structure.
*    Any source code provided may be used, adapted, or otherwise transformed for use in the source code base of the Open Navigation Surface project, including re-distribution of the code through any means in which the source code is generally made available.

Any requests that do not meet these requirements will be returned to the submitter, with comments as to cause, without further consideration by the BAG-ARB. The BAG-ARB, at their discretion, may request, in writing, confirmation of any or all the above terms, or any others as may seem appropriate at the time, from the originator of the request. Provision of this confirmation shall be a mandatory condition for acceptance and adoption of the request.

### Request for an Extension to an Existing Group

Extensions to an existing group, for example adding a new ellipsoid or datum definition to the metadata, or another element to an existing group-specific metadata attribute, may be submitted in the same way as for an [extension HDF group](FSD-Extensions.md#request-for-an-extension-hdf-group), including the requirements for free distribution and source code reuse. In addition, requests must contain a strong rationale for why the addition should be adopted.

### Request for a Bug Fix

Requests for a Bug Fix should include a full description of the problem, with as much information about causes and conditions as possible, including the revision of the BAG library being used, and the version of the BAG format being constructed. If available, details about the machine’s system architecture, platform, or operating system would also be beneficial to the BAG-ARG for assessing the requested bug fix.

A suitable test case should also be included, if possible, that allows the problem to be exercised. Do not send binary data attachments to the e-mail list in the first instance, since this list is distributed widely. If required, a member of the BAG-ARB will request supporting data by other means.

If a fix for the problem is known, it should be included with the initial submission. Please note that the conditions on distribution, adaptation, and re-use of source code in Section 8.2 apply to any source code submitted as a potential fix.

## [Next: Glossary of Acronyms](FSD-Glossary.md)