
use strict;
use Breve;

package Breve::MenuItem;

our @ISA = qw(Breve::Abstract);

###The MenuItem class holds menu items associated with objects.  Menus can be associated with Mobile objects, in which case they are shown as contextual menu items, or associated with Control objects in which case they are shown under the global application menu.

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;

	$self->{ checked } = 0;
        $self->{ enabled } = 0;
	$self->{ menuPointer } = undef;
	$self->{ method } = "";
	$self->{ name } = "";
	$self->{ owner } = undef;

	$self->SUPER::init();

	return $self;
}

sub check {
	###Places a check mark next to the menu item.

	my $self;
	( $self ) = @_;

	if( ( not $self->{ menuPointer } ) ) {
		return;
	}

	Breve::callInternal($self, "menuItemSetCheck", $self->{ menuPointer }, 1 );
	$self->{ checked } = 1;
}

sub createMenu {
	###This method initializes a menu item with title menuName for theObject which will call methodName when selected.

	my ($self, $menuName, $theObject, $methodName );
	( $self, $menuName, $theObject, $methodName ) = @_;

	$self->{ name } = $menuName;
	$self->{ method } = $methodName;
	$self->{ owner } = $theObject;

	$self->{ owner }->addDependency( $self );

	$self->{ menuPointer } = Breve::callInternal($self, "menuItemNew", $self->{ owner }, $self->{ method }, $self->{ name } );
	if( ( not $self->{ menuPointer } ) ) {
		print "error adding menu item for method %s." % (  $methodName );
		breve->deleteInstances( $self );
		$self->{ enabled } = 1;
		return 0;

	}

	return $self;
}

sub dearchive {
	my $self;
	( $self ) = @_;

	$self->{ menuPointer } = Breve::callInternal($self, "menuItemNew", $self->{ owner }, $self->{ method }, $self->{ name } );
	if( $self->{ checked } ) {
		$self->check();
	}	 else {
		$self->uncheck();
	}

	if( $self->{ enabled } ) {
		$self->enable();
	}	 else {
		$self->disable();
	}

	return 1;
}

sub disable {
	###Disables the menu item such that it cannot be selected.

	my $self;
	( $self ) = @_;

	if( ( not $self->{ menuPointer } ) ) {
		return;
	}

	Breve::callInternal($self, "menuItemSetEnabled", $self->{ menuPointer }, 0 );
	$self->{ enabled } = 0;
}

sub enable {
	###Enables a menu item, if it is disabled.

	my $self;
	( $self ) = @_;

	if( ( not $self->{ menuPointer } ) ) {
		return;
	}

	Breve::callInternal($self, "menuItemSetEnabled", $self->{ menuPointer }, 1 );
	$self->{ enabled } = 1;
}

sub getDescription {
	my $self;
	( $self ) = @_;

	return $self->{ name };
}

sub uncheck {
	###Removes the check mark next to the menu item, if it exists.

	my $self;
	( $self ) = @_;

	if( ( not $self->{ menuPointer } ) ) {
		return;
	}

	Breve::callInternal($self, "menuItemSetCheck", $self->{ menuPointer }, 0 );
	$self->{ checked } = 0;
}

1;

