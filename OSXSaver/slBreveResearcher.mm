//
//  slBreveResearcher.m
//  slBreveSaver
//
//  Created by jon klein on Mon Feb 23 2004.
//  Copyright (c) 2004 s pi der la nd. All rights reserved.
//

#import "slBreveResearcher.h"

@implementation slBreveResearcher

- (char*)getSimName {
	return strdup([[self downloadSimulation] cString]);
}

- (char*)getDefaultsName {
	return "breveResearcher";
}

- (NSString*)downloadSimulation {
	NSURL *url = [NSURL URLWithString: SIMULATION_URL];
	NSString *string = [NSString stringWithContentsOfURL: url];
	BOOL r;

	NSString *filename = [outputDirectory stringByAppendingString: @"/breveResearcher-currentSimulation.tz"];
		
	r = [string writeToFile: filename atomically: YES];

	return [filename retain];
}

@end
