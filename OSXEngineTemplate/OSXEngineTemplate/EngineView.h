/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>
#import "glUtil.h"

@interface EngineView : NSOpenGLView {
	CVDisplayLinkRef displayLink;
}

@end
