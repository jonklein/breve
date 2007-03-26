
use strict;
use Breve;

package Breve::Image;

###The Image class provides an interface to work with images and  textures.  The individual pixels of the image can be read  or changed by the simulation as desired. <P> The image class can read rendered images from the screen using the  method METHOD(read-pixels), so that agents in the 3D world  can have access to real rendered data.  In addition, the method  METHOD(get-pixel-pointer) can be used to provide a pointer to the  RGBA pixel data so that plugins can access and analyze image data. This could be used, among other things, to implement agent vision.

our @ISA = qw(Breve::Abstract);

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	$self->{ currentHeight } = 0;
	$self->{ currentWidth } = 0;
	$self->{ imageData } = undef;
	$self->{ modified } = 0;
	$self->{ textureNumber } = 0;
	init( $self );
	return $self;
}

sub archive {
	my $self;
	( $self ) = @_;

	$self->{ textureNumber } = -1;
	return $self->SUPER::archive();
}

sub dearchive {
	my $self;
	( $self ) = @_;

	$self->setSize( $self->{ currentWidth }, $self->{ currentHeight } );
	return $self->SUPER::dearchive();
}

sub destroy {
	my $self;
	( $self ) = @_;

	if( $self->{ imageData } ) {
		Breve::callInternal( $self, "imageDataFree", $self->{ imageData } );
	}

}

sub getAlphaPixel {
	###Returns the alpha channel pixel at the image coordinates (x, y). The pixel value is given on a scale from 0.0 to 1.0.

	my ($self, $x, $y );
	( $self, $x, $y ) = @_;

	if( ( not $self->{ imageData } ) ) {
		return 0;
	}

	return Breve::callInternal( $self, "imageGetValueAtCoordinates", $self->{ imageData }, ( ( $x * 4 ) + 3 ), $y );
}

sub getBluePixel {
	###Returns the blue pixel at the image coordinates (x, y). The pixel value is given on a scale from 0.0 to 1.0.

	my ($self, $x, $y );
	( $self, $x, $y ) = @_;

	if( ( not $self->{ imageData } ) ) {
		return 0;
	}

	return Breve::callInternal( $self, "imageGetValueAtCoordinates", $self->{ imageData }, ( ( $x * 4 ) + 2 ), $y );
}

sub getCompressionSize {
	###Compresses the image and returns the compression size.  Useful for  generating simple complexity measures based on image compression.

	my $self;
	( $self ) = @_;

	return Breve::callInternal( $self, "imageGetCompressionSize", $self->{ imageData } );
}

sub getGreenPixel {
	###Returns the green pixel at the image coordinates (x, y). The pixel value is given on a scale from 0.0 to 1.0.

	my ($self, $x, $y );
	( $self, $x, $y ) = @_;

	if( ( not $self->{ imageData } ) ) {
		return 0;
	}

	return Breve::callInternal( $self, "imageGetValueAtCoordinates", $self->{ imageData }, ( ( $x * 4 ) + 1 ), $y );
}

sub getHeight {
	###Returns the width of the image.  

	my $self;
	( $self ) = @_;

	if( ( not $self->{ imageData } ) ) {
		return 0;
	}

	return Breve::callInternal( $self, "imageGetHeight", $self->{ imageData } );
}

sub getImageData {
	my $self;
	( $self ) = @_;

	return $self->{ imageData };
}

sub getPixelPointer {
	###Returns a pointer to the pixels this image is holding in RGBA format.  The size of the buffer is 4 * height * width.  This  data is provided for plugin developers who wish to read or  write pixel data directly.  

	my $self;
	( $self ) = @_;

	return Breve::callInternal( $self, "imageGetPixelPointer", $self->{ imageData } );
}

sub getRedPixel {
	###Returns the red pixel at the image coordinates (x, y). The pixel value is given on a scale from 0.0 to 1.0.

	my ($self, $x, $y );
	( $self, $x, $y ) = @_;

	if( ( not $self->{ imageData } ) ) {
		return 0;
	}

	return Breve::callInternal( $self, "imageGetValueAtCoordinates", $self->{ imageData }, ( $x * 4 ), $y );
}

sub getRgbPixel {
	###Returns the red, green and blue components of the pixel at image coordinates (x, y) as a vector.

	my ($self, $x, $y );
	( $self, $x, $y ) = @_;
	my $b = 0;
	my $g = 0;
	my $r = 0;

	if( ( not $self->{ imageData } ) ) {
		return Breve::Vector->new( 0, 0, 0 );
	}

	$r = Breve::callInternal( $self, "imageGetValueAtCoordinates", $self->{ imageData }, ( $x * 4 ), $y );
	$g = Breve::callInternal( $self, "imageGetValueAtCoordinates", $self->{ imageData }, ( ( $x * 4 ) + 1 ), $y );
	$b = Breve::callInternal( $self, "imageGetValueAtCoordinates", $self->{ imageData }, ( ( $x * 4 ) + 2 ), $y );
	return Breve::Vector->new( $r, $g, $b );
}

sub getTextureNumber {
	###Internal use only.

	my $self;
	( $self ) = @_;

	if( ( $self->{ textureNumber } == -1 ) ) {
		$self->{ textureNumber } = Breve::callInternal( $self, "imageUpdateTexture", $self->{ imageData } );
	}

	return $self->{ textureNumber };
}

sub getWidth {
	###Returns the width of the image.  

	my $self;
	( $self ) = @_;

	if( ( not $self->{ imageData } ) ) {
		return 0;
	}

	return Breve::callInternal( $self, "imageGetWidth", $self->{ imageData } );
}

sub init {
	my $self;
	( $self ) = @_;

	$self->SUPER::init();

	$self->{ textureNumber } = -1;
}

sub initWith {
	my ($self, $imageWidth, $imageHeight );
	( $self, $imageWidth, $imageHeight ) = @_;

	$self->setSize( $imageWidth, $imageHeight );
}

sub iterate {
	my $self;
	( $self ) = @_;

	if( $self->{ modified } ) {
		Breve::callInternal( $self, "imageUpdateTexture", $self->{ imageData } );
		$self->{ modified } = 0;

	}

}

sub load {
	###Loads an image from the file imageFile.

	my ($self, $imageFile );
	( $self, $imageFile ) = @_;

	if( $self->{ imageData } ) {
		Breve::callInternal( $self, "imageDataFree", $self->{ imageData } );
	}

	$self->{ imageData } = Breve::callInternal( $self, "imageLoadFromFile", $imageFile );
	if( ( not $self->{ imageData } ) ) {
		print "Error loading image %s!" % (  $imageFile );
		return 0;

	}

	if( ( $self->{ textureNumber } != -1 ) ) {
		$self->{ modified } = 1;
	}

	return $self;
}

sub readPixels {
	###Reads pixels into this Image from the rendered image on the  screen.  The resulting image can be written to a file or  analyzed if desired.  This is only supported in graphical versions of breve.

	my ($self, $x, $y );
	( $self, $x, $y ) = @_;

	Breve::callInternal( $self, "imageReadPixels", $self->{ imageData }, $x, $y );
}

sub setAlphaPixel {
	###Sets the alpha pixel value at coordinates (x, y) to alphaPixel. alphaPixel should be a value between 0.0 and 1.0.

	my ($self, $alphaPixel, $x, $y );
	( $self, $alphaPixel, $x, $y ) = @_;

	$self->{ modified } = 1;
	Breve::callInternal( $self, "imageSetValueAtCoordinates", $self->{ imageData }, ( ( $x * 4 ) + 3 ), $y, $alphaPixel );
}

sub setBluePixel {
	###Sets the blue pixel value at coordinates (x, y) to bluePixel. bluePixel should be a value between 0.0 and 1.0.

	my ($self, $bluePixel, $x, $y );
	( $self, $bluePixel, $x, $y ) = @_;

	$self->{ modified } = 1;
	Breve::callInternal( $self, "imageSetValueAtCoordinates", $self->{ imageData }, ( ( $x * 4 ) + 2 ), $y, $bluePixel );
}

sub setGreenPixel {
	###Sets the green pixel value at coordinates (x, y) to greenPixel. greenPixel should be a value between 0.0 and 1.0.

	my ($self, $greenPixel, $x, $y );
	( $self, $greenPixel, $x, $y ) = @_;

	$self->{ modified } = 1;
	Breve::callInternal( $self, "imageSetValueAtCoordinates", $self->{ imageData }, ( ( $x * 4 ) + 1 ), $y, $greenPixel );
}

sub setPixel {
	###Deprecated -- for compatibility only.

	my ($self, $pixelVector, $x, $y );
	( $self, $pixelVector, $x, $y ) = @_;

	$self->setRedPixel( $pixelVector->x(), $x, $y );
	$self->setGreenPixel( $pixelVector->y(), $x, $y );
	$self->setBluePixel( $pixelVector->z(), $x, $y );
}

sub setRedPixel {
	###Sets the red pixel value at coordinates (x, y) to redPixel. redPixel should be a value between 0.0 and 1.0.

	my ($self, $redPixel, $x, $y );
	( $self, $redPixel, $x, $y ) = @_;

	$self->{ modified } = 1;
	Breve::callInternal( $self, "imageSetValueAtCoordinates",
			     $self->{ imageData }, ( $x * 4 ), $y, $redPixel );
}

sub setRgbPixel {
	###Sets the red, green and blue pixel values at image coordinates  (x, y) from the values in pixelVector.

	my ($self, $pixelVector, $x, $y );
	( $self, $pixelVector, $x, $y ) = @_;

	$self->setRedPixel( $pixelVector->x(), $x, $y );
	$self->setGreenPixel( $pixelVector->y(), $x, $y );
	$self->setBluePixel( $pixelVector->z(), $x, $y );
}

sub setSize {
	###Creates an empty image buffer with width imageWidth and length imageLength.

	my ($self, $imageWidth, $imageHeight );
	( $self, $imageWidth, $imageHeight ) = @_;

	$self->{ currentHeight } = $imageHeight;
	$self->{ currentWidth } = $imageWidth;
	if( $self->{ imageData } ) {
		Breve::callInternal( $self, "imageDataFree", $self->{ imageData } );
	}

	$self->{ imageData } = Breve::callInternal( $self, "imageDataInit", $imageWidth, $imageHeight );
	return $self;
}

sub write {
	###Write the image to imageFile.  The image is written as a  PNG file, so imageFile should end with .PNG.

	my ($self, $imageFile );
	( $self, $imageFile ) = @_;

	Breve::callInternal( $self, "imageWriteToFile", $self->{ imageData }, $imageFile );
}


1;

