/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

#import "slMovieRecord.h"

/*!
	\brief An Objective-C class interface for recording QuickTime movies.

    slMovieRecord is an ObjectiveC interface to QuickTime to create
    movies from raw pixel data--aimed most specifically at creating
    movies from OpenGL glReadPixel data.

    Current limitations include the assumption that the data is 
    8 bit per sample RGBA.

    1) Create a movie with initWithPath:::.  If this fails, the 
       object will deallocate itself and return NULL.  The bounds 
       must stay constant through the life of the entire movie and
       the value passed in now will determine how large a pixel buffer
       the object will expect later.

    2) Add frames using either addFrameFromRGBAPixels:
       addFrameFromPixMapHandle::.  If either returns a value other
       than 0, the frame was not added and it is likely that future
       frame additions will fail.

       or 

    2) Queue frames using queueFrameFromRGBAPixels:, add them later with 
       addFramesFromQueue.  

    3) Close the movie using closeMovie.

    4) Deallocate the object--create a new instance for a new movie
       but do not reuse this one.
*/

@implementation slMovieRecord

#define MV_CODEC	'rpza'
#define MV_LOSSLESS_CODEC	'SVQ3'
#define MV_RAW_CODEC	'raw '

- initWithPath:(NSString *)path bounds:(NSRect)bounds timeScale:(int)scale qualityLevel:(int)quality {	
    int e;
    PixMapHandle pixMapH;
    FSSpec fs;
    OSErr err;

    CodecQ qualities[] = { codecLowQuality, codecNormalQuality, codecMaxQuality };

    if(quality < 0 || quality > 6) quality = 1;

	if(quality == 6) {
		movieCodec = MV_RAW_CODEC;
	    movieQuality = codecLosslessQuality;        
	} else if(quality == 4) {
		movieCodec = MV_LOSSLESS_CODEC;
	    movieQuality = codecLosslessQuality;
	} else {
		movieCodec = MV_CODEC;
	    movieQuality = qualities[quality];
	}

    /* set the duration and padding to their default values */

    [self setDuration: 1];
    [self setRowPadding: 0];

    theFrameArray = [[NSMutableArray arrayWithCapacity: 200] retain];

    if(theRefNum || theMovie) {
        NSLog(@"Object already initialized\n");
        return NULL;
    }

    err = [path fsSpec: &fs];

    if(err) {
        NSLog(@"Error creating FSSpec for %@\n", path);
        [self release];
        return NULL;
    }

    err = EnterMovies();

    if(err) {
        NSLog(@"Error initializing movies toolbox\n");
        [self release];
        return NULL;
    }

    SetRect(&theBounds, 0, 0, bounds.size.width, bounds.size.height);
    err = NewGWorld(&theGWorld, 32, &theBounds, NULL, NULL, 0);

    if(err) {
        NSLog(@"Error %d creating GWorld\n", err);
        [self release];
        return NULL;
    }

    pixMapH = GetPortPixMap(theGWorld);

    err = GetMaxCompressionSize(pixMapH, &theBounds, 32, movieQuality, movieCodec, bestCompressionCodec, &maxCompressionSize);

    if(err) {
        NSLog(@"Error %d getting maxCompressionSize\n", err);
        [self release];
        return NULL;
    }

    theCompressedData = NewHandle(maxCompressionSize);

    if(!theCompressedData) {
        NSLog(@"Error allocating memory for compressed movie data\n");
        [self release];
        return NULL;
    }

    CreateMovieFile(&fs, 'BREV', smCurrentScript, createMovieFileDeleteCurFile, &theRefNum, &theMovie);

    e = GetMoviesError();

    if(!theMovie || e) {
        NSLog(@"Movie error %d during CreateMovieFile\n", e);
        [self release];
        return NULL;
    }

    theTrack = NewMovieTrack(theMovie, FixRatio(bounds.size.width, 1), FixRatio(bounds.size.height, 1), kNoVolume);

    if(e = GetMoviesError()) {
        NSLog(@"Movie error %d during NewMovieTrack\n", e);
        [self release];
        return NULL;
    }

    theMedia = NewTrackMedia(theTrack, VideoMediaType, scale, NULL, 0);

    if(e = GetMoviesError()) {
        NSLog(@"Movie error %d after NewTrackMedia\n", e);
        [self release];
        return NULL;
    }

    if(e = BeginMediaEdits(theMedia)) {
        NSLog(@"Movie error %d after BeginMediaEdits\n", e);
        [self release];
        return NULL;
    }

    theName = [[path lastPathComponent] retain];

    return self;
}

- (NSString*)name {
    return theName;
}

- (void)setRowPadding:(int)padding {
    theRowPadding = padding;
}

- (int)rowPadding {
    return theRowPadding;
}

- (void)setDuration:(int)duration {
    theDuration = duration;
}

- (int)duration {
    return theDuration;
}

- (void)queueFrameFromRGBAPixels:(char*)ptr {
    int length;
    NSData *data;

    length = (theBounds.right + theRowPadding) * theBounds.bottom * 4;

    data = [NSData dataWithBytes: ptr length: length];

    [theFrameArray addObject: data];
}

- (int)getQueueSize {
    return [theFrameArray count];
}

- (int)addFramesFromQueue {
    unsigned int n;
    int result;
    NSData *data;

    for(n=0;n<[theFrameArray count];n++) {
        data = [theFrameArray objectAtIndex: n];
        result = [self addFrameFromRGBAPixels: (char*)[data bytes]];

        if(result) return result;
    }

    return 0;
}

- (int)addFrameFromQueue:(int)n {
    return [self addFrameFromRGBAPixels: (char*)[[theFrameArray objectAtIndex: n] bytes]];
}

/* 
    Add an image of RGBA (8-bits per sample) data to the movie.  
    It is not uncommon for this type of data to be padded at the 
    end (empty bytes past the actual data for each row of pixels),
    so we can account for this using the rowPadding arg.  If you 
    don't use any padding--just raw pixels, then set padding to 
    zero.
*/

- (int)addFrameFromRGBAPixels:(char*)ptr {
    PixMapHandle pmh;
    char *pmhPixels;
    int result;
    int x, y;
    int rowBytes;
    int ptrRowBytes;

    /* fill in the GWorld pixmap with our data manually. */

    pmh = GetPortPixMap(theGWorld);

    LockPixels(pmh);
    pmhPixels = GetPixBaseAddr(pmh);

    rowBytes = (**pmh).rowBytes & 0x3fff;

    ptrRowBytes = theRowPadding + theBounds.right;

    /* This function works with RGBA data.  Problem is that our GWorld */
    /* Apparently wants ARGB.  Usch. */

    for(x=0;x<theBounds.bottom;x++) {
        for(y=0;y<rowBytes;y+=4) {
            pmhPixels[rowBytes * x + y + 0] = ptr[4 * ptrRowBytes * x + y + 3];
            pmhPixels[rowBytes * x + y + 1] = ptr[4 * ptrRowBytes * x + y + 0];
            pmhPixels[rowBytes * x + y + 2] = ptr[4 * ptrRowBytes * x + y + 1];
            pmhPixels[rowBytes * x + y + 3] = ptr[4 * ptrRowBytes * x + y + 2];
        }
    }

    result = [self addFrameFromPixMapHandle: pmh];

    UnlockPixels(pmh);

    return result;
}

- (int)addFrameFromPixMapHandle:(PixMapHandle)pixMapH {
    ImageDescriptionHandle imageDesc = NULL;
    OSErr err;

    imageDesc = (ImageDescriptionHandle)NewHandle(4);

    err = CompressImage(pixMapH, &theBounds, movieQuality, movieCodec, imageDesc, *theCompressedData);

    if(err) {
        NSLog(@"CompressImage returned error %d\n", err);
        return err;
    }

    err = AddMediaSample(theMedia, theCompressedData, 0, (**imageDesc).dataSize, theDuration, (SampleDescriptionHandle)imageDesc, 1, 0, NULL);

    if(err) {
        NSLog(@"AddMediaSample returned error %d\n", err);
    }

    return err;
}

- (int)closeMovie {
    int r = 0;
    OSErr err;
    short resID = movieInDataForkResID;

    if(theMedia && theTrack) {
        err = EndMediaEdits(theMedia);
        if(err) {
            NSLog(@"Error %d in EndMediaEdits\n", err);
            r = -1;
        }

        err = InsertMediaIntoTrack(theTrack, 0, 0, GetMediaDuration(theMedia), 0x00010000); 
        if(err) {
            NSLog(@"Error %d in InsertMediaIntoTrack\n", err);
            r = -1;
        }
    }

    if(theRefNum) {
        err = AddMovieResource(theMovie, theRefNum, &resID, NULL);
        if(err) {
            NSLog(@"Error %d in AddMovieResource\n", err);
            r = -1;
        }

        err = CloseMovieFile(theRefNum);
        if(err) {
            NSLog(@"Error %d in CloseMovieFile\n", err);
            r = -1;
        }
    }

    return r;
}

- (void)dealloc {
    [theFrameArray removeAllObjects];
    [theFrameArray release];

    DisposeMovie(theMovie);
    DisposeGWorld(theGWorld);

    ExitMovies();

    [theName release];
    [super dealloc];
}

@end

@implementation NSString(Carbon_Additions)

- (int)fsSpec:(FSSpec*)s {
    FSRef fsRef;
    FILE *fp;

    /* this FSSpec code will fail if the file doesn't already exist. */
    /* Make the file exist. */

    if(!(fp = fopen([self cString], "w"))) {
        NSLog(@"Error creating file\n");
        return -1;
    }

    fclose(fp);

    if (FSPathMakeRef([self UTF8String], &fsRef, NULL)) {
        NSLog(@"Error in FSPathMakeRef\n");
        return -1;
    }
    
    if (FSGetCatalogInfo(&fsRef, kFSCatInfoNone, NULL, NULL, s, NULL)) {
        NSLog(@"Error in FSGetCatalogInfo\n");
        return -1;
    }
    
    return 0;
}

@end
