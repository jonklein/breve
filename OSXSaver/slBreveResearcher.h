//
//  slBreveResearcher.h
//  slBreveSaver
//
//  Created by jon klein on Mon Feb 23 2004.
//  Copyright (c) 2004 s pi der la nd. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "slBreveSaver.h"


#define SIMULATION_URL @"http://helios.hampshire.edu/~jkCS/CurrentSimulation.tz"


@interface slBreveResearcher : slBreveSaver {

}

- (NSString*)downloadSimulation;

@end
