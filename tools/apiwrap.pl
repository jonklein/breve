#!/usr/bin/perl

#
# apiwrap -- 
# 	a script for the breve simulation environment (http://www.spiderland.org)
#	that reads a subset of C function definitions and converts them into breve
#	API functions which can be called from steve, or any other language 
#	frontend.
# 

if($#ARGV < 2 || $#ARGV > 3) {
	print STDERR "usage: $0 headerfile includename outputname [configfile]\n";
	exit 0;
}

open(FILE, $ARGV[0]) || die "Cannot open $ARGV[0]: $!\n";

$filename = $ARGV[2];

$filename =~ s/\.[ch]$//;
$filename =~ s/^.*\///;
$filename = ucfirst($filename);

if($#ARGV == 3) {
	open(CONFIGFILE, $ARGV[3]) || die "Cannot open $ARGV[3]: $!\n";

	$line = join('', <CONFIGFILE>);

	eval($line);

	close(CONFIGFILE);
}

open(CFILE, ">breveFunctions${filename}.c") || die "Cannot open breveFunctions${filename}.c for writing: $!\n";

# Some recursive regular expressions for parsing out a function definition

$symbol = "[a-zA-Z0-9_]+";

# Types: standard C types, known breve structures, or pointer types.

$type = "(?:void\\s+)|(?:int\\s+)|(?:float\\s+)|(?:double\\s+)|(?:slMatrix\s+)|(?:(?:struct\\s\*)?$symbol\\s\*\\*)";


# a declaration is a type and a symbol.

$declaration = "\\s*($type)\\s*($symbol)\\s*";


# a declaration list is 0 or more comma-delimited declarations

$declaration_list = "(?:(?:)|(?:$declaration)|(?:(?:$declaration,)*$declaration))";


# a function: a declaration followed by a parenthesized declaration list

$function_definition = "^$declaration\\s*\\($declaration_list\\);";

select CFILE;

print CFILE<<__EOT__;
/*
	the breve simulation environment

	This file was generated automatically by the script $0.
*/

#include "kernel.h"
#include "$ARGV[1]"

/*\@{*/
/*! \\addtogroup InternalFunctions */
__EOT__

foreach $line (<FILE>) {
	chop $line;

	next if ($line eq "");
	next if ($line =~ /^#/);

	$line =~ s/\s+/ /g;

	if($line !~ /$function_definition/x) {
		next;
	} 

	# get the return type and the name for this function.

	$line =~ /^$declaration/;

	$return_type = $1;
	$name = $2;

	$functionname = ucfirst($name);
	$functionname = "breveFunction$functionname";
	$return_type =~ s/\s+$//g;

	# get the arguments for the function

	$line =~ /\((.*)\)/;
	$argument_list = $1;
	@arguments = split(/,/, $argument_list);

	# start printing out the declaration

print CFILE<<__EOT__;
/*!
	\\brief A breve API function wrapper for the C-function \\ref $name.

	See the documentation for \\ref $name for more details.
*/

int $functionname(brEval arguments[], brEval *result, brInstance *instance) {
__EOT__

	$index = 0;
	$argnames = ( );

	@accessors = ( );

	foreach $argument (@arguments) {
		$argument =~ /^$declaration/;
		$argtype = $1;
		$argname = $2;
		
		$argtype =~ s/\s+$//g;

		$accessor = accessorForType($argtype);

		if($argtype =~ /\*$/) {
			$accessor = "\t${argtype}$argname = $accessor(&arguments[$index]);\n";
		} else {
			$accessor = "\t$argtype $argname = $accessor(&arguments[$index]);\n";
		}

		push @accessors, $accessor;

		$enum = enumForType($argtype);

		if($index == 0) {
			$function_arguments = $argname;
			$enum_arguments = "$enum,";
		} else {
			$function_arguments = "$function_arguments, $argname";
			$enum_arguments = "$enum_arguments $enum,";
		}

		$index++;
	}

	$return_accessor = accessorForType($return_type);
	$return_enum = enumForType($return_type);

	$enum_arguments = "$return_enum, $enum_arguments";

	$return_result = "";
	$return_result = "$return_accessor(result) = " if($return_type ne "void");

	$return_function = "$name($function_arguments)";

	if($return_type eq "char *") { $return_function = "slStrdup($return_function)"; }

	if(eval("defined \&$name;")) {
		eval("${name}(\\\@accessors, \$return_result, \$return_function);");
	} else {
		outputFunction(\@accessors, $return_result, $return_function);
	}

	print CFILE "}\n\n";

	$newcall = "\tbrNewBreveCall(namespace, \"$name\", $functionname, $enum_arguments 0);\n";

	push @newcalls, $newcall;
}

#
# Print out the 
#

print CFILE<<__EOT__;
/*@}*/

void breveInit${filename}Functions(brNamespace *namespace) {
@newcalls}
__EOT__

close(CFILE);

exit(0);

sub accessorForType() {
	$symbol = symbolForType($_[0]);

	return "&BRVECTOR" if($symbol eq "VECTOR");

	return "BR$symbol";
}

sub enumForType() {
	$symbol = symbolForType($_[0]);
	return "AT_$symbol";
}

sub symbolForType() {
	$type = $_[0];
	return "INT" if($type eq "int");
	return "DOUBLE" if($type eq "double");
	return "DOUBLE" if($type eq "float");
	return "STRING" if($type eq "char *");
	return "LIST" if($type eq "slList *");
	return "HASH" if($type eq "slHash *");
	return "MATRIX" if($type eq "slMatrix");
	return "VECTOR" if($type eq "slVector *");
	return "VECTOR" if($type eq "PushVector *");
	return "DATA" if($type eq "slData *");
	return "NULL" if($type eq "void");
	return "POINTER";
}

sub outputFunction() {
	$a_ref = $_[0];
	$result = $_[1];
	$function = $_[2];

	@accessors = @$a_ref;

	if($#accessors != -1) { print CFILE "@accessors\n"; }

print CFILE <<__EOT__;
	${result}${function};

	return EC_OK;
__EOT__
}
