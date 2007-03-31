
use strict;
use Breve;

package Breve::Real;

our @ISA = qw(Breve::Object);

###A class which is never instantiated--just used as a logical distinction from the Abstract classes.  See the child classes OBJECT(Mobile), OBJECT(Link) and OBJECT(Stationary) for more information.  The methods documented here may be used  with any of the child classes.

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	$self->{ bitmap } = 0;
	$self->{ collisionHandlerList } = ();
	$self->{ collisionShape } = undef;
	$self->{ color } = ();
	$self->{ displayShape } = undef;
	$self->{ e } = 0;
	$self->{ eT } = 0;
	$self->{ lightmap } = 0;
	$self->{ lines } = ();
	$self->{ menus } = ();
	$self->{ mu } = 0;
	$self->{ neighborhoodSize } = 0;
	$self->{ realWorldPointer } = undef;
	$self->{ texture } = 0;
	$self->{ textureScaleX } = 0;
	$self->{ textureScaleY } = 0;
	init( $self );
	return $self;
}

sub addDottedLine {
	###Adds a dotted line to otherObject.  See METHOD(add-line) for more information on object lines.

	my ($self, $otherObject, $theColor );
	( $self, $otherObject, $theColor = Breve::Vector->new( 0.000000, 0.000000, 0.000000 ) ) = @_;

	$self->addLine( $otherObject, $theColor, "- - - - - - - - " );
}

sub addLine {
    ###Adds a line to be drawn from this object to otherObject.  The line can be removed  later using METHOD(remove-line). <P> The optional argument theColor specifies the color of the line.  The default color is the vector (0, 0, 0), black. <P> The optional argument theStyle specifies a pattern for the line.  theStyle is a  string of 16 spaces and/or dashes which specify the line pattern to be drawn. A dotted line, for example, would use the pattern "- - - - - - - - ".  A thickly dashed line would use the pattern "--------		".  If no style is given, a  solid line is drawn.  <P> If a line to otherObject already exists, its color and/or style will be updated.

    my ($self, $otherObject, $theColor, $theStyle );
    ( $self, $otherObject, $theColor = Breve::Vector->new( 0.000000, 0.000000, 0.000000 ), $theStyle = "----------------" ) = @_;
    my $line = undef;

    $self->removeLine( $otherObject );
    $line = Breve::Line->new(); #### MANUAL: MAKE 1 OF THESE!;
    $line->connect( $self, $otherObject, $theColor, $theStyle );
    push @{$self->{ lines }}, $line;
}

sub addMenu {
	###Adds a menu named menuName to the application which will result in a call to theMethod for the calling instance. <p> If the calling instance is the Controller object, then the menu will become the "main" simulation menu.  Otherwise, the menu will become a contextual menu associated with the specific object in the simulation. <p> Note that unlike the METHOD(handle-collision) which sets the collision handler for the whole type (class, that is), this method affects only the instance for which it is called, meaning that each instance of a certain class may have a different menu.

	my ($self, $menuName, $theMethod );
	( $self, $menuName, $theMethod ) = @_;
	my $newMenu = undef;

	$newMenu = Breve::MenuItem->new(); #### MANUAL: MAKE 1 OF THESE!
	$newMenu->createMenu( $menuName, $self, $theMethod );
	$self->addDependency( $newMenu );

	push @{$self->{ menus }}, $newMenu;

	return $newMenu;
}

sub addMenuSeparator {
	###Adds a separator menu item--really just an empty menu item.

	my $self;
	( $self ) = @_;
	my $newMenu = undef;

	$newMenu = Breve::MenuItem->new(); #### MANUAL: MAKE 1 OF THESE!
	$newMenu->createMenu( "", $self, "" );
	$self->addDependency( $newMenu );
	$self->{ menus }.append( $newMenu );
	return $newMenu;
}

sub dearchive {
	my $self;
	( $self ) = @_;
	my $handler = ();

	die("NOT IMPLEMENTED IN PERL!\n");

#	foreach $handler ($self->{ collisionHandlerList }) {
#		if( $handler[ 1 ] ) {
#			Breve::callInternal( $self, "addCollisionHandler", $self, $handler[ 0 ], $handler[ 1 ] );
#		}
#;
#		if( $handler[ 2 ] ) {
#			Breve::callInternal( $self, "setIgnoreCollisionsWith", $self, $handler[ 0 ], 1 );
#		}
#;
#
#	}
#
#
#	if( ( $self->{ texture } > -1 ) ) {
#		$self->setTexture( $self->{ texture } );
#	}
#
#	if( ( $self->{ lightmap } > -1 ) ) {
#		$self->setLightmap( $self->{ lightmap } );
#	}
#
#	if( ( $self->{ bitmap } > -1 ) ) {
#		$self->setBitmap( $self->{ bitmap } );
#	}
#
#	$self->setTextureScaleX( $self->{ textureScaleX } );
#	$self->setTextureScaleY( $self->{ textureScaleY } );
#	$self->setNeighborhoodSize( $self->{ neighborhoodSize } );
#	$self->setColor( $self->{ color } );
#	return $self->SUPER::dearchive();
}

sub delete {
    my $self;
    ( $self ) = @_;
    
    if( $self->{ realWorldPointer } ) {
	Breve::callInternal( $self, "realRemoveObject", $self->{ realWorldPointer } );
    }
    
    breve->deleteInstances( $self->{ menus } );
}

sub deleteInstance {
	###Produces a dialog box (if supported by the current breve  engine) asking if the user wants to delete the object.   This is typically used in response to a user action like a click or menu callback.

	my $self;
	( $self ) = @_;
	my $result = 0;

	$result = $self->{ controller }->showDialog( "Really Delete Instance?", "Deleting this object may cause a fatal error in the simulation.  Really remove it?", "Okay", "Cancel" );
	if( $result ) {
		breve->deleteInstances( $self );
	}

}

sub disableShadows {
	###If shadow volumes are enabled (see OBJECT(Control)), disables shadow volumes for this object. This is the default when shadow volumes have been enabled.

	my $self;
	( $self ) = @_;

	Breve::callInternal( $self, "realSetDrawShadows", $self->{ realWorldPointer }, 0 );
}

sub drawAsPoint {
	###Draws the object as a single point.  This is by far the fastest way to display an agent.  Points can be used to draw upwards of 20,000 agents with a reasonable frame rate, while drawing as many spheres or  bitmaps would slow down the simulation significantly.

	my $self;
	( $self ) = @_;

	Breve::callInternal( $self, "realSetDrawAsPoint", $self->{ realWorldPointer }, 1 );
}

sub enableShadows {
	###If shadow volumes are enabled (see OBJECT(Control)), enables shadow volumes for this object. This is the default when shadow volumes have been enabled.

	my $self;
	( $self ) = @_;

	Breve::callInternal( $self, "realSetDrawShadows", $self->{ realWorldPointer }, 1 );
}

sub getCollisionShape {
	###Returns the OBJECT(Shape) used for collisions associated with this Mobile object.

	my $self;
	( $self ) = @_;

	return $self->{ collisionShape };
}

sub getColor {
	###Returns the color of the object.

	my $self;
	( $self ) = @_;

	return $self->{ color };
}

sub getDisplayShape {
	###Returns the OBJECT(Shape) used for display associated with this Mobile object.

	my $self;
	( $self ) = @_;

	return $self->{ displayShape };
}

sub getLightExposure {
	###When used in conjunction with light exposure detection (OBJECTMETHOD(Control:enable-light-exposure-detection)), this method returns the level of light exposure on this object.

	my $self;
	( $self ) = @_;

	if( $self->{ realWorldPointer } ) {
		return Breve::callInternal( $self, "realGetLightExposure", $self->{ realWorldPointer } );
	}	 else {
		return 0;
	}

}

sub getLocation {
	###Returns the vector location of this object. 

	my $self;
	( $self ) = @_;

	return Breve::callInternal( $self, "realGetLocation", $self->{ realWorldPointer } );
}

sub getNeighborhoodSize {
	###gets the neighborhood size for the current object.

	my $self;
	( $self ) = @_;

	return $self->{ neighborhoodSize };
}

sub getNeighbors {
	###Returns a list of all real objects in the simulation that are within the "neighborhood" range of this object in the world.

	my $self;
	( $self ) = @_;

	return Breve::callInternal( $self, "realGetNeighbors", $self->{ realWorldPointer } );
}

sub getRotation {
	###Deprecated

	my $self;
	( $self ) = @_;

	return Breve::callInternal( $self, "realGetRotation", $self->{ realWorldPointer } );
}

sub getRotationMatrix {
	###Returns the matrix rotation of this object. 

	my $self;
	( $self ) = @_;

	return Breve::callInternal( $self, "realGetRotation", $self->{ realWorldPointer } );
}

sub getShape {
	###Deprecated 

	my $self;
	( $self ) = @_;

	return $self->{ collisionShape };
}

sub getWorldObjectPointer {
	###Used internally.

	my $self;
	( $self ) = @_;

	return $self->{ realWorldPointer };
}

sub handleCollisions {
    ###Adds a collision handler for this object.  When a collision occurs between an instance of the this type and theType, the breve engine will automatically call theMethod of the colliding instance.

    my ($self, $theType, $theMethod );
    ( $self, $theType, $theMethod ) = @_;

    push @{$self->{ collisionHandlerList }}, ( [ $theType, $theMethod, 0 ] ); #not sure if this is right.
    Breve::callInternal( $self, "addCollisionHandler", $self, $theType, $theMethod );
}

sub hideAxis {
	###Hides the X and Y axes for the object.  The axes are hidden by default, so you'll only need this method if you've previously enabled them using METHOD(show-axis).

	my $self;
	( $self ) = @_;

	Breve::callInternal( $self, "realSetDrawAxis", $self->{ realWorldPointer }, 0 );
}

sub hideBoundingBox {
	###Hides the bounding box for the object.  The bounding box is  hidden by default, so you'll only need this method if you've  previously enabled them using METHOD(show-axis).

	my $self;
	( $self ) = @_;

	Breve::callInternal( $self, "realSetBoundingBox", $self->{ realWorldPointer }, 0 );
}

sub hideNeighborLines {
	###Hides lines to this objects neighbors.

	my $self;
	( $self ) = @_;

	Breve::callInternal( $self, "realSetNeighborLines", $self->{ realWorldPointer }, 0 );
}

sub ignoreCollisions {
	###Instructs the engine to ignore physical collisions with theType objects. This does not affect collision callbacks specified with METHOD(handle-collisions).

	my ($self, $theType );
	( $self, $theType ) = @_;

	die("not implemented ignoreCollisions.\n");

	$self->{ collisionHandlerList }.append( [ $theType, 0, 1 ] );
	Breve::callInternal( $self, "setIgnoreCollisionsWith", $self, $theType, 1 );
}

sub init {
	my $self;
	( $self ) = @_;

	$self->SUPER::init();

	$self->{ texture } = -1;
	$self->{ lightmap } = -1;
	$self->{ bitmap } = -1;
	$self->{ textureScaleX } = 16;
	$self->{ textureScaleY } = 16;
	$self->addMenu( "Delete Instance", "deleteInstance" );
	$self->addMenu( "Follow With Camera", "watch" );
}

sub makeInvisible {
	###Makes the object invisible.  Can be made visible again later using the method METHOD(make-visible).

	my $self;
	( $self ) = @_;

	Breve::callInternal( $self, "realSetVisible", $self->{ realWorldPointer }, 0 );
}

sub makeVisible {
	###Makes the object visible again (if it has previously been hidden using METHOD(make-invisible).

	my $self;
	( $self ) = @_;

	Breve::callInternal( $self, "realSetVisible", $self->{ realWorldPointer }, 1 );
}

sub move {
	###Moves this object to location newLocation.

	my ($self, $newLocation );
	( $self, $newLocation ) = @_;

	if(!$self->{ realWorldPointer }) {
	    die( "Attempt to move uninitialized Mobile object." );
	}

	Breve::callInternal( $self, "realSetLocation", $self->{ realWorldPointer }, $newLocation );
}

sub point {
    ###An easier way to rotate an object--this function rotates an object such that the local point theVertex, points towards the world direction theLocation.  In other words, theLocation is where you want the object to face, and theVertex indicates which side of the object is to be considered the "front".

    my ($self, $theVertex, $theLocation );
    ( $self, $theVertex, $theLocation ) = @_;
    my $v = ();
    my $a = 0;
    
    if((!$theVertex->length()) or (!$theLocation->length())) {
	return;
    }
    
    $v = Breve::callInternal( $self, "cross", $theVertex, $theLocation );
    $a = Breve::callInternal( $self, "angle", $theVertex, $theLocation );
    if( $v->length() == 0.000000  ) {
	$self->rotate( $theVertex, 0.010000 );
	return;
	
    }
    
    $self->rotate( $v, $a );
}

sub raytrace {
	###Computes the vector from theLocation towards theDirection that hits the shape of this object. <p> If the object was not hit vector (0, 0, 0) will be returned. <p> The location and direction vector must be given relative to the world's coordinate frame. 

	my ($self, $theLocation, $theDirection );
	( $self, $theLocation, $theDirection ) = @_;

	return Breve::callInternal( $self, "raytrace", $self->{ realWorldPointer }, $theLocation, $theDirection );
}

sub removeAllLines {
	###Removes all lines connecting this object to other objects.

	my $self;
	( $self ) = @_;

	breve->deleteInstances( $self->{ lines } );
	$self->{ lines } = [];
}

sub removeLine {
	###Removes the line connecting this object to otherObject.

	my ($self, $otherObject );
	( $self, $otherObject ) = @_;
	my $line = undef;

	foreach $line ($self->{ lines }) {
		if( $line->isLinked( $otherObject ) ) {
			breve->deleteInstances( $line );
			return;

		}
;

	}


}

sub setBitmap {
	###Deprecated.

	my ($self, $textureNumber );
	( $self, $textureNumber ) = @_;

	$self->{ bitmap } = $textureNumber;
	Breve::callInternal( $self, "realSetBitmap", $self->{ realWorldPointer }, ( $textureNumber + 1 ) );
}

sub setBitmapHeading {
	###If this object is in 2d bitmap mode, the rotation of the  bitmap will be set to radianAngle.

	my ($self, $radianAngle );
	( $self, $radianAngle ) = @_;

	Breve::callInternal( $self, "realSetBitmapRotation", $self->{ realWorldPointer }, $radianAngle );
}

sub setBitmapHeadingPoint {
	###If this object is in 2d bitmap mode, the rotation of the  bitmap will be set to degreeAngle degrees.

	my ($self, $rotationVector );
	( $self, $rotationVector ) = @_;

	Breve::callInternal( $self, "realSetBitmapRotationTowardsVector", $self->{ realWorldPointer }, $rotationVector );
}

sub setBitmapImage {
	###Changes the bitmap of this object to bitmapImage, an instance of  class image.  If bitmapImage is NULL, bitmapping is turned off for the object.

	my ($self, $bitmapImage );
	( $self, $bitmapImage ) = @_;

	if( ( not $bitmapImage ) ) {
		$self->{ bitmap } = -1;
	}	 else {
		$self->{ bitmap } = $bitmapImage->getTextureNumber();
	}

	Breve::callInternal( $self, "realSetBitmap", $self->{ realWorldPointer }, $self->{ bitmap } );
}

sub setBitmapTransparency {
	###Sets the transparency to alphaValue, a number between 0.0  (totally transparent) and 1.0 (fully opaque). 

	my ($self, $alphaValue );
	( $self, $alphaValue ) = @_;

	Breve::callInternal( $self, "realSetAlpha", $self->{ realWorldPointer }, $alphaValue );
}

sub setCollisionShape {
	###Associates a OBJECT(Shape) object with this object for collision purposes.  This shape will not be displayed for this object unless METHOD(set-display-shape) is also called with this shape.  Returns this object.

	my ($self, $theShape );
	( $self, $theShape ) = @_;

	if( (!$theShape ) or (!$theShape->getPointer() )) {
	    die( "attempt to register Mobile object with uninitialized shape.");
	}

	if( $self->{ collisionShape } ) {
	    $self->removeDependency( $self->{ collisionShape } );
	}

	$self->{ collisionShape } = $theShape;
	$self->addDependency( $self->{ collisionShape } );
	Breve::callInternal( $self, "realSetShape", $self->{ realWorldPointer }, $theShape->getPointer() );
	return $self;
}

sub setColor {
	###Sets the color of this object to newColor.

	my ($self, $newColor );
	( $self, $newColor ) = @_;

	$self->{ color } = $newColor;
	Breve::callInternal( $self, "realSetColor", $self->{ realWorldPointer }, $newColor );
}

sub setDisplayShape {
	###Associates a OBJECT(Shape) object with this object for display purposes only. This shape will not be used for collision detection unless it is passed to  METHOD(set-collision-shape) as well.  Returns this object.

	my ($self, $theShape );
	( $self, $theShape ) = @_;

	if((!$theShape ) or (!$theShape->getPointer())) {
	    die("Attempt to register Mobile object with uninitialized shape.");
	}

	if( $self->{ displayShape } ) {
	    $self->removeDependency( $self->{ displayShape } );
	}

	$self->{ displayShape } = $theShape;
	$self->addDependency( $self->{ displayShape } );
	Breve::callInternal( $self, "realSetDisplayShape", $self->{ realWorldPointer }, $theShape->getPointer() );
	return $self;
}

sub setE {
	###Sets the "coefficient of restitution" a value which determines the elasticity of the object in a collision. Valid values range from 0.0 to 1.0, with 0.0 representing a totally inelastic collision (such as a lump of clay) while 1.0 represents a totally (and unrealistically) elastic collision (such as a rubber ball).

	my ($self, $newE );
	( $self, $newE ) = @_;

	$self->{ e } = $newE;
	Breve::callInternal( $self, "realSetCollisionProperties", $self->{ realWorldPointer }, $self->{ e }, $self->{ eT }, $self->{ mu } );
}

sub setET {
	###Sets the "tangential coefficient of restitution", a frictional version of the "coefficient of restitution" described in the documentation for METHOD(set-e).  The value ranges from -1.0 to 1.0.  Negative values mean that friction pushes against the sliding object.

	my ($self, $newET );
	( $self, $newET ) = @_;

	$self->{ eT } = $newET;
	Breve::callInternal( $self, "realSetCollisionProperties", $self->{ realWorldPointer }, $self->{ e }, $self->{ eT }, $self->{ mu } );
}

sub setLightmap {
	###Deprecated.

	my ($self, $textureNumber );
	( $self, $textureNumber ) = @_;

	$self->{ lightmap } = $textureNumber;
	Breve::callInternal( $self, "realSetLightmap", $self->{ realWorldPointer }, ( $self->{ lightmap } + 1 ) );
}

sub setLightmapImage {
	###Sets the object to be displayed using a "lightmap".  A  lightmap uses the texture specified and treats it like a light source.  It's hard to explain.  Give it a try for yourself. <p> set-lightmap only has an effect on sphere shapes.  Other  shapes can be textured, but only spheres can be made into  lightmaps.

	my ($self, $lightmapImage );
	( $self, $lightmapImage ) = @_;

	if( ( not $lightmapImage ) ) {
		$self->{ lightmap } = -1;
	}	 else {
		$self->{ lightmap } = $lightmapImage->getTextureNumber();
	}

	Breve::callInternal( $self, "realSetLightmap", $self->{ realWorldPointer }, $self->{ lightmap } );
}

sub setMu {
	###Sets the coefficient of friction to newMu.  mu is a  parameter controlling friction between two bodies and  may be any value between 0 and infinity.

	my ($self, $newMu );
	( $self, $newMu ) = @_;

	$self->{ mu } = $newMu;
	Breve::callInternal( $self, "realSetCollisionProperties", $self->{ realWorldPointer }, $self->{ e }, $self->{ eT }, $self->{ mu } );
}

sub setNeighborhoodSize {
	###Used in conjunction with METHOD(get-neighbors), this function will set the neighborhood size for the current object.

	my ($self, $size );
	( $self, $size ) = @_;

	$self->{ neighborhoodSize } = $size;
	Breve::callInternal( $self, "realSetNeighborhoodSize", $self->{ realWorldPointer }, $size );
}

sub setRotation {
	###Sets the rotation of this object around vector axis thisAxis  by scalar amount (in radians).  This is an "absolute" rotation--the  current rotation of the object does not affect how the  object will be rotated.  For a rotation relative to the  current orientation, set METHOD(relative-rotate).

	my ($self, $thisAxis, $amount );
	( $self, $thisAxis, $amount ) = @_;
	my $length = 0;

	$length = $thisAxis->length();
	if( ( $length == 0.000000 ) ) {
		return;
	}

	$thisAxis->NORMALIZE();
	Breve::callInternal( $self, "realSetRotation", $self->{ realWorldPointer }, $thisAxis, $amount );
}

sub setRotationEulerAngles {
	###Sets the rotation of this object to the Euler angles specified by angles (in radians).

	my ($self, $angles );
	( $self, $angles ) = @_;
	my $m;
	my $r22 = 0;
	my $r21 = 0;
	my $r20 = 0;
	my $r12 = 0;
	my $r11 = 0;
	my $r10 = 0;
	my $r02 = 0;
	my $r01 = 0;
	my $r00 = 0;

	$r00 = ( ( Breve::callInternal( $self, "cos", $angles->_z() ) * Breve::callInternal( $self, "cos", $angles->_x() ) ) - ( ( Breve::callInternal( $self, "cos", $angles->_y() ) * Breve::callInternal( $self, "sin", $angles->_x() ) ) * Breve::callInternal( $self, "sin", $angles->_z() ) ) );
	$r01 = ( ( Breve::callInternal( $self, "cos", $angles->_z() ) * Breve::callInternal( $self, "sin", $angles->_x() ) ) + ( ( Breve::callInternal( $self, "cos", $angles->_y() ) * Breve::callInternal( $self, "cos", $angles->_x() ) ) * Breve::callInternal( $self, "sin", $angles->_z() ) ) );
	$r02 = ( Breve::callInternal( $self, "sin", $angles->_z() ) * Breve::callInternal( $self, "cos", $angles->_y() ) );
	$r10 = ( ( ( -Breve::callInternal( $self, "sin", $angles->_z() ) ) * Breve::callInternal( $self, "cos", $angles->_x() ) ) - ( ( Breve::callInternal( $self, "cos", $angles->_y() ) * Breve::callInternal( $self, "sin", $angles->_x() ) ) * Breve::callInternal( $self, "cos", $angles->_z() ) ) );
	$r11 = ( ( ( -Breve::callInternal( $self, "sin", $angles->_z() ) ) * Breve::callInternal( $self, "sin", $angles->_x() ) ) + ( ( Breve::callInternal( $self, "cos", $angles->_y() ) * Breve::callInternal( $self, "cos", $angles->_x() ) ) * Breve::callInternal( $self, "cos", $angles->_z() ) ) );
	$r12 = ( Breve::callInternal( $self, "cos", $angles->_z() ) * Breve::callInternal( $self, "sin", $angles->_y() ) );
	$r20 = ( Breve::callInternal( $self, "sin", $angles->_y() ) * Breve::callInternal( $self, "sin", $angles->_x() ) );
	$r21 = ( ( -Breve::callInternal( $self, "sin", $angles->_y() ) ) * Breve::callInternal( $self, "cos", $angles->_x() ) );
	$r22 = Breve::callInternal( $self, "cos", $angles->_y() );

	$m = Breve::Matrix->new(  $r00, $r01, $r02, $r10, $r11, $r12, $r20, $r21, $r22 );
	$self->setRotationMatrix( $m );
}

sub setRotationMatrix {
	###Sets the rotation of this object to the rotation matrix theRotation. Working with matrices can be complicated, so a more simple approach is to use METHOD(rotate).

	my ($self, $theRotation );
	( $self, $theRotation ) = @_;

	Breve::callInternal( $self, "realSetRotationMatrix", $self->{ realWorldPointer }, $theRotation );
}

sub setShape {
	###Sets the shape of this object, for both display and collision detection purposes.  To set the shape for one purpose, but not the other, use the methods METHOD(set-display-shape) or METHOD(set-collision-shape).

	my ($self, $theShape );
	( $self, $theShape ) = @_;

	$self->setDisplayShape( $theShape );
	$self->setCollisionShape( $theShape );
}

sub setTexture {
	###Deprecated -- use METHOD(set-texture-image) instead.

	my ($self, $textureNumber );
	( $self, $textureNumber ) = @_;

	Breve::callInternal( $self, "realSetTexture", $self->{ realWorldPointer }, ( $textureNumber + 1 ) );
}

sub setTextureImage {
	###Changes the texture of this object to textureImage, an instance of  class Image.  If textureImage is NULL texturing is turned off for  the object.

	my ($self, $textureImage );
	( $self, $textureImage ) = @_;

	if(!$textureImage) {
		$self->{ texture } = -1;
	} else {
		$self->{ texture } = $textureImage->getTextureNumber();
	}

	Breve::callInternal( $self, "realSetTexture", $self->{ realWorldPointer }, $self->{ texture } );
}

sub setTextureScale {
	###Changes the "scale" of the texture.  When a texture is applied over a shape, this value is used to decide how large the texture will be in terms of breve-world units.  The default value is 16, meaning that a 16x16 face will have one copy of the textured image. For smaller objects, this number will have to be decreased, or else the texture will be too big and will not be visible.

	my ($self, $scaleSize );
	( $self, $scaleSize ) = @_;

	$self->{ textureScaleX } = $scaleSize;
	Breve::callInternal( $self, "realSetTextureScale", $self->{ realWorldPointer }, $scaleSize, $scaleSize );
}

sub setTextureScaleX {
	###Sets the texture scale in the X dimension.  The Y texture scale  value is unchanged.  See METHOD(set-texture-scale) for more information.

	my ($self, $scaleSize );
	( $self, $scaleSize ) = @_;

	$self->{ textureScaleX } = $scaleSize;
	Breve::callInternal( $self, "realSetTextureScale", $self->{ realWorldPointer }, $scaleSize, $self->{ textureScaleY } );
}

sub setTextureScaleY {
	###Sets the texture scale in the Y dimension.  The X texture scale  value is unchanged.  See METHOD(set-texture-scale) for more information.

	my ($self, $scaleSize );
	( $self, $scaleSize ) = @_;

	$self->{ textureScaleY } = $scaleSize;
	Breve::callInternal( $self, "realSetTextureScale", $self->{ realWorldPointer }, $self->{ textureScaleX }, $scaleSize );
}

sub setTransparency {
	###Sets the transparency of this object to alphaValue, a number  between 1.0 (totally opaque) and 0.0 (fully transparent).

	my ($self, $alphaValue );
	( $self, $alphaValue ) = @_;

	Breve::callInternal( $self, "realSetAlpha", $self->{ realWorldPointer }, $alphaValue );
}

sub showAxis {
	###Shows the X and Y axes for the object.

	my $self;
	( $self ) = @_;

	Breve::callInternal( $self, "realSetDrawAxis", $self->{ realWorldPointer }, 1 );
}

sub showBoundingBox {
	###Shows the bounding box for the object.

	my $self;
	( $self ) = @_;

	Breve::callInternal( $self, "realSetBoundingBox", $self->{ realWorldPointer }, 1 );
}

sub showNeighborLines {
	###Draws lines to this objects neighbors (when neighbor checking is enabled).

	my $self;
	( $self ) = @_;

	Breve::callInternal( $self, "realSetNeighborLines", $self->{ realWorldPointer }, 1 );
}

sub watch {
	###Makes the camera follow this object.

	my $self;
	( $self ) = @_;

	$self->{ controller }->watch( $self );
}


1;

