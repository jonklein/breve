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

#import "slBreveCreatures.h"
#import <unistd.h>

@implementation slBreveCreatures

- (void)loadSettings {
	brEval result;
	
	if(!viewEngine) return;
	
	brInstance *controller = brEngineGetController(viewEngine);
	
	/*
	
	arg.type = AT_VECTOR;
	BRVECTOR(&arg).x = [defaults floatForKey: @"cloudRed"];
	BRVECTOR(&arg).y = [defaults floatForKey: @"cloudGreen"];
	BRVECTOR(&arg).z = [defaults floatForKey: @"cloudBlue"];

	brMethodCallByNameWithArgs(controller, "set-background-color", &ap, 1, &result);

	arg.type = AT_VECTOR;
	BRVECTOR(&arg).x = [defaults floatForKey: @"skyRed"];
	BRVECTOR(&arg).y = [defaults floatForKey: @"skyGreen"];
	BRVECTOR(&arg).z = [defaults floatForKey: @"skyBlue"];
	
	brMethodCallByNameWithArgs(controller, "set-background-color", &ap, 1, &result);

	disableFX = [defaults integerForKey: @"disableEffects"];

	if(disableFX) {
		brMethodCallByName(controller, "disable-shadows", &result);
		brMethodCallByName(controller, "disable-reflections", &result);
	} else {
		brMethodCallByName(controller, "enable-shadows", &result);
		brMethodCallByName(controller, "enable-reflections", &result);	
	}
	 
	*/
	 
}

- (char*)getSimName {
    return "Creatures.tz";
}   

- (char*)getDefaultsName {
	return "breveCreatures";
}


- (IBAction)resetEvolution:(id)sender {
    char *str;

    str = (char*)[[NSString stringWithFormat: @"%@/breveCreatures.xml", outputDirectory] cString];

    if(str) unlink(str);

	// make sure that the XML file does not get recreated.

	[self callControllerMethod: @"disable-save"];
}

- (NSString*)getNibName {
    return @"breveWalker.nib";
}   

@end
