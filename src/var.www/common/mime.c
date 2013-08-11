/* $Gateweaver: mime.c,v 1.1 2007/09/04 14:54:35 cmaxwell Exp $ */
/*
 * Copyright (c) 2007 Christopher Maxwell.  All rights reserved.
 */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "mime.h"

#define DIRMIMETYPE "0000000dir"
#define VOLMIMETYPE "0000000vol"
#define GRAMIMETYPE "0000000gra"

static struct {
	const char *s_img;
	const char *l_img;
	const char *ext;
} mimetypes[] = {
	{ "audio",		"ac3",		"ac3" },
	{ "audio",		"aiff",		"aiff" },
	{ "zip",		"arj",		"arj" },
	{ "audio",		"asf",		"asf" },
	{ "audio",		"au",		"au" },
	{ "audio",		"x-msvideo","avi" },
	{ "empty",		"bak",		"bak" },
	{ "bmp",		"bmp",		"bmp" },
	{ "empty",		"c",		"c" },
	{ "empty",		"cc",		"cc" },
	{ "doc",		"doc",		"doc" },
	{ "empty",		"eps",		"eps" },
	{ "application","application",	"exe" },
	{ "gif",		"gif",		"gif" },
	{ "zip",		"zip",		"gz" },
	{ "htm",		"htm",		"htm" },
	{ "html",		"html",		"html" },
	{ "jpeg",		"jpeg",		"jpeg" },
	{ "jpg",		"jpg",		"jpg" },
	{ "audio",		"mid",		"mid" },
	{ "audio",		"midi",		"midi" },
	{ "quicktime",	"mov",		"mov" },
	{ "audio",		"mp3",		"mp3" },
	{ "audio",		"mpeg",		"mpeg" },
	{ "audio",		"mpg",		"mpg" },
	{ "png",		"png",		"png" },
	{ "postscript",	"ps",		"ps" },
	{ "pdf",		"pdf",		"pdf" },
	{ "ppt",		"ppt",		"ppt" },
	{ "realmedia",	"ra",		"ra" },
	{ "realmedia",	"ra",		"ram" },
	{ "realmedia",	"rm",		"rm" },
	{ "text",		"rtf",		"rtf" },
	{ "svg",		"svg",		"svg" },
	{ "tar",		"zip",		"tar" },
	{ "text",		"txt",		"txt" },
	{ "tif", 		"tif",		"tif" },
	{ "tiff",		"tiff",		"tiff" },
	{ "audio",		"wav",		"wav" },
	{ "wmf",		"x-msvideo","wmf" },
	{ "wmv",		"x-msvideo","wmv" },
	{ "xls",		"xls",		"xls" },
	{ "zip",		"zip",		"zip" },
	{ NULL, NULL, NULL }
};

static struct {
	int once;
	MimeTypes types;
} mime;

int
mime_init(void)
{
	FILE *fp;
	size_t len, lineno;
	char *l, *c, *s;
	const char *mimefile;
	MimeType *mt;
	MimeExtension *me;

	if (mime.once++)
		return 0;

	if (strlen(conf.mimetypes) == 0)
		mimefile = CONF_MIMETYPES;
	else
		mimefile = conf.mimetypes;

	if ((fp = fopen(mimefile, "r")) == NULL)
		return -1;
	while ((s = c = l = fparseln(fp, &len, &lineno, NULL, 0)) != NULL) {
		while (*c && !isspace(*c))
			c++;
		*c = '\0';
		if (strlen(s) == 0)
			continue;
		if ((mt = add_MimeType(&mime.types)) == NULL)
			return -1;
		if ((mt->type = strdup(s)) == NULL) {
			del_MimeType(&mime.types, mt);
			return -1;
		}

		while (c < l + len) {
			c++;
			while (*c && isspace(*c))
				c++;
			s = c;
			while (*c && !isspace(*c))
				c++;
			*c = '\0';
			if ((me = add_MimeExtension(&mt->exts)) == NULL)
				continue;
			if ((me->ext = strdup(s)) == NULL) {
				del_MimeExtension(&mt->exts, me);
				continue;
			}
		}

		free(l);
	}
	fclose(fp);

	return 0;
}

void
mime_dump(void)
{
	unsigned int u, v;
	if (mime_init() == -1) {
		fprintf(stderr, "cannot load mime.types");
		return;
	}

	for (u = 0; u < mime.types.len; u++) {
		printf("%s", mime.types.val[u].type);
		for (v = 0; v < mime.types.val[u].exts.len; v++)
			printf("\t%s", mime.types.val[u].exts.val[v].ext);
		printf("\n");
	}
}

MimeType *
mime_find(const RobinFilename *name)
{
	unsigned int u, v;
	const char *s;
	size_t len = 0;

	if (mime_init() == -1) {
		fprintf(stderr, "cannot load mime.types");
		return NULL;
	}

	s = (const char *)name->data + name->length;
	while (s > (const char *)name->data && *--s != '.')
		len++;
	if (s < (const char *)name->data + name->length)
		s++;

	for (u = 0; u < mime.types.len; u++)
		for (v = 0; v < mime.types.val[u].exts.len; v++)
			if (strlen(mime.types.val[u].exts.val[v].ext) == len &&
					strncasecmp(mime.types.val[u].exts.val[v].ext, s, len) == 0)
				return &mime.types.val[u];

	return NULL;
}

const char *
mime_geticon(const RobinFilename *name, ROBIN_TYPE type)
{
	unsigned int u;
	const char *ext = NULL;

	if (type == RTYPE_DIRECTORY)
		return DIRMIMETYPE;
	else if (type == RTYPE_VOLUME)
		return VOLMIMETYPE;
	else if (type == RTYPE_GRAFT)
		return GRAMIMETYPE;

	if (name == NULL || name->data == NULL || name->length == 0)
		return "empty";

	ext = (const char *)name->data + name->length - 1;
	while (ext > (const char *)name->data && *ext != '.')
		ext--;
	if (ext != name->data && ext < (const char *)name->data + name->length) {
		ext++;
		for (u = 0; mimetypes[u].s_img != NULL; u++)
			if (strncasecmp(mimetypes[u].ext, ext,
						(const char *)name->data + name->length - ext) == 0)
				if (mimetypes[u].s_img)
					return mimetypes[u].s_img;
	}

	return "empty";
}

const char *
mime_geticon_large(const RobinFilename *name, ROBIN_TYPE type)
{
	unsigned int u;
	const char *ext;

	if (type == RTYPE_DIRECTORY)
		return DIRMIMETYPE;
	else if (type == RTYPE_VOLUME)
		return VOLMIMETYPE;
	else if (type == RTYPE_GRAFT)
		return GRAMIMETYPE;

	ext = (const char *)name->data + name->length;
	while (ext > (const char *)name->data && *ext != '.')
		ext--;
	if (ext != name->data && ext < (const char *)name->data + name->length) {
		ext++;
		for (u = 0; mimetypes[u].s_img != NULL; u++)
			if (strncasecmp(mimetypes[u].ext, ext,
						(const char *)name->data + name->length - ext) == 0)
				if (mimetypes[u].l_img)
					return mimetypes[u].l_img;
	}

	return "txt";
}

const char *
mime_gettype(const RobinFilename *name, ROBIN_TYPE type)
{
	MimeType *mt;

	if (type == RTYPE_DIRECTORY)
		return "text/directory";
	else if (type == RTYPE_VOLUME)
		return "text/volume";
	else if (type == RTYPE_GRAFT)
		return "text/graft";
	else if (type == RTYPE_SYMLINK)
		return "text/symlink";

	if ((mt = mime_find(name)) == NULL)
		return "application/unknown";

	return mt->type;
}
