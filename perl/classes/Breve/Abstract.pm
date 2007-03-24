
use strict;

package Breve::Abstract;

our @ISA = qw(Breve::Object);

### Abstract is a logical separation of the OBJECT(Object) class, used for objects which have no representation in the simulated  world.  This is in contrast to OBJECT(Real) objects, which  do have representations in the simulated world. Since "abstract" is only a logical distinction, the Abstract class  currently has no methods implemented.  '''

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	$self->init();
	return $self;
}

sub init {
    my $self = shift;
    $self->SUPER::init();
}

1;
