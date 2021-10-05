#include <iostream>

location: /
	root: /www
	indx: index.html

location: /cgi-bin
	root: /cgi
	indx: index.pl

location: /kapouet
	root: /tmp/www
	indx: index.html


target: /
itog_p: /www/index.html

target: /kub
itog_p: /www/kub/index.html

target: /www/
itog_p: /www/index.html

target: /no_index
itog_p: /www/no_index/index.html

target: /no_index/lol
itog_p: /www/no_index/lol/index.html

target: /kapouet/pouic/toto/pouet
itog_p: /tmp/www/pouic/toto/pouet

target: /cgi-bin/printenv.pl/foo/bar?var1=value1&var2=with%20percent%20encoding
itog_p: /cgi/printenv.pl/foo/bar?var1=value1&var2=with%20percent%20encoding


.pl
.php
.py
.cgi

int	main()
{

}
