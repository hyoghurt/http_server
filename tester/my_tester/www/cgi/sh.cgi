#!/bin/sh
#pagetwo.cgi
#отображение страницы с помощью вывода команды Unix
MYDATE=`date +%A" "%d" "%B" "%Y`
USERS=`who |wc -l`
echo "Content-type: text/html"
echo ""
echo "<HTML>"
echo "<H1><CENTER> THIS IS MY SECOND CGI PAGE</CENTER></H1>"
echo "<HR>"
echo "<H2><CENTER>$MYDATE</CENTER></H2>"
echo "<H3><CENTER> Total amount of users on to-day: $USERS</CENTER></H3>"
echo "<PRE>"
if  [ "$USERS" -lt 10 ]; then
echo "<CENTER> It must be early or it ls dinner time</CENTER>"
echo "<CENTER> because there ain't many users logged on</CENTER>"
fi
echo "</PRE>"
echo "</HTML>"
