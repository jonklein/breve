
use strict;

package Breve::Control;

our @ISA = qw(Breve::Abstract);

###Summary: a parent class for the "controller" object required for all simulations. <P> The Control object sets up and controls simulations.  Every simulation must have one Control subclass.  The user subclass of Control should  set up the simulation in the "init" method. <p> The Control class also acts as the main interaction between the user and the breve environment itself.  It provides access to elements of  the user interface, for example, so that the user can add menus and other interface features from inside simulations. <p> Because the breve engine is designed to run on a variety of systems, there are varying levels of support for some of these features.  In some cases, the features won't be supported at all on a system.  A background daemon written to use the breve engine, for example, will not place a dialog on the screen and wait for user input.

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;

	$self->{ backgroundColor } = breve->vector();
	$self->{ backgroundTexture } = 0;
	$self->{ blurFactor } = 0;
	$self->{ blurFlag } = 0;
	$self->{ blurMenu } = undef;
	$self->{ camOffset } = breve->vector();
	$self->{ camTarget } = breve->vector();
        $self->{ camera } = undef;
	$self->{ cameraPointer } = undef;
	$self->{ deltaOffset } = breve->vector();
	$self->{ deltaTarget } = breve->vector();
	$self->{ drawEveryFrame } = 0;
	$self->{ drawMenu } = undef;
	$self->{ floorDefined } = 0;
	$self->{ fogColor } = breve->vector();
	$self->{ fogFlag } = 0;
	$self->{ fogIntensity } = 0;
	$self->{ fogMenu } = undef;
	$self->{ frozen } = 0;
	$self->{ genericLinkShape } = undef;
	$self->{ genericShape } = undef;
	$self->{ lightAmbientColor } = breve->vector();
	$self->{ lightDiffuseColor } = breve->vector();
	$self->{ lightFlag } = 0;
	$self->{ lightMenu } = undef;
	$self->{ lightPosition } = breve->vector();
	$self->{ loadedImages } = ();
	$self->{ movie } = undef;
	$self->{ movieMenu } = undef;
	$self->{ offsetting } = 0;
	$self->{ reflectFlag } = 0;
	$self->{ reflectMenu } = undef;
	$self->{ selectedObject } = undef;
	$self->{ shadowFlag } = 0;
        $self->{ shadowMenu } = undef;
	$self->{ shadowVolumeFlag } = 0;
	$self->{ simStep } = 0;
	$self->{ simTime } = 0;
	$self->{ smoothFlag } = 0;
	$self->{ watchObject } = undef;
	$self->{ xRot } = 0;
	$self->{ yRot } = 0;

	$self->SUPER::init();
	$self->init();
     
	return $self;
}

sub acceptUpload {
	###This method is automatically called when an object is uploaded through a OBJECT(NetworkServer).  This implementation simply  prints out a message saying that the object has been received, but your controller may override the method to take other actions.

	my ($self, $uploadedObject, $host );
	( $self, $uploadedObject, $host ) = @_;

	print "object %s sent from host %s" % (  $uploadedObject, $host );
}

sub addMenu {
	###Adds a menu named menuName to the application which will result in a call to theMethod for the calling instance. <p> If the calling instance is the Controller object, then the menu will become the "main" simulation menu.  Otherwise, the menu will become a contextual menu associated with the specific object in the simulation. <p> Note that unlike the METHOD(handle-collision) which sets the collision handler for the whole type (class, that is), this method affects only the instance for which it is called, meaning that each instance of a certain class may have a different menu.

	my ($self, $menuName, $theMethod );
	( $self, $menuName, $theMethod ) = @_;

	my $mitem = Breve::MenuItem->new();
	Breve::brPerlAddInstance($mitem);
	$mitem->createMenu( $menuName, $self, $theMethod );
	return $mitem;
}

sub addMenuSeparator {
	###Adds a separator menu item--really just an empty menu item.

	my $self;
	( $self ) = @_;

	my $msep = Breve::MenuItem->new();
	Breve::brPerlAddInstance($msep);
	$msep->createMenu( "", $self, "" );
	return $msep;
}

sub aimCamera {
	###Deprecated.

	my ($self, $location );
	( $self, $location ) = @_;

	$self->setCameraTarget( $location );
}

sub archive {
	my $self;
	( $self ) = @_;

	$self->{ camTarget } = Breve::callInternal("cameraGetTarget");
	$self->{ camOffset } = Breve::callInternal($self, "cameraGetOffset");
	return $self->SUPER::archive();
}

sub beep {
	###Plays the system beep sound, if supported by the implementation.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "playSound");
}

sub bulletPanCameraOffset {
	###Sets the camera in motion to smoothly change the camera offset over stepCount iteration steps, with the physical simulation frozen in the meantime.

	my ($self, $amount, $stepCount );
	( $self, $amount, $stepCount ) = @_;

	$self->{ frozen } = 1;
	$self->panCameraOffset( $amount, $stepCount );
}

sub catchKey0x7FDown {
	###This method is automatically called when the delete key is pressed  down to delete the selected object.  It deletes the selected instance. Do not call this method manually--it would work, but it would be  a bit roundabout. 

	my $self;
	( $self ) = @_;

	if( $self->{ selectedObject } ) {
		$self->{ selectedObject }->deleteInstance();
	}

}

sub clearScreen {
	###Clears the camera to the current background color.  This  method clears blurred artifacts which are drawn after  enabling METHOD(enable-blur).  If blurring has not been enabled, this method has no visual effect.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "cameraClear", $self->{ cameraPointer } );
}

sub click {
	###Called automatically when the user clicks on an theObject from  the graphical display window of the simulation.  The default behavior of this method is to select the object that was clicked and execute its "click" method, if it exists.   <p> If you do not wish to allow users to select objects in your  simulation, you should implement your own click method in your controller object. <p> If you wish to implement your own click method, but still want to maintain the default behavior of this method, make sure you  call "super click on theObject" from your method. <p> <b>theObject may be NULL</b>--an uninitialized object.  This means that a click occurred, but no object was selected, i.e., a deselection event.  You should test theObject before calling any of its methods.

	my ($self, $theObject );
	( $self, $theObject ) = @_;

	if( ( $self->{ selectedObject } == $theObject ) ) {
		return;
	}

	if( $self->{ selectedObject } ) {
		$self->{ selectedObject }->hideBoundingBox();
		$self->{ selectedObject }->hideAxis();

	}

	$self->{ selectedObject } = $theObject;
	if( ( ( $self->{ selectedObject } and $self->{ selectedObject }->isA( "Link" ) ) and $self->{ selectedObject }->getMultibody() ) ) {
		$self->{ selectedObject } = $self->{ selectedObject }->getMultibody();
	}

	if( ( ( ( not $self->{ selectedObject } ) or ( not $self->{ selectedObject }->canRespond( "showBoundingBox" ) ) ) or ( not $self->{ selectedObject }->canRespond( "showAxis" ) ) ) ) {
		return;
	}

	$self->{ selectedObject }->showBoundingBox();
	$self->{ selectedObject }->showAxis();
}

sub dearchive {
	my $self;
	( $self ) = @_;
	my $image = "";
	my @images = ();

	$self->{ cameraPointer } = Breve::callInternal($self, "getMainCameraPointer");
	$self->{ camera }->setCameraPointer( $self->{ cameraPointer } );
	foreach $image ($self->{ loadedImages }) {
	    push @images, $image;
	}


	foreach $image (@images) {
	    $self->loadImage( $image );
	}


	$self->moveLight( $self->{ lightPosition } );
	$self->setBackgroundTexture( $self->{ backgroundTexture } );
	$self->setBackgroundColor( $self->{ backgroundColor } );
	$self->setFogColor( $self->{ fogColor } );
	$self->pointCamera( $self->{ camTarget }, $self->{ camOffset } );
	$self->setBlurFactor( $self->{ blurFactor } );
	if( ( $self->{ lightFlag } == 1 ) ) {
		$self->enableLighting();
	}

	if( ( $self->{ shadowFlag } == 1 ) ) {
		$self->enableShadows();
	}

	if( ( $self->{ shadowVolumeFlag } == 1 ) ) {
		$self->enableShadowVolumes();
	}

	if( ( $self->{ fogFlag } == 1 ) ) {
		$self->enableFog();
	}

	if( ( $self->{ reflectFlag } == 1 ) ) {
		$self->enableReflections();
	}

	if( ( $self->{ smoothFlag } == 1 ) ) {
		$self->enableSmoothDrawing();
	}

	if( ( $self->{ blurFlag } == 1 ) ) {
		$self->enableBlur();
	}

	if( $self->{ drawEveryFrame } ) {
		$self->enableDrawEveryFrame();
	}	 else {
		$self->disableDrawEveryFrame();
	}

	return 1;
}

sub dearchiveXml {
	###Asks the controller to dearchive an object from an XML file.  The XML file must have been created using  OBJECTMETHOD(Object:archive-as-xml).

	my ($self, $filename );
	( $self, $filename ) = @_;

	return Breve::callInternal($self, "dearchiveXMLObject", $filename );
}

sub disableBlur {
	###Disables blurring.  See METHOD(enable-blur) for more  information.

	my $self;
	( $self ) = @_;

	$self->{ blurFlag } = 0;
	Breve::callInternal($self, "cameraSetBlur", $self->{ cameraPointer }, 0 );
	$self->{ blurMenu }->uncheck();
}

sub disableDrawEveryFrame {
	###Allows the rendering engine to drop frames if the simulation is moving  faster than the display.  This can lead to faster simulations with choppier displays.  Not all breve development environments support this option.  The  Mac OS X application does, as do all threaded command-line breve programs. <p> There is rarely any benefit from using this method, except in instances where the drawing of a scene is complex, and the computation is simple.  The included DLA.tz demo is an example of one such simulation which benefits immensely from this feature.

	my $self;
	( $self ) = @_;

	$self->{ drawEveryFrame } = 0;
	Breve::callInternal($self, "setDrawEveryFrame", 0 );
}

sub disableFog {
	###Disables fog for the main camera.  See METHOD(enable-fog) for more information.

	my $self;
	( $self ) = @_;

	$self->{ fogFlag } = 0;
	Breve::callInternal($self, "cameraSetDrawFog", $self->{ cameraPointer }, 0 );
	$self->{ fogMenu }->uncheck();
}

sub disableFreedInstanceProtection {
	###Disabling freed instance protection means that the breve engine will not attempt to keep track of freed objects, and will yield better memory performance when large numbers of objects are being created and destroyed. <p> The downside is that improper access of freed instances may cause  crashes or unexpected behavior when freed instance protection is  disabled.  Simulations should thus always use freed instance  protection during development and testing, and the feature should only be disabled when the developer is confident that no freed instance bugs exist. <p> Freed instance protection may be reenabled with  METHOD(enable-freed-instance-protection), but only instances freed while instance protection is enabled will be protected.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "setFreedInstanceProtection", 0 );
}

sub disableLightExposureDetection {
	###Disables light exposure detection.  See METHOD(enable-light-exposure-detection).

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "setDetectLightExposure", 0 );
}

sub disableLightExposureDrawing {
	###Disables drawing of the light exposure buffer to the screen.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "setDrawLightExposure", 0 );
}

sub disableLighting {
	###disable lighting for the main camera

	my $self;
	( $self ) = @_;

	$self->{ lightFlag } = 0;
	Breve::callInternal($self, "setDrawLights", $self->{ lightFlag } );
	$self->{ lightMenu }->uncheck();
	$self->{ shadowMenu }->disable();
	$self->{ reflectMenu }->disable();
}

sub disableOutline {
	###Disables outline drawing.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "setDrawOutline", 0 );
}

sub disableReflections {
	###Disable reflections for the main camera.  See METHOD(enable-reflections for  more information on reflections.

	my $self;
	( $self ) = @_;

	$self->{ reflectFlag } = 0;
	Breve::callInternal($self, "setDrawReflection", $self->{ reflectFlag } );
	$self->{ reflectMenu }->uncheck();
}

sub disableShadowVolumes {
	###Disable shadow volumes for the main camera.  See  METHOD(enable-shadow-volumes) for more information on shadows.

	my $self;
	( $self ) = @_;

	$self->{ shadowVolumeFlag } = 0;
	Breve::callInternal($self, "setDrawShadowVolumes", $self->{ shadowFlag } );
}

sub disableShadows {
	###Disable "flat" shadows for the main camera.  See  METHOD(enable-shadows) for more information on shadows.

	my $self;
	( $self ) = @_;

	$self->{ shadowFlag } = 0;
	Breve::callInternal($self, "setDrawShadow", $self->{ shadowFlag } );
	$self->{ shadowMenu }->uncheck();
}

sub disableSmoothDrawing {
	###Disable smooth drawing for the main camera.  See METHOD(enable-smooth-drawing) for more information.

	my $self;
	( $self ) = @_;

	$self->{ smoothFlag } = 0;
	Breve::callInternal($self, "cameraSetDrawSmooth", $self->{ cameraPointer }, $self->{ smoothFlag } );
	$self->{ drawMenu }->uncheck();
}

sub disableText {
	###Disable the timestamp and camera position texts (which appear when  changing the camera angle or position).  The text is on by default.   The text can be re-enabled using METHOD(enable-text).

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "setDrawText", 0 );
}

sub enableBlur {
	###Enables blur.  Blurring simply draws a frame without  totally erasing the previous frame. 

	my $self;
	( $self ) = @_;

	$self->{ blurFlag } = 1;
	Breve::callInternal($self, "cameraSetBlur", $self->{ cameraPointer }, 1 );
	$self->{ blurMenu }->check();
	$self->clearScreen();
}

sub enableDrawEveryFrame {
	###If the method METHOD(disable-draw-every-frame) has been called previously,	 this method will resort to the default behavior, namely that the rendering engine will try to render an image for each and every iteration of the breve engine.

	my $self;
	( $self ) = @_;

	$self->{ drawEveryFrame } = 1;
	Breve::callInternal($self, "setDrawEveryFrame", 1 );
}

sub enableFog {
	###Enables fog for the main camera.  This adds the visual effect of fog to the world.  Fog parameters can be set using methods METHOD(set-fog-color),  METHOD(set-fog-intensity) and METHOD(set-fog-limits). <p> Fog and lightmap effects don't mix.

	my $self;
	( $self ) = @_;

	$self->{ fogFlag } = 1;
	Breve::callInternal($self, "cameraSetDrawFog", $self->{ cameraPointer }, 1 );
	$self->{ fogMenu }->check();
}

sub enableFreedInstanceProtection {
	###Freed instance protection means that the breve engine retains  instances which have been freed in order to make sure that they are not being incorrectly accessed.  This has a small memory cost associated with each freed object. <p> Freed instance protection is enabled by default, so you'll only need to call this method if it has been disabled using  METHOD(disable-freed-instance-protection).

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "setFreedInstanceProtection", 1 );
}

sub enableLightExposureDetection {
	###<B>Experimental</B> <P> Light exposure detection will attempt to tell you how much "sunlight"  is reaching each object in your simulation.  You can set the location of the light source with METHOD(set-light-exposure-source).  Then,  use the method get-light-exposure (in OBJECT(Stationary), OBJECT(Mobile), and OBJECT(Link)) in order to find out how much light was detected for individual objects. <P> The direction of the sunlight is hardcoded towards the world point  (0, 0, 0), and only spreads out to fill an angle of 90 degrees. These limitations may be removed in the future if needed.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "setDetectLightExposure", 1 );
}

sub enableLightExposureDrawing {
	###Enables drawing of the light exposure buffer to the screen.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "setDrawLightExposure", 1 );
}

sub enableLighting {
	###enable lighting for the main camera

	my $self;
	( $self ) = @_;

	$self->{ lightFlag } = 1;

	Breve::callInternal($self, "setDrawLights", $self->{ lightFlag } );
	#$self->{ lightMenu }->check();
	if( $self->{ floorDefined } ) {
		$self->{ shadowMenu }->enable();
		$self->{ reflectMenu }->enable();

	}

}

sub enableOutline {
	###Enables outline drawing.  Outline drawing is a wireframe black and white draw style.  Reflections and textures are ignored when outlining is  enabled.  Outlining is useful for producing diagram-like images.  It  looks cool.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "setDrawOutline", 1 );
}

sub enableReflections {
	###Enable reflections for the main camera.  Reflections are used to draw a mirror image of objects in the world onto a single plane.  Because of  the complexity of drawing reflections, they can only be drawn onto a single plane of a OBJECT(Stationary) object--see the method catch-shadows of OBJECT(Stationary) for more information.  The reflection-catching object must already be defined in order for this method to take effect.

	my $self;
	( $self ) = @_;

	if( ( not $self->{ floorDefined } ) ) {
		return;
	}

	$self->{ reflectFlag } = 1;
	Breve::callInternal($self, "setDrawReflection", $self->{ reflectFlag } );
	$self->{ reflectMenu }->check();
}

sub enableShadowVolumes {
	###Enables shadows drawn using a "shadow volume" algorithm.  This is an alternative to the shadows rendered using METHOD(enable-shadows). <p> Shadow volumes allow all objects in the simulation to shadow one-another, as opposed to having objects only shadow a single plane (as is the case with the METHOD(enable-shadows) algorithm).  Shadow volumes are in fact superior in every way but one: shadow volumes will not generate accurate shadows of bitmapped objects the way the original algorithm will. If you want high-quality bitmap shadows in your simulation, use  METHOD(enable-shadows), otherwise, shadow volumes are likely the better choice.

	my $self;
	( $self ) = @_;

	$self->{ shadowVolumeFlag } = 1;
	Breve::callInternal($self, "setDrawShadowVolumes", $self->{ shadowVolumeFlag } );
}

sub enableShadows {
	###Enable shadows for the main camera.  Shadows use the current  position of the light in order to render shadows onto a flat  plane in the world.  Because of the complexity of drawing shadows,  they can only be drawn onto a single plane of a OBJECT(Stationary)  object--see the method catch-shadows of OBJECT(Stationary) for more  information.  The shadow-catching object must already be defined in order for this method to take effect. <P> For an improved shadowing algorithm, see METHOD(enable-shadow-volumes).

	my $self;
	( $self ) = @_;

	if( ( not $self->{ floorDefined } ) ) {
		return;
	}

	$self->{ shadowFlag } = 1;
	Breve::callInternal($self, "setDrawShadow", $self->{ shadowFlag } );
	$self->{ shadowMenu }->check();
}

sub enableSmoothDrawing {
	###Enable smooth drawing for the main camera.  Smooth drawing enables  a smoother blending of colors, textures and lighting.  This feature is especially noticeable when dealing with spheres or large objects. <p> It is strongly recommended that smooth drawing be enabled whenever  lighting is enabled (see METHOD(enable-lighting)).  Otherwise, major artifacts may be visible, especially on larger polygons. <p> The disadvantage of smooth drawing is a potential performance hit. The degree of this performance hit depends on the number of polygons in the scene.  If speed is an issue, it is often best to disable  both lighting and smooth drawing.

	my $self;
	( $self ) = @_;

	$self->{ smoothFlag } = 1;
	Breve::callInternal($self, "cameraSetDrawSmooth", $self->{ cameraPointer }, $self->{ smoothFlag } );
	$self->{ drawMenu }->check();
}

sub enableText {
	###enables the timestamp and camera position texts (which appear when changing  the camera angle or position).  This is the default setting.  The text can  be disabled using METHOD(disable-text).

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "setDrawText", 1 );
}

sub endSimulation {
	###Ends the simulation gracefully.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "endSimulation");
}

sub execute {
	###Executes the shell command systemCommand using /bin/sh.   Returns the output of command.  Supported on UNIX-based implementations only (Mac OS X and Linux included), not supported on Windows.

	my ($self, $systemCommand );
	( $self, $systemCommand ) = @_;

	return Breve::callInternal($self, "system", $systemCommand );
}

sub getArgument {
	###If this instance of breve was run from the command line, this method returns the argument at index theIndex.  The argument is always returned as a string, though this may naturally be converted to other types  depending on the context.  The arguments (like arrays and lists in <i>steve</i>) are zero based, meaning that the first element has index 0.  <b>The first  argument (the one at index 0) is always the name of the simulation  file</b>.  Use this method in conjunction with  METHOD(get-argument-count).  Make sure you check the number of arguments available before calling this method--requesting an out-of-bounds argument will cause a fatal error in the simulation.

	my ($self, $theIndex );
	( $self, $theIndex ) = @_;

	return Breve::callInternal($self, "getArgv", $theIndex );
}

sub getArgumentCount {
	###If this instance of breve was run from the command line, this method returns the number of arguments passed to the program.  <b>The first argument is always the name of the simulation file</b>.  Use this method in conjunction with METHOD(get-argument).

	my $self;
	( $self ) = @_;

	return Breve::callInternal($self, "getArgc");
}

sub getCameraOffset {
	###Returns the current offset from of the camera from its target.   Note that the camera offset can be changed manually by the user,  so it may be wise to use this function in conjunction with camera  movements to ensure consistency.

	my $self;
	( $self ) = @_;

	return Breve::callInternal($self, "cameraGetOffset");
}

sub getCameraTarget {
	###Returns the current target of the camera.  Note that the camera target can be changed manually by the user, so it may be wise  to use this function in conjunction with camera movements to  ensure consistency.

	my $self;
	( $self ) = @_;

	return Breve::callInternal($self, "cameraGetTarget");
}

sub getDragObject {
	my $self;
	( $self ) = @_;

	if( ( $self->{ selectedObject } and ( $self->{ selectedObject }->isA( "Mobile" ) or $self->{ selectedObject }->isA( "MultiBody" ) ) ) ) {
		return $self->{ selectedObject };
	}

	return 0;
}

sub getGenericLinkShape {
	###Returns a "generic" shape for links, a cube with size (.1, 1, .1).

	my $self;
	( $self ) = @_;

	return $self->{ genericLinkShape };
}

sub getGenericShape {
	###Returns a "generic" shape for agents, a sphere of radius 1.0.

	my $self;
	( $self ) = @_;

	return $self->{ genericShape };
}

sub getHsvColor {
	###All colors in breve expect colors in the RGB format--a vector where  the 3 elements represent red, green and blue intensity on a scale  from 0.0 to 1.0.   <p> This method returns the HSV color vector for a given vector rgbColor in RGB color format.  

	my ($self, $rgbColor );
	( $self, $rgbColor ) = @_;

	return Breve::callInternal($self, "RGBtoHSV", $rgbColor );
}

sub getIntegrationStep {
	###Returns the current integration step size.

	my $self;
	( $self ) = @_;

	return $self->{ simStep };
}

sub getInterfaceVersion {
	###Returns a string identifying the program using the breve engine.   This string is in the format "name/version".

	my $self;
	( $self ) = @_;

	return Breve::callInternal($self, "getInterfaceVersion");
}

sub getIterationStep {
	###Returns the current iteration step size.

	my $self;
	( $self ) = @_;

	return $self->{ simTime };
}

sub getLightExposureCamera {
	###Returns a camera that can be used to control the light detection light-source.

	my $self;
	( $self ) = @_;

	$self->{ camera } = breve->createInstances( breve->Camera, 1 );
	$self->{ camera }->setCameraPointer( Breve::callInternal($self, "getLightExposureCamera") );
}

sub getMainCamera {
	###Returns the OBJECT(Camera) object corresponding to the main camera. This allows you to directly control camera options.

	my $self;
	( $self ) = @_;

	return $self->{ camera };
}

sub getMouseXCoordinate {
	###Returns the X-coordinate of the mouse relative to the simulation window. The value may be negative if the mouse is to the left of the simulation view. See also METHOD(get-mouse-y-coordinate).

	my $self;
	( $self ) = @_;

	return Breve::callInternal($self, "getMouseX");
}

sub getMouseYCoordinate {
	###Returns the Y-coordinate of the mouse. The value may be negative if the mouse is outside of the simulation view, towards the bottom of the screen.   See also METHOD(get-mouse-x-coordinate).

	my $self;
	( $self ) = @_;

	return Breve::callInternal($self, "getMouseY");
}

sub getRealTime {
	###Returns the number of seconds since January 1st, 1970 with microsecond precision.

	my $self;
	( $self ) = @_;

	return Breve::callInternal($self, "getRealTime");
}

sub getRgbColor {
	###All colors in breve expect colors in the RGB format--a vector where  the 3 elements represent red, green and blue intensity on a scale  from 0.0 to 1.0.   <p> This method returns the RGB color vector for a given vector hsvColor in HSV color format.  

	my ($self, $hsvColor );
	( $self, $hsvColor ) = @_;

	return Breve::callInternal($self, "HSVtoRGB", $hsvColor );
}

sub getSelection {
	###Returns the "selected" object--the object which  has been clicked on in the simulation.

	my $self;
	( $self ) = @_;

	if( ( ( $self->{ selectedObject } and $self->{ selectedObject }->isA( "Link" ) ) and $self->{ selectedObject }->getMultibody() ) ) {
		return $self->{ selectedObject }->getMultibody();
	}

	return $self->{ selectedObject };
}

sub getTime {
	###Returns the simulation time of the world.

	my $self;
	( $self ) = @_;

	return Breve::callInternal($self, "getTime");
}

sub init {
	###Initializes the Control object by setting up default values for  variables such as the size of the integration timestep.  Subclasses of Control may override these defaults in their own init functions.

	my $self;
	( $self ) = @_;

	Breve::brPerlAddInstance($self);
	Breve::brPerlSetController($self->{brInstance});

	$self->SUPER::init();

	$self->{ cameraPointer } = Breve::callInternal($self, "getMainCameraPointer");
	$self->{ camera } = Breve::Camera->new();

	$self->addDependency( $self->{ camera } );
	$self->{ camera }->setCameraPointer( $self->{ cameraPointer } );
	$self->{ floorDefined } = 0;
	$self->setIntegrationStep( 0.005000 );
	$self->setIterationStep( 0.050000 );
	$self->enableDrawEveryFrame();
	$self->{ backgroundTexture } = -1;
	$self->setBackgroundColor( Breve::Vector->new( 0.500000, 0.700000, 1.000000 ) );
	$self->setFogColor( Breve::Vector->new( 0.800000, 0.800000, 0.800000 ) );
	$self->{ xRot } = 0.000000;
	$self->{ yRot } = 0.000000;
	$self->moveLight( Breve::Vector->new( 5, 6, 9 ) );
	$self->pointCamera( Breve::Vector->new( 0, 0, 0 ), Breve::Vector->new( 0, 0, 30 ) );
	$self->setBackgroundScrollRate( 0.001000, 0.000100 );
	$self->{ lightMenu } = $self->addMenu( "Use Lighting", "toggleLighting" );
	$self->{ drawMenu } = $self->addMenu( "Use Smooth Shading", "toggleSmooth" );
	$self->{ shadowMenu } = $self->addMenu( "Draw Shadows", "toggleShadows" );
	$self->{ fogMenu } = $self->addMenu( "Draw Fog", "toggleFog" );
	$self->{ reflectMenu } = $self->addMenu( "Draw Reflections", "toggleReflections" );
	$self->{ blurMenu } = $self->addMenu( "Use Motion Blur", "toggleBlur" );
	$self->addMenuSeparator();
	$self->{ movieMenu } = $self->addMenu( "Record Movie to 'simulation.mpeg'", "toggleRecordingToMovie" );
	$self->addMenu( "Save Snapshot to 'simulation.png'", "saveSnapshotToFile" );
	$self->enableSmoothDrawing();
	$self->disableLighting();
	$self->addMenuSeparator();
	
	my $sphere = Breve::Sphere->new();
	my $cube = Breve::Cube->new();
	
	$self->{ genericShape } = $sphere->initWith( 1.000000 );
	$self->{ genericLinkShape } = $cube->initWith( Breve::Vector->new( 0.100000, 1, 0.100000 ) );
}

sub iterate {
	my $self;
	( $self ) = @_;
	my $result = 0;

	if( $self->{ watchObject } ) {
		$self->setCameraTarget( $self->{ watchObject }->getLocation() );
	}

	if( ( not $self->{ frozen } ) ) {
		$result = Breve::callInternal($self, "worldStep", $self->{ simTime }, $self->{ simStep } );
	}

	if( ( $result == -1 ) ) {
		raise Exception( "An error occurred during world simulation." );
	}

	if( $self->{ offsetting } ) {
		$self->setCameraOffset( ( $self->getCameraOffset() + $self->{ deltaOffset } ) );
		$self->setCameraTarget( ( $self->getCameraTarget() + $self->{ deltaTarget } ) );
		$self->{ offsetting } = ( $self->{ offsetting } - 1 );
		if( ( $self->{ offsetting } == 0 ) ) {
			$self->{ frozen } = 0;
		}
	}
}

sub loadImage {
	###Loads an image from a file, returning an OBJECT(Image) object. <P> This method is provided for backwards compatability only. The p <p>

	my ($self, $file );
	( $self, $file ) = @_;
	my $image = undef;

	print "warning: the Control method 'load-image' is now deprecated!";
	$image = breve->createInstances( breve->Image, 1 );
	if( $image->load( $file ) ) {
		$self->{ loadedImages }[ $self->{ loadedImages } ] = $file;

	}	 else {
		breve->deleteInstances( $image );
		return -1;

	}

	return ( $image->getTextureNumber() - 1 );
}

sub loadImageWithoutAlpha {
	###Deprecated.

	my ($self, $file );
	( $self, $file ) = @_;

	return $self->loadImage( $file );
}

sub makeNewInstance {
	###Returns a new instance of the class className.

	my ($self, $className );
	( $self, $className ) = @_;

	return Breve::callInternal($self, "newInstanceForClassString", $className );
}

sub moveLight {
	###Moves the source light to theLocation.  The default position is  (0, 0, 0) which is the origin of the world.  

	my ($self, $theLocation );
	( $self, $theLocation ) = @_;

	$self->{ lightPosition } = $theLocation;
	Breve::callInternal($self, "setLightPosition", $theLocation );
}

sub offsetCamera {
	###Depricated.  

	my ($self, $amount );
	( $self, $amount ) = @_;

	$self->setCameraOffset( $amount );
}

sub panCameraOffset {
	###Sets the camera in motion to smoothly change the camera offset over stepCount iteration steps.

	my ($self, $amount, $stepCount );
	( $self, $amount, $stepCount ) = @_;

	if( ( $stepCount < 2 ) ) {
		$self->offsetCamera( $amount );
		return;

	}

	$self->{ deltaOffset } = ( ( $amount - $self->getCameraOffset() ) / $stepCount );
	$self->{ offsetting } = ( $stepCount + 1 );
}

sub panCameraTarget {
	###Sets the camera in motion to smoothly change the camera target over stepCount iteration steps.

	my ($self, $newTarget, $stepCount );
	( $self, $newTarget, $stepCount ) = @_;

	if( ( $stepCount < 2 ) ) {
		$self->setCameraTarget( $newTarget );
		return;

	}

	$self->{ deltaTarget } = ( ( $newTarget - $self->getCameraTarget() ) / $stepCount );
	$self->{ offsetting } = ( $stepCount + 1 );
}

sub parseXmlNetworkRequest {
	my ($self, $s );
	( $self, $s ) = @_;

	return Breve::callInternal($self, "dearchiveXMLObjectFromString", $s );
}

sub pause {
	###Pauses the simulation as though the user had done so through the user interface.  This method is not supported on all  breve client interfaces.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "pauseSimulation");
}

sub pivotCamera {
	###Rotates the camera (from it's current position) on the x-axis by dx and on the y-axis by dy.

	my ($self, $dx, $dy );
	( $self, $dx = 0.000000, $dy = 0.000000 ) = @_;
	my $rot = Breve::Vector->new();

	$rot = $self->{ camera }->getRotation();
	die("NOT IMPLEMENTED in Control.pm\n");
	#$rot.x = ( $rot.x + $dx );
	#$rot.y = ( $rot.y + $dy );
	#$self->{ camera }->setRotation( $rot.x, $rot.y );
	# fix above commented lines
}

sub pointCamera {
	###Points the camera at the vector location.  The optional argument offset specifies the offset of the camera relative to the location target.

	my ($self, $location, $offset );
	( $self, $location, $offset = Breve::Vector->new( 0.000000, 0.000000, 0.000000 ) ) = @_;

	$self->{ camTarget } = $location;
	$self->setCameraTarget( $location );
	if( ( $offset->length() != 0.000000 ) ) {
		$self->{ camOffset } = $offset;
		$self->setCameraOffset( $offset );

	}

}

sub reportObjectAllocation {
	###Prints data about current object allocation to the log.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "objectAllocationReport");
}

sub saveAsXml {
	###Writes the entire state of the world to an XML file, filename.   filename should have one of the following extensions: .xml,  .brevexml, .tzxml. <p> After saving the state of the world as an XML file, you can later start a new run of the same simulation from the saved state.  You will still need the original steve code which generated the file in order to restart the simulation.

	my ($self, $filename );
	( $self, $filename ) = @_;

	Breve::callInternal($self, "writeXMLEngine", $filename );
}

sub saveSnapshot {
	###Takes a PNG snapshot of the current simulation display and saves  it to a file named filename.  filename should end with ".png".

	my ($self, $filename );
	( $self, $filename ) = @_;

	Breve::callInternal($self, "snapshot", $filename );
}

sub saveSnapshotToFile {
	my $self;
	( $self ) = @_;

	$self->saveSnapshot( "simulation.png" );
}

sub setBackgroundColor {
	###Sets the background color of the rendered world to newColor.

	my ($self, $newColor );
	( $self, $newColor ) = @_;

	$self->{ backgroundColor } = $newColor;
	Breve::callInternal($self, "setBackgroundColor", $newColor );
}

sub setBackgroundScrollRate {
	###Sets the rate of the background image scrolling.  Purely cosmetic.

	my ($self, $xValue, $yValue );
	( $self, $xValue, $yValue ) = @_;

	Breve::callInternal($self, "setBackgroundScroll", $xValue, $yValue );
}

sub setBackgroundTexture {
	my ($self, $newTexture );
	( $self, $newTexture ) = @_;

	$self->{ backgroundTexture } = ( $newTexture + 1 );
	Breve::callInternal($self, "setBackgroundTexture", ( $newTexture + 1 ) );
}

sub setBackgroundTextureImage {
	###Sets the background color of the rendered world to newTexture. newTexture must be a texture returned by the method  METHOD(load-image) or 0 for the default texture.  Setting the texture to -1 will turn off background texturing.

	my ($self, $newTextureImage );
	( $self, $newTextureImage ) = @_;

	$self->{ backgroundTexture } = $newTextureImage->getTextureNumber();
	Breve::callInternal($self, "setBackgroundTexture", $self->{ backgroundTexture } );
}

sub setBlurFactor {
	###Sets the blur level to factor.  Factor should be a value  between 1.0, which corresponds to the highest blur level,  and 0.0, which corresponds to the lowest blur level.   <p>Blur must first be enabled using the method  METHOD(enable-blur).  Note % that a blur level of 0.0 is  still a minor blur--to disable % blur completely, use the  method METHOD(disable-blur).

	my ($self, $factor );
	( $self, $factor ) = @_;

	$self->{ blurFactor } = $factor;
	Breve::callInternal($self, "cameraSetBlurFactor", $self->{ cameraPointer }, $factor );
}

sub setCameraOffset {
	###Offsets the camera from the target by amount.  The target of the camera remains the same.

	my ($self, $offset );
	( $self, $offset ) = @_;

	Breve::callInternal($self, "cameraSetOffset", $offset );
}

sub setCameraRotation {
	###Sets the camera rotation on the x-axis to rx and the y-axis to ry.   This method sets the rotation without regard for the current rotation. If you want to offset the camera rotation from the current position,  use the method METHOD(pivot-camera) instead.

	my ($self, $rx, $ry );
	( $self, $rx, $ry ) = @_;

	$self->{ camera }->setRotation( $rx, $ry );
}

sub setCameraTarget {
	###Aims the camera at location.  The offset of the camera (the offset from the existing target) stays the same.

	my ($self, $location );
	( $self, $location ) = @_;

	Breve::callInternal($self, "cameraSetTarget", $location );
}

sub setDisplayMessage {
	###Sets a text string in the simulation display.  xLoc and yLoc  represent the location of the text.  The coordinate system used goes from (-1, -1) to (1, 1) with (-1, -1) at the lower left hand  corner, (0, 0) in the center of the window and (1, 1) in  the top right hand corner.  The color of the text is set to textColor. <p> Up to 8 messages can be displayed in the simulation window.   messageNumber specifies which message "slot" to modify.   Subsequent calls to this method with the same slot number erase previous entries.

	my ($self, $theString, $messageNumber, $xLoc, $yLoc, $textColor );
	( $self, $theString, $messageNumber, $xLoc, $yLoc, $textColor = Breve::Vector->new( 0.000000, 0.000000, 0.000000 ) ) = @_;

	Breve::callInternal($self, "cameraSetText", $theString, $messageNumber, $xLoc, $yLoc, $textColor );
}

sub setDisplayText {
	###Sets a text string in the simulation display.  xLoc and yLoc  represent the location of the text.  The coordinate system  used goes from (-1, -1) to (1, 1) with (-1, -1) at the lower  left hand corner, (0, 0) in the center of the window and  (1, 1) in the top right hand corner. <p> The optional argument messageNumber may be used to specify  up to 8 different messages.

	my ($self, $theString, $xLoc, $yLoc, $messageNumber );
	( $self, $theString, $xLoc = -0.950000, $yLoc = -0.950000, $messageNumber = 0.000000 ) = @_;

	Breve::callInternal($self, "cameraSetText", $theString, $messageNumber, $xLoc, $yLoc, Breve::Vector->new( 0, 0, 0 ) );
}

sub setDisplayTextScale {
	###Sets the scaling factor for text in the display window.   See METHOD(set-display-text) and METHOD(set-display-message)  for more information on adding text messages to the display  window.

	my ($self, $scale );
	( $self, $scale ) = @_;

	Breve::callInternal($self, "cameraSetTextScale", $scale );
}

sub setFloorDefined {
	my $self;
	( $self ) = @_;

	if( $self->{ lightFlag } ) {
		$self->{ shadowMenu }->enable();
		$self->{ reflectMenu }->enable();

	}

	$self->{ floorDefined } = 1;
}

sub setFogColor {
	###Sets the fog color to newColor.  Fog must first be turned on with METHOD(enable-lighting) before fog is displayed.

	my ($self, $newColor );
	( $self, $newColor ) = @_;

	$self->{ fogColor } = $newColor;
	Breve::callInternal($self, "cameraSetFogColor", $self->{ cameraPointer }, $newColor );
}

sub setFogIntensity {
	###Sets the fog intensity to newIntensity.  Fog must first be turned on with METHOD(enable-lighting) before fog is displayed.

	my ($self, $newIntensity );
	( $self, $newIntensity ) = @_;

	$self->{ fogIntensity } = $newIntensity;
	Breve::callInternal($self, "cameraSetFogIntensity", $self->{ cameraPointer }, $newIntensity );
}

sub setFogLimits {
	###The calculation which calculates fog needs to know where the fog  starts (the point at which the fog appears) and where the fog ends (the point at which the fog has reached it's highest intensity). <p> This method sets the start value to fogStart and the end value to  fogEnd.  fogStart must be greater than or equal to zero.  fogEnd must be greater than fogStart. <p> Fog must first be turned on with METHOD(enable-lighting) before fog  is displayed.

	my ($self, $fogStart, $fogEnd );
	( $self, $fogStart, $fogEnd ) = @_;

	Breve::callInternal($self, "setFogDistances", $fogStart, $fogEnd );
}

sub setIntegrationStep {
	###Sets the integration stepsize to timeStep.  The integration stepsize determines how quickly the simulation runs: large values (perhaps as high as 1 second) mean that the simulation runs quickly at the  cost of accuracy, while low values mean more accuracy, but slower simulations. <p> The control object and its subclasses set the integration timeStep to reasonable values, so this method should only be invoked by expert users with a firm grasp of how they want their simulation to run. <p> Additionally, this value is only used as a suggestion--the  integrator itself may choose to adjust the integration stepsize according to the accuracy of previous timesteps.

	my ($self, $timeStep );
	( $self, $timeStep ) = @_;

	$self->{ simStep } = $timeStep;
}

sub setInterfaceItem {
	###This method will set the interface item tag to newValue.  This is for simulations which have an OS X nib file associated with them.

	my ($self, $tag, $newValue );
	( $self, $tag, $newValue ) = @_;

	return Breve::callInternal($self, "setInterfaceString", $newValue, $tag );
}

sub setIterationStep {
	###Sets the iteration stepsize to timeStep.  The iteration stepsize is simply the number of simulated seconds run between calling the  controller object's "iterate" method. <b>This value may not be smaller than the integration timestep</b>.  <p> The control object and its subclasses set the iteration stepsize  to reasonable values, so this method should only be invoked by expert users with a firm grasp of how they want their simulation to run.  Small values slow down the simulation considerably. <p> For physical simulations, the iteration stepsize should be  considerably larger than the integration stepsize.  The iteration stepsize, in this case, can be interpreted as the reaction time of the agents in the world to changes in their environment.

	my ($self, $timeStep );
	( $self, $timeStep ) = @_;

	$self->{ simTime } = $timeStep;
	if( ( $self->{ simStep } > $self->{ simTime } ) ) {
		$self->{ simTime } = $self->{ simStep };
	}

}

sub setLightAmbientColor {
	###Sets the ambient, or background, color of the light to newColor.   Only has an effect on the rendering when lighting has been turned  on using METHOD(enable-lighting).

	my ($self, $newColor );
	( $self, $newColor ) = @_;

	$self->{ lightAmbientColor } = $newColor;
	Breve::callInternal($self, "setLightAmbientColor", $newColor );
}

sub setLightColor {
	###Sets the color of the light to newColor.  Only has an effect on the rendering when lighting has been turned on using  METHOD(enable-lighting). <p> This method sets both the ambient and diffuse colors, which can also be set individually with METHOD(set-light-ambient-color) and  METHOD(set-light-diffuse-color).

	my ($self, $newColor );
	( $self, $newColor ) = @_;

	$self->setLightAmbientColor( $newColor );
	$self->setLightDiffuseColor( $newColor );
}

sub setLightDiffuseColor {
	###Sets the diffuse, or foreground, color of the light to newColor.   Only has an effect on the rendering when lighting has been turned  on using METHOD(enable-lighting). <p> The diffuse color is the color coming directly from the light, as opposed to the "ambient" light that is also generated.

	my ($self, $newColor );
	( $self, $newColor ) = @_;

	$self->{ lightDiffuseColor } = $newColor;
	Breve::callInternal($self, "setLightDiffuseColor", $newColor );
}

sub setLightExposureSource {
	###Changes the light source for calculating exposure.  See  METHOD(enable-light-exposure-detection).

	my ($self, $source );
	( $self, $source ) = @_;

	Breve::callInternal($self, "setLightExposureSource", $source );
}

sub setOutputFilter {
	###Sets the output filter level.  This value determines the level of  detail used in printing simulation engine errors and messages. The default value, 0, prints only regular output.  An output filter of 50 will print out all normal output as well as some warnings and  other information useful mostly to breve developers.  Other values may be added in the future to allow for more granularity of error  detail.

	my ($self, $filterLevel );
	( $self, $filterLevel ) = @_;

	Breve::callInternal($self, "setOutputFilter", $filterLevel );
}

sub setRandomSeed {
	###Sets the random seed to newSeed.  Setting the random seed determines the way random numbers are chosen.  Two runs which use the same  random seed will yield the same exact random numbers.  Thus, by setting the random number seed manually, you can make simulations repeatable.

	my ($self, $newSeed );
	( $self, $newSeed ) = @_;

	Breve::callInternal($self, "randomSeed", $newSeed );
}

sub setRandomSeedFromDevRandom {
	###Sets the random seed to a value read from /dev/random (if available). <P> By default, breve sets the random seed based on the current time.  This is generally sufficient for most simulations.  However, if  you are dealing with a setup in which multiple simulations might  be launched simultaneously (such as a cluster setup), then you may  have a situation in which the same random seed would be used for  multiple runs, and this will make you unhappy.  Using this method will restore happiness and harmony to your life.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "randomSeedFromDevRandom");
}

sub setZClip {
	###Sets the Z clipping plan to theDistance.  The Z clipping plan  determines how far the camera can see.  A short Z clipping distance means that objects far away will not be drawn.   <p> The default value is 200.0 and this works well for most simulations, so there is often no need to use this method. <p> Using a short Z clipping distance improves drawing quality, avoids  unnecessary rendering and can speed up drawing during the simulation.   However, it may also cause objects you would like to observe in the  simulation to not be drawn because they are too far away.

	my ($self, $theDistance );
	( $self, $theDistance ) = @_;

	Breve::callInternal($self, "setZClip", $theDistance );
}

sub showDialog {
	###Shows a dialog box (if supported by the current breve environment) and waits for the user to click on one of the buttons.   <p> If the "yes button" is clicked on, the method returns 1--if the  "no button" is clicked, or if the feature is not supported, 0 is returned.

	my ($self, $title, $message, $yesString, $noString );
	( $self, $title, $message, $yesString, $noString ) = @_;

	return Breve::callInternal($self, "dialogBox", $title, $message, $yesString, $noString );
}

sub sleep {
	###Pauses execution for s seconds.  s does not have to be a whole  number.  Sleeping for a fraction of a second at each iteration (inside your controller's iterate method) can effectively slow  down a simulation which is too fast to observe.

	my ($self, $s );
	( $self, $s ) = @_;

	Breve::callInternal($self, "sleep", $s );
}

sub stacktrace {
	###Prints out a breve stacktrace--all of the methods which have been called to get to this point in the simulation.  This method is  useful for debugging.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "stacktrace");
}

sub toggleBlur {
	###Toggle motion blur for the main camera.  See METHOD(enable-blur) for  more information on reflections.

	my $self;
	( $self ) = @_;

	if( ( $self->{ blurFlag } == 1 ) ) {
		$self->disableBlur();
	}	 else {
		$self->enableBlur();
	}

}

sub toggleFog {
	###Toggle fog for the main camera

	my $self;
	( $self ) = @_;

	if( ( $self->{ fogFlag } == 1 ) ) {
		$self->disableFog();
	}	 else {
		$self->enableFog();
	}

}

sub toggleLighting {
	###toggle lighting for the main camera

	my $self;
	( $self ) = @_;

	if( ( $self->{ lightFlag } == 1 ) ) {
		$self->disableLighting();
	}	 else {
		$self->enableLighting();
	}

}

sub toggleRecordingToMovie {
	my $self;
	( $self ) = @_;

	if( $self->{ movie } ) {
		$self->{ movieMenu }->uncheck();
		$self->{ movie }->close();
		breve->deleteInstances( $self->{ movie } );
		return;

	}

	$self->{ movie } = breve->createInstances( breve->Movie, 1 );
	$self->{ movie }->record( "simulation.mpeg" );
	$self->{ movieMenu }->check();
}

sub toggleReflections {
	###Toggle reflections for the main camera.  See METHOD(enable-reflections) for  more information on reflections.

	my $self;
	( $self ) = @_;

	if( ( not $self->{ floorDefined } ) ) {
		return;
	}

	$self->{ shadowFlag } = 1;
	if( ( $self->{ reflectFlag } == 1 ) ) {
		$self->disableReflections();
	}	 else {
		$self->enableReflections();
	}

}

sub toggleShadows {
	###Toggle shadows for the main camera.  See METHOD(enable-shadows) and  METHOD(disable-shadows) for more information on shadows.  

	my $self;
	( $self ) = @_;

	if( ( $self->{ shadowFlag } == 1 ) ) {
		$self->disableShadows();
	}	 else {
		$self->enableShadows();
	}

}

sub toggleSmooth {
	###Toggle smooth drawing for the main camera.  See METHOD(enable-smooth-drawing) and  METHOD(disable-smooth-drawing) for more information.

	my $self;
	( $self ) = @_;

	if( ( $self->{ smoothFlag } == 1 ) ) {
		$self->disableSmoothDrawing();
	}	 else {
		$self->enableSmoothDrawing();
	}

}

sub uniqueColor {
	###Returns a unique color for each different value of n up  to 198.  These colors are allocated according to an  algorithm which attempts to give distinguishable colors, though this is subjective and not always possible.

	my ($self, $n );
	( $self, $n ) = @_;

	return Breve::callInternal($self, "uniqueColor", $n );
}

sub unpause {
	###Pauses the simulation as though the user had done so through the user interface.  This method is not supported on all  breve client interfaces.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "unpauseSimulation");
}

sub updateNeighbors {
	###The neighborhood for each object in the simulation is the set of  other objects within a specified radius--calling this method will update the neighborhood list for each object.  This method is  only useful in conjunction with methods in OBJECT(Real) which  set the neighborhood size and then later retrieve the neighbor list.

	my $self;
	( $self ) = @_;

	Breve::callInternal($self, "updateNeighbors");
}

sub watch {
	###Points the camera at the OBJECT(Mobile) object theObject.  If  theObject is passed in as 0, then the camera will stop watching a previously watched object.

	my ($self, $theObject );
	( $self, $theObject ) = @_;

	$self->{ watchObject } = $theObject;
}

sub zoomCamera {
	###Zooms the camera to theDistance away from the current target-- whether the target is a vector or object.

	my ($self, $theDistance );
	( $self, $theDistance ) = @_;

	Breve::callInternal($self, "cameraSetZoom", $theDistance );
}


1;

