#!/usr/bin/perl
print "Content-type: text/html\n\n";

print "<html>\n<body>\n<h1>Environment</h1>\n";
foreach (sort keys %ENV) 
{
 print "<b>$_</b>: $ENV{$_}<br>\n";
}
print "</body>\n</html>";
exit;
