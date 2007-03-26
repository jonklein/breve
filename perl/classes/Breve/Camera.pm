
use strict;
use Breve;

package Breve::Camera;

our @ISA = qw(Breve::Abstract);

###Summary: creates a new rendering perspective in the simulated world. <P> The Camera class is used to set up a viewing perspective in a simulation. Creating a new camera object places a viewing area with the new camera  perspective in the main viewing window. <P> See the OBJECT(Image) class to read data from a Camera (or from the  main simulation window) into a pixel buffer.  This can be useful for implementing vision algorithms.

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	$self->{ cameraPointer } = undef;
	$self->{ shared } = 0;
	$self->init();
	return $self;
}

sub delete {
	my $self;
	( $self ) = @_;

	if( ( $self->{ cameraPointer } and ( not $self->{ shared } ) ) ) {
		Breve::callInternal($self, "cameraFree", $self->{ cameraPointer } );
	}

}

sub disable {
	###Disables this camera.  The view from this camera will not be  updated or drawn to the viewing window. 

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "cameraSetEnabled", $self->{ cameraPointer }, 0 );
}

sub disableSmoothDrawing {
	###Disable smooth drawing for the main camera.   See METHOD(enable-smooth-drawing) for more information.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "cameraSetDrawSmooth", $self->{ cameraPointer }, 0 );
}

sub disableText {
	###Disables text for this camera.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "cameraTextSetEnabled", $self->{ cameraPointer }, 0 );
}

sub enable {
	###Enables the camera.  The view from this camera will be updated and drawn to the viewing window after each iteration.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "cameraSetEnabled", $self->{ cameraPointer }, 1 );
}

sub enableSmoothDrawing {
	###Enable smooth drawing for the camera.  Smooth drawing enables a smoother blending of colors, textures and lighting.  This feature is especially noticeable when dealing with spheres or large objects. <p> The disadvantage of smooth drawing is a potential performance hit. The degree of this performance hit depends on the number of polygons in the scene.  If speed is an issue, it is often best to disable both lighting and smooth drawing.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "cameraSetDrawSmooth", $self->{ cameraPointer }, 1 );
}

sub enableText {
	###Enables text for this camera.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "cameraTextSetEnabled", $self->{ cameraPointer }, 1 );
}

sub getHeight {
	###Returns the current camera width.

	my $self;
	( $self ) = @_;

	return Breve::callInternal($self, "cameraGetHeight", $self->{ cameraPointer } );
}

sub getRotation {
	###Returns a vector containing the rotation of the camera about the X-  and Y-axes return cameraGetRotation(cameraPointer).

	my $self;
	( $self ) = @_;

	return Breve::callInternal($self, "cameraGetRotation", $self->{ cameraPointer } );
}

sub getWidth {
	###Returns the current camera width.

	my $self;
	( $self ) = @_;

	return Breve::callInternal($self, "cameraGetWidth", $self->{ cameraPointer } );
}

sub init {
	my $self;
	( $self ) = @_;

	$self->SUPER::init();
	
	$self->{ cameraPointer } = Breve::callInternal($self,"cameraNew");

	$self->setSize( 100, 100);
	$self->setPosition( 50, 60 );

	return 0;
}

sub look {
	###Moves the camera to position and aims it at target.  target is is the target's location <b>relative to the camera</b>, not the target's "real-world" location.

	my ($self, $target, $position );
	( $self, $target, $position ) = @_;

	Breve::callInternal($self, "cameraPosition", $self->{ cameraPointer }, $position, $target );
}

sub setCameraPointer {
	###Used internally.

	my ($self, $p );
	( $self, $p ) = @_;

	if( ( not $self->{ shared } ) ) {
	    Breve::callInternal($self, "cameraFree", $self->{ cameraPointer } );
	}
	$self->{ cameraPointer } = $p;
	$self->{ shared } = 1;
}

sub setPosition {
	###Sets the position of the camera viewing area inside the main window.

	my ($self, $newX, $newY );
	( $self, $newX, $newY ) = @_;;

	Breve::callInternal($self, "cameraPositionDisplay", $self->{cameraPointer}, $newX, $newY );
}

sub setRotation {
	###Sets the rotation of the camera about the X- and Y-axes.

	my ($self, $rx, $ry );
	( $self, $rx, $ry ) = @_;

	Breve::callInternal($self,"cameraSetRotation", $self->{ cameraPointer }, $rx, $ry );
}

sub setSize {
    ###Sets the size of the camera viewing area.
    my $self = shift;
    my $newHeight = shift;
    my $newWidth = shift;

   Breve::callInternal($self,"cameraResizeDisplay", $self->{ cameraPointer }, $newWidth, $newHeight );
}

sub setZClip {
	###Sets the Z clipping plan to theDistance.  The Z clipping plan determines how far the camera can see.  A short Z clipping distance means that objects far away will not be drawn. <p> The default value is 500.0 and this works well for most simulations, so there is often no need to use this method. <p> Using a short Z clipping distance improves drawing quality, avoids unnecessary rendering and can speed up drawing during the simulation. However, it may also cause objects you would like to observe in the simulation to not be drawn because they are too far away.

	my ($self, $distance );
	( $self, $distance ) = @_;

	Breve::callInternal($self,"cameraSetZClip", $self->{ cameraPointer }, $distance );
}


1;

