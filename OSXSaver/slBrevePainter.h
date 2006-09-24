/* slBrevePainter */

#import <Cocoa/Cocoa.h>

#import "slBreveSaver.h"

@interface slBrevePainter : slBreveSaver {
    IBOutlet NSPopUpButton *filePopup;
    IBOutlet NSTextField *customString;
    IBOutlet NSButtonCell *zoomCheck;

	int custom;
	NSString *selection;
	id customItem;
	NSTimer *timer;
}

- (IBAction)runOpenSheet:sender;
- (void)openPanelDidEnd:(NSOpenPanel *)panel returnCode:(int)returnCode contextInfo:(void *)contextInfo;

- (IBAction)updateImageMenu:sender;
- (IBAction)checkZoomBox:sender;
- (void)setZoom:(int)v;

- (id)getImagesInDirectory:(NSString*)dir inArray:(NSMutableArray*)array;

- (void)setupImageMenu;
- (void)updateImage;
- (void)setImage:(NSString*)image;

@end
