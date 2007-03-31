package Breve;

use 5.008006;
use strict;
use warnings;

require Exporter;

use __init__;

our @ISA = qw(Exporter);
our %EXPORT_TAGS = ( 'all' => [ qw(
	
) ] );
our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );
our @EXPORT = qw(
	
);
our $VERSION = '0.01';

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Breve - Perl interface for building simulations in Breve

=head1 SYNOPSIS

  use Breve;

In your main package, create a new Perl object descended from
the Breve::Control class. Be sure to call its init() function. It will
then automatically set itself as the controller for the simulation
and setup the necessary variables for runtime.

=head1 DESCRIPTION

Blah blah blah.

=head2 EXPORT

None by default.

=head1 SEE ALSO

http://www.spiderland.org/

=head1 AUTHOR

Carson Baker, E<lt>carson@E<gt>

=head1 COPYRIGHT AND LICENSE

*****************************************************************************                                                                           
 The breve Simulation Environment                                          
 Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       
                                                                           
 This program is free software; you can redistribute it and/or modify      
 it under the terms of the GNU General Public License as published by      
 the Free Software Foundation; either version 2 of the License, or         
 (at your option) any later version.                                       
                                                                           
 This program is distributed in the hope that it will be useful,           
 but WITHOUT ANY WARRANTY; without even the implied warranty of            
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             
 GNU General Public License for more details.                              
                                                                           
 You should have received a copy of the GNU General Public License         
 along with this program; if not, write to the Free Software               
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
***************************************************************************

=cut
