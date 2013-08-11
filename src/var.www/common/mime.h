/* $Gateweaver: mime.h,v 1.1 2007/09/04 14:54:35 cmaxwell Exp $ */
#ifndef _MIME_H_
#define _MIME_H_

#ifndef CONF_MIMETYPES
#define CONF_MIMETYPES	"/conf/mime.types"
#endif

int mime_init(void);
void mime_dump(void);
MimeType * mime_find(const RobinFilename *);
const char * mime_geticon(const RobinFilename *, ROBIN_TYPE);
const char * mime_geticon_large(const RobinFilename *, ROBIN_TYPE);
const char * mime_gettype(const RobinFilename *, ROBIN_TYPE);

#endif
