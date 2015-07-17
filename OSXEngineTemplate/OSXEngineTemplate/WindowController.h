/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#import <Cocoa/Cocoa.h>
#import "EngineView.h"
@interface WindowController : NSWindowController {
    
	// IBOutlet must be used so that, in Inteface Builder,
	// we can connect the view in the NIB to windowedView
	IBOutlet EngineView *view;
}

@end
