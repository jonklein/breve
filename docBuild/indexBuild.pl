#!/usr/bin/perl

# that's some real messy code you got there, lou.

my $ignore_methods = "^init\$|^destroy\$|^iterate\$|^archive\$|^dearchive\$";

print<<__EOT__;
<title>breve Class Library Method Index</title>
<STYLE TYPE="text/css">
    BODY, P, LI {
        color: black;
        font-family: Geneva,Arial,Helvetica,Swiss,SunSans-Regular;
        font-size: 95%;
    }
    BODY {
        margin-left: 10%;
        margin-right: 10%;
    }
    TT {
        font-size: 80%;
    }
</STYLE>
<body bgcolor="#ffffff">
<i>
The following index is auto-generated from the breve class files.
<p>
For more information on breve and steve, refer to the 
<a href="http://www.spiderland.org/breve">breve</a> homepage.
</i>
<p>
<p>
__EOT__

foreach $in (@ARGV) {
	if(!open(CLASS, $in)) {
		print STDERR "warning: cannot open $in: $!\n";
		next;
	}

	@methods = grep(/^\s*\+\s*to\s+.*/, <CLASS>);

	$file = $in;
	$file =~ s/.*\///;
	$html = $file;
	$html =~ s/\.tz/\.html/;

	foreach $method (@methods) {
		chop $method;
		$method =~ s/:$//;
		$method =~ s/^\s*//;
		
		($name) = ($method =~ /\+ to ([^\s]*)/);

		next if($name =~ /$ignore_methods/);

		push @method_index, "$name<br>\n<font size=-2>$method [<a href=\"$html\">$file</a>]</font><p>\n";
		push @all_methods, "$name:$file:$html";
	}

	close(CLASS);
}

@method_index = sort @method_index;

foreach $i (@method_index) {
	($meth, $class) = ($i =~ /^(.*)\<br\>.*\[\<a href.*\>(.*)\<\/a\>\]/s);
	
	$class =~ s/\.tz/\.html/;

	$i =~ s/$meth/\<a href=\"$class#$meth\"\>$meth\<\/a\>/;

	print $i;
}

print<<__EOT__;
<p>
<p>
</body>
__EOT__
