use strict;

package Breve;

# constructor
sub new() {
	my ($class) = @_;
	
	my $self = {
		#	_state => {},
		#	_agents => [],
	};
	
	bless $self, $class;
	
	$self->initialState();
	
	return $self;
}

sub initialState() {
	my $self = shift;
	#$self->{_state}->{oil_level} = 100;
	#$self->{_state}->{grass_height} = 200;
}

sub setState() {
	my ($self, $key, $value) = @_;
	#$self->{_state}->{$key} = $value;
}

sub getState($) {
	my ($self, $query) = @_;
	#return $self->{_state}->{$query};
}	


return 1;
