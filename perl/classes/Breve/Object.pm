
use strict;
use Breve;

package Breve::Object;

###Summary: the top level object class. <P> The Object class is the root class.  All classes used in breve have Object as an ancestor.  The object class implements some basic  services that all classes will have access to. <p> Subclassing Object directly is rare.  The classes OBJECT(Real) and  OBJECT(Abstract) are logical separations of the Object class containing  "real" objects (which correspond to a physical entity in the simulated  world) and "abstract" objects which are generally used for computation  or control of the real objects.  You should consider subclassing   one of these classes instead.

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	my $self->{ birthTime } = 0;
	my $self->{ controller } = undef;
	init( $self );
	return $self;
}

sub addDependency {
	###Makes this instance depend on instance i when archiving and  dearchiving.  This means that if this instance is archived, then i will also have to be archived, and that when this  instance is dearchived, that i will have to be dearchived  first. <p> Dependencies can cause large numbers of instances to be archived in response to a single archiving event (as dependencies of  dependencies, and dependencies of dependencies of dependencies, ad infinitum will also be archived).  This means that you should make dependencies sparingly, only when absolutely required. <p> Circular dependencies are forbidden.

	my ($self, $i );
	( $self, $i ) = @_;

	if( $i ) {
		Breve::addDependency( $self, $i );
	}

}

sub announce {
	###Sends a notification with the message theMessage to all observer   objects.  See METHOD(observe) for information on making an object an observer.

	my ($self, $theMessage );
	( $self, $theMessage ) = @_;

	Breve::notify( $self, $theMessage );
}

sub archive {
	my $self;
	( $self ) = @_;

	return 1;
}

sub archiveAsXml {
	###Writes the current object to the XML file fileName.

	my ($self, $fileName );
	( $self, $fileName ) = @_;

	Breve::archiveXMLObject( $self, $fileName );
}

sub callMethod {
	###Calls the method named methodName for this object.  Returns the result of the method call.

	my ($self, $methodName );
	( $self, $methodName ) = @_;

	return Breve::callMethodNamed( $self, $methodName, [] );
}

sub callMethod {
	###Calls the method named methodName for this object.  Returns the result of the method call. <p> The arguments to the object are passed in using the list argList.  Since keywords are not passed in, this method relies on the order the arguments appear in the argument list and passes them to methodName in the order  in which they appear in methodName's definition. <p> Why not call a method  directly?  This method is used in circumstances where you might want to have some sort of callback method.  As an example, let's  say you write a general purpose class which can sort objects based  on different criteria.  How would the user specify these arbitrary criteria?  Using this method would allow the user to pass in the name of the method they want to use, and the sorting object could use this method to execute the callback. <p> If the concept of a callback doesn't make sense, then you can probably ignore this method.

	my ($self, $methodName, $argList );
	( $self, $methodName, $argList ) = @_;

	return Breve::callMethodNamed( $self, $methodName, $argList );
}

sub canRespond {
	###Returns true or false (1 or 0) depending on whether this instance can respond to a method called methodName. <p> But wow, what an awkward declaration!  Same reason as the  method METHOD(is).  Again, works like a statement that replies with true or false: object can-respond to "run". <p> It's really not my fault that the infinitive of "can" is "be able".

	my ($self, $methodName );
	( $self, $methodName ) = @_;

	return Breve::respondsTo( $self, $methodName );
}

sub dearchive {
	my $self;
	( $self ) = @_;

	return 1;
}

sub destroy {
	###Automatically called when this object is freed.  This method should never be called manually.  If subclasses need to free objects or data, they should implement their own "destroy"  methods.

	my $self;
	( $self ) = @_;

}

sub disableAutoFree {
	###Disables garbage collection on a per-object basis.

	my $self;
	( $self ) = @_;

	Breve::setGC( $self, 0 );
}

sub enableAutoFree {
	###Enables garbage collection on a per-object basis.

	my $self;
	( $self ) = @_;

	Breve::setGC( $self, 1 );
}

sub getAge {
	###Returns the number of seconds this object has existed in the  simulation.

	my $self;
	( $self ) = @_;

	return ( $self->{ controller }->getTime() - $self->{ birthTime } );
}

sub getController {
	###Returns the controller object that associated with the current  simulation.  It's preferable to simply reference the variable "controller".

	my $self;
	( $self ) = @_;

	return Breve::getController( $self);
}

sub getDescription {
	###This method should provide a textual description of an object. When the "print" command prints an object, it calls this method to get a description of the object.  By default, print will show the class and pointer of an object, but by overriding this method in your own classes, you can append other sorts of data to the  output.

	my $self;
	( $self ) = @_;

}

sub getType {
	###Returns as a string the type of this object.

	my $self;
	( $self ) = @_;

	return Breve::objectName( $self );
}

sub init {
	my $self;
	( $self ) = @_;

	$self->{ controller } = $self->getController();
	$self->{ birthTime } = $self->{ controller }->getTime();
}

sub isA {
	###This method returns true or false (1 or 0) depending on whether the instance in question belongs to class className.  This  method checks if className is a superclass of the current  object as well. <p> Deprecated because "is" is reserved in Python 

	my ($self, $className );
	( $self, $className ) = @_;

	return Breve::isa( $self, $className );
}

sub isA {
	###This method returns true or false (1 or 0) depending on whether the instance in question belongs to class className.  This  method checks if className is a superclass of the current  object as well.

	my ($self, $className );
	( $self, $className ) = @_;

	return Breve::isa( $self, $className );
}

sub isASubclass {
	###Returns 1 if this object is a subclass of the class specified with className, returns 0 otherwise.

	my ($self, $className );
	( $self, $className ) = @_;

	return Breve::isa( $self, $className );
}

sub observe {
	###Causes the current object to observe theObject.  By registering as and observer, the current object will receive a call to theMethod whenever theObject calls the METHOD(announce) method with notificiation theNotification.

	my ($self, $theObject, $theNotification, $theMethod );
	( $self, $theObject, $theNotification, $theMethod ) = @_;

	Breve::addObserver( $self, $theObject, $theNotification, $theMethod );
}

sub postDearchiveSetController {
	###Used internally to set the controller instance for this variable.   Used after object dearchiving.

	my $self;
	( $self ) = @_;

	$self->{ controller } = $self->getController();
}

sub removeDependency {
	###Removes theObject from this object's dependency list.  See METHOD(add-dependency) for more information on dependencies.

	my ($self, $theObject );
	( $self, $theObject ) = @_;

	Breve::removeDependency( $self, $theObject );
}

sub schedule {
	###Schedules a call to theMethod when the simulation time equals theTime.  The margin of error of the callback time is equal to iteration step (see METHOD(set-iteration-step)). <br> If you want to schedule an event at a time relative to the current time, use the method METHOD(get-time) to get the current simulation time and then add the offset you want.

	my ($self, $theMethod, $theTime );
	( $self, $theMethod, $theTime ) = @_;

	Breve::addEvent( $self, $theMethod, $theTime, 0 );
}

sub scheduleRepeating {
	my ($self, $theMethod, $theInterval );
	( $self, $theMethod, $theInterval ) = @_;

	Breve::addEvent( $self, $theMethod, $theInterval, $theInterval );
}

sub sendOverNetwork {
	###Sends this object over the network to a breve server on host hostName listening on port portNumber.

	my ($self, $hostName, $portNumber );
	( $self, $hostName, $portNumber ) = @_;

	return Breve::sendXMLObject( $self, $hostName, $portNumber, $self );
}

sub unobserve {
	###Unregisters the current object as an observer of theObject with  notification theNotification.

	my ($self, $theObject, $theNotification );
	( $self, $theObject, $theNotification ) = @_;

	Breve::removeObserver( $self, $theObject, $theNotification );
}


;

1;

