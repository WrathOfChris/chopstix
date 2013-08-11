#!/bin/sh

# 10cpi
echo -ne \\022
echo 1234567890 10CPI

# 12cpi
echo -ne \\033:
echo 123456789012 12CPI

# 17cpi / compressed
echo -n \\017
echo 12345678901234567 17CPI

# back to normal
echo -ne \\022

# master font selector
# 	0:	pica	\
#	1:	elite	/
#	4:	condensed	\
#	8:	emphasized	/
#	16:	double-strike
#	32: double-wide
#	64:	italic
#	128:underline

#echo -ne \\033!\\010
echo -ne \\033E
echo Emphasized text
echo -ne \\033F

#echo -ne \\033!\\020
echo -ne \\033G
echo Double-strike text
echo -ne \\033H

#echo -ne \\033!\\040
echo -ne \\033W1
echo Double-wide text
echo -ne \\033W0

#echo -ne \\033!\\100
echo -ne \\0335
echo Italic text
echo -ne \\0334

#echo -ne \\033!\\200
echo -ne \\033-1
echo Underlined text
echo -ne \\033-0

echo -ne \\033:
echo -ne \\033E
echo -ne \\033G
echo -ne \\033W1
echo 12cpi Emphasized Double-strike Double-wide text
echo -ne \\033W0
echo -ne \\033H
echo -ne \\033F
