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

#import <Cocoa/Cocoa.h>

#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

#import <sys/types.h>
#import <sys/dir.h>

#import "slBreveGLView.h"
#import "slTextView.h"
#import "slMovieRecord.h"

#import "steve.h"
#import "engine.h"

enum {
    BX_STOPPED = 1,
    BX_RUN,
    BX_PAUSE
};

@interface slBreveEngine : NSObject {
    IBOutlet id movieSaveAccView;
    IBOutlet id movieFrameNumberText;
    IBOutlet id movieQualityPopup;
    IBOutlet id movieQualityMessageText;

    IBOutlet id displayView;
    IBOutlet id interfaceController;

	IBOutlet id oldSpeedMenuItem;

    slMovieRecord *displayMovie;
    
    NSRecursiveLock *engineLock;
    NSRecursiveLock *threadLock;
    
    brEngine *engine;
	breveFrontend *frontend;

    int runState;

	double speedFactor;

	char *outputPath;
}

- (void)setOutputPath:(char*)path;

/* returns the current brEngine, if it exists */

- (brEngine*)getEngine;
- (int)getRunState;

- (void)initEngine;
- (void)freeEngine;

/* the engine can be locked and unlocked externally so that */
/* no processing is done */

- (void)lock;
- (void)unlock;

/* start, pause, unpause, stop the simulation */

- (int)startSimulationWithText:(char*)buffer withFilename:(char*)name withSavedSimulationFile:(char*)saved fullscreen:(BOOL)full;
- (void)pauseSimulation;
- (void)unpauseSimulation;
- (void)stopSimulation;

- (int)checkSyntax:(char*)buffer withFilename:(char*)name;

/* also used in starting a simulation */

- (int)parseText:(char*)text withFilename:(char*)name withSavedSimulationFile:(char*)name;

- (brEngine*)getEngine;
- (void)runWorld:sender;
- (BOOL)startMovie;
- (void)stopMovie;
- (stInstance*)getSelectedInstance;

- (int)runCommand:(char*)command;

- (void)doSelectionAt:(NSPoint)p;
- (void)doKeyEvent:(char)key isDown:(int)d;

- (void)interfaceActionTag:(int)tag stringValue:(NSString*)s;

char *getLoadNameCallback();
char *getSaveNameCallback();
int dialogCallback(char *title, char *message, char *b1, char *b2);
char *interfaceVersionCallback();
int soundCallback();
int pauseCallback();
void setNibCallback(char *n);
int interfaceSetStringCallback(char *string, int tag);

void *newWindowCallback(char *name, void *graph);
void freeWindowCallback(void *data);
void renderWindowCallback(void *data);

- (IBAction)changeMovieQuality:sender;

- (IBAction)setSpeed:sender;

- (void)go;

@end
