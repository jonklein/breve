#!/usr/bin/perl

use strict;

my $bgcolor = "#ffffff";
my $ignore_methods = "^init\$|^delete\$|^destroy\$|^iterate\$|^post-iterate\$|^archive\$|^dearchive\$|^get-description\$";

my $frameIndex=<<__EOT__;
<HTML>
  <HEAD>
    <TITLE>breve class documentation</TITLE>
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
  </HEAD>
  <FRAMESET cols="2,6">
    <FRAME SRC="classlist.html" name="index">
    <FRAME SRC="Object.html" name="class">
  </FRAMESET>
  <NOFRAMES>
  </NOFRAMES>
</HTML>
__EOT__

my $WARNING=<<__EOT__;
<center>
<font size=-2>
These pages are auto-generated from self-documenting comments embedded 
in class files.
<br>
For more information on <b>breve</b> and <i>steve</i>, refer to the 
<a href="http://www.spiderland.org/breve" target="_top">breve homepage</a>.
</font>
</center>
__EOT__

if(scalar(@ARGV) != 1) {
    print "usage: $0 outputdir\n";
    exit(1);
}

if(!chdir($ARGV[0])) {
    print "Cannot change directory to $ARGV[0]: $!\n";
    exit(1);
}

open(FRAMES, ">index.html") || die "Cannot open index.html: $!\n";
print FRAMES $frameIndex;
close(FRAMES);

my $input = join('', <STDIN>);

my @all_classes;
my @classes = split(/\n#\s*/, $input);

my $class;
my $section;
my @sections;

my %classhash;

foreach $class (@classes) {
    my ($parent, $classname, $filename);

	my ($firstline, $aka);
	($firstline) = ($class =~ /^(.*)/);

    ($parent, $classname) = ($firstline =~ /^(\w+)\s*:\s*(\w+)/);
	($filename) = ($firstline =~ /\[(\S+)\]$/);
	($aka) = ($firstline =~ /\(aka (\w+)\)/);

	$filename =~ s/.*\///;

    next if($parent eq "" || $classname eq "");

    # now get rid of the first line with the names...
    $class =~ s/^.*//;

    # grab the starting comments and get rid of them.
    my $classdesc;

    $classdesc = $class;
    
    $classdesc =~ s/\n\+.*//s;
    ($class) = ($class =~ /(\n\+.*)/s);

    print "processing $parent : $classname\n";
    push(@all_classes, $classname);

	$classhash{$classname} = $parent;

    my @sections = split(/^\+\s+section\s*/m, "\n$class");

	if(!open(DOC, ">$classname.html")) {
		print STDOUT "error opening $classname.html: $!\n";
		next;
    }

    select(DOC);

   	header($parent, $classname, $filename, $classdesc, $bgcolor);

	print "<BLOCKQUOTE>\n";
	foreach $section (@sections) {
		my $title;

		if($section =~ /^[\t\ ]*\"/) {
			($title) = ($section =~ /\"([^\"]*)\"/);
		} else {
			$title = "";
		}

    	my @methods = split(/^\+\s+to\s*/m, "\n$section");

	    @methods = sort @methods;

    	sectionHeader($title, \@methods);
	}
	print "</BLOCKQUOTE>\n";

	# foreach $section (@sections) {
	#	(my $title) = ($section =~ /\"([^\"]*)\"/);

	$class =~ s/\+\s+section.*\n//g;

   	my @methods = split(/^\+\s+to\s*/m, "\n$class");

    @methods = sort @methods;

   	sectionBody("none", \@methods);

	footer();

    close(DOC);

    select(STDOUT);
}

if(!open(DOC, ">classlist.html")) {
    print STDOUT "error opening classlist.html: $!\n";
    exit(1);
}

select(DOC);

print<<__EOT__;
<HEAD>
<TITLE>All Classes</TITLE>
<STYLE TYPE="text/css">
    BODY, P, LI {
        color: black;
        font-family: Geneva,Arial,Helvetica,Swiss,SunSans-Regular;
        font-size: 80%;
    }
    BODY {
        margin-left: 10%;
        margin-right: 10%;
    }
    TT {
        font-size: 80%;
    }
</STYLE>
</HEAD>
<BODY BACKGROUND="../aqua.gif">
<p>
$WARNING
<p>
<a href="method_index.html" target=\"class\">Method index</a>
<p>
<font size=+1>Classes:</font>
<P>
__EOT__

my $name;

print_subclasses("Object");

footer();

close DOC;

sub header() {
    my $parent = $_[0];
    my $class = $_[1];
    my $filename = $_[2];
    my $classdesc = $_[3];
    my $color = $_[4];

    my $parent_link;

	my $no_tz = $filename;

	$no_tz =~ s/\.tz//;

    if($parent eq "NULL") {
		$parent = "(no parent)";
        $parent_link = "(no parent)";
    } else {
        $parent_link = "<a href=\"$parent.html\">$parent</a>";
    }

    print<<__EOT__;
<head>
<title>$parent : $class</title>
<STYLE TYPE="text/css">
    BODY, P, LI {
		color: black;
		font-family: Verdana, Arial, sans-serif;
		font-size: small;
    }

    BODY {
        margin-left: 10%;
        margin-right: 10%;
    }
    TT {
        font-size: 80%;
    }
</STYLE>
</head>
<body>
<p>
<center>
<p>
<img src="breve_icon.jpg"></center>
<p>
<p>

<i>$WARNING</i>

<h2>$parent_link : $class</h2>
This class is included as part of the file $filename.
<br>
To use this class, include the line \"<b>\@use $no_tz.</b>\"
__EOT__
    if($classdesc =~ /\%/) {
        my @nothing = ();
        $classdesc = process_docs($classdesc, \@nothing);

        print "<p>\n<h3>Class description:</h3><p>\n";
		print "<blockquote>\n";
        print "$classdesc";
		print "</blockquote>\n";
    }

	print "<P>\n";
	print "<H3>Class methods:</H3></P>\n";
}

sub sectionHeader() {
	my $title = $_[0];
    my $method_ref = $_[1];
    my @methods = @$method_ref;
    my $method;

	print "<P><B>$title</B>";
    print "<UL>\n";

    foreach $method (@methods) {
        my $name;

        ($name) = ($method =~ /(^[\w-]+)/);
        next if($name eq "" || $name =~ /$ignore_methods/);

        print "<li><a href=\"#$name\">$name</a></li>\n";
    }

    print "</UL>\n";
}

sub sectionBody() {
	my $title = $_[0];
    my $method_ref = $_[1];
    my @methods = @$method_ref;
	my $method;

    foreach $method (@methods) {
        my $name;

        ($name) = ($method =~ /(^[\w-]+)/);
        next if($name eq "" || $name =~ /$ignore_methods/);

        my $declaration;
        ($declaration) = ($method =~ /^([^%]*)%/s);
        # ($declaration) = ($method =~ /^(.*)/);
		$declaration =~ s/\n/ /sg;
		$declaration =~ s/\s+/ /sg;
		$declaration =~ s/\w+$//sg;

        $method =~ s/^[^%]*/\n/s;

       	# next if($declaration eq "" || $name eq "");

		if(!$declaration) {
			print STDERR "warning: no documentation for method \"$name\"\n";
		}

        my $newdec = $declaration;
        $declaration =~ s/$name/\<b\>$name\<\/b\>/;

        $newdec =~ s/(^[\w-]+)//;

        my @variables = split(/\([\w-]*\)/, $newdec);

        $declaration = process_docs($declaration, \@variables);

        print "<hr>\n";
        print "<a name=\"$name\">$declaration<p></a>\n";

        # get rid of the name

        $method = process_docs($method, \@variables);

		print "<blockquote>\n";
        print "<p>$method<p>\n";
		print "</blockquote>\n";
    }

    print "</body>\n";
}


sub footer() {
    my $now = localtime();

    print "<hr><font size=-2><i>Documentation created $now</i></font>";
}

sub process_docs() {
    my $input = $_[0];
    my $var_ref = $_[1];
    my @vars = @$var_ref;

    $input =~ s/\s*\n[%\s\n]*/\ /g;
    $input =~ s/OBJECT\(([\w-]+)\)/<a href="$1.html">$1<\/a>/g;
    $input =~ s/METHOD\(([\w-]+)\)/<a href="#$1">$1<\/a>/g;
    $input =~ s/OBJECTMETHOD\(([\w-]+):([\w-]+)\)/<a href="$1.html#$2">$2 ($1)<\/a>/g;

    my $v;

    foreach $v (@vars) {
        my $word;
        ($word) = ($v =~ /([\w-]*)\s*$/);

		next if($word eq "");

        $input =~ s/(\W)$word(\W)/$1<i>$word<\/i>$2/g;
    }

	$input =~ s/(\s)breve(\W)/$1<b>breve<\/b>$2/g;

    return $input;
}

sub print_subclasses() {
	my $parent = $_[0];
	my $level = $_[1];

	my @out = ();

   	print "<a href=\"$parent.html\" target=\"class\">$parent</a><br>\n" if($level == 0);

	$level++;

	foreach $name (keys %classhash) {
		push(@out, $name) if($classhash{$name} eq $parent);
	}

	@out = sort @out;

	# print "<ul>";

	my $n;

	foreach $name (@out) {
		for($n=0;$n<$level;$n++) {
			print "&nbsp;&nbsp;";
		}

    	print "- <a href=\"$name.html\" target=\"class\">$name</a><br>\n";
		print_subclasses($name, $level + 1);
	}

	# print "</ul>"
}
