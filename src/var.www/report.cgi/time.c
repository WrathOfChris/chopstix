/* $Gateweaver: time.c,v 1.3 2007/09/04 21:45:58 cmaxwell Exp $ */
/*
 * Copyright (c) 2007 Christopher Maxwell.  All rights reserved.
 */
#include <string.h>
#include "report.h"

time_t
time_today(enum time_type type)
{
	time_t now = time(NULL);
	struct tm tm;

	bcopy(localtime(&now), &tm, sizeof(tm));
	if (type == TIME_START) {
		tm.tm_hour = 0;
		tm.tm_min = 0;
		tm.tm_sec = 0;
	} else if (type == TIME_END) {
		tm.tm_hour = 23;
		tm.tm_min = 59;
		tm.tm_sec = 60;
	} else
		return now;

	now = mktime(&tm);
	return now;
}

time_t
time_yesterday(enum time_type type)
{
	time_t now = time(NULL) - (24 * 60 * 60);
	struct tm tm;

	bcopy(localtime(&now), &tm, sizeof(tm));
	if (type == TIME_START) {
		tm.tm_hour = 0;
		tm.tm_min = 0;
		tm.tm_sec = 0;
	} else if (type == TIME_END) {
		tm.tm_hour = 23;
		tm.tm_min = 59;
		tm.tm_sec = 60;
	} else
		return now;

	return mktime(&tm);
}

const char *
time_print(time_t now)
{
	static char datetime[sizeof("9999-12-31T23:59:60+0000")];
	strftime(datetime, sizeof(datetime), DATE_FORMAT, localtime(&now));
	return datetime;
}

time_t
time_parse(const char *str)
{
	struct tm tm = {0};
	strptime(str, DATE_FORMAT, &tm);
	return mktime(&tm);
}

time_t
time_ext(enum time_type type)
{
	time_t now = time(NULL);
	struct tm tm;

	bcopy(localtime(&now), &tm, sizeof(tm));
	switch (type) {
		case TIME_START:
			tm.tm_hour = 0;
			tm.tm_min = 0;
			tm.tm_sec = 0;
			break;

		case TIME_END:
			tm.tm_hour = 23;
			tm.tm_min = 59;
			tm.tm_sec = 60;
			break;

		case TIME_TODAY:
			tm.tm_hour = 0;
			tm.tm_min = 0;
			tm.tm_sec = 0;
			break;

		case TIME_YESTERDAY:
			tm.tm_mday--;
			tm.tm_hour = 0;
			tm.tm_min = 0;
			tm.tm_sec = 0;
			break;

		case TIME_TOMORROW:
			tm.tm_mday++;
			tm.tm_hour = 0;
			tm.tm_min = 0;
			tm.tm_sec = 0;
			break;

		case TIME_LASTWEEK:
			tm.tm_mday -= 7;
		case TIME_THISWEEK:
			tm.tm_mday -= tm.tm_wday;
			tm.tm_hour = 0;
			tm.tm_min = 0;
			tm.tm_sec = 0;
			break;

		case TIME_LASTMONTH:
			tm.tm_mon--;
		case TIME_THISMONTH:
			tm.tm_mday = 1;
			tm.tm_hour = 0;
			tm.tm_min = 0;
			tm.tm_sec = 0;
			break;

		case TIME_LASTYEAR:
			tm.tm_year--;
		case TIME_THISYEAR:
			tm.tm_mon = 0;
			tm.tm_mday = 1;
			tm.tm_hour = 0;
			tm.tm_min = 0;
			tm.tm_sec = 0;
			tm.tm_isdst = 0;
			break;

		default:
			return now;
	}

	now = mktime(&tm);
	return now;
}
