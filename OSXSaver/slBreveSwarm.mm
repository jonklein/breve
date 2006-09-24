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

#import "slBreveSwarm.h"

@implementation slBreveSwarm

- (void)loadSettings {
    behavior = [defaults integerForKey: @"flockBehavior"];
    [flockBehaviorPopup selectItemAtIndex: behavior];
    [super loadSettings];

	if(viewEngine) {
		// viewEngine->world->backgroundTextureColor.x = [defaults floatForKey: @"cloudRed"];
		// viewEngine->world->backgroundTextureColor.y = [defaults floatForKey: @"cloudGreen"];
		// viewEngine->world->backgroundTextureColor.z = [defaults floatForKey: @"cloudBlue"];

		// viewEngine->world->backgroundColor.x = [defaults floatForKey: @"skyRed"];
		// viewEngine->world->backgroundColor.y = [defaults floatForKey: @"skyGreen"];
		// viewEngine->world->backgroundColor.z = [defaults floatForKey: @"skyBlue"];
	}

	disableFX = [defaults integerForKey: @"disableEffects"];

	if(disableFX) {
		// viewEngine->camera->drawReflection = 0;
		// viewEngine->camera->drawShadow = 0;
		// viewEngine->camera->recompile = 1;
	} 
}


- (char*)getDefaultsName {
	return "breveSwarm";
}

- (void)saveSettings {
    behavior = [flockBehaviorPopup indexOfSelectedItem];
    [defaults setInteger: behavior forKey: @"flockBehavior"];
    [super saveSettings];
}

- (void)startBreveSimulation {
    int action;

    srandom(time(NULL));
    if(behavior == 3) action = random() % 3;
    else action = behavior;

    switch(action) {
        case 0:
            [self callControllerMethod: @"flock-obediently"];
            break;
        case 1:
            [self callControllerMethod: @"flock-normally"];
            break;
        case 2:
            [self callControllerMethod: @"flock-wackily"];
            break;
    }

    [super startBreveSimulation];
}

- (char*)getSimName {
    return "SwarmSaver.tz";
}

- (NSString*)getNibName {
    return @"breveSwarm.nib";
}

@end
