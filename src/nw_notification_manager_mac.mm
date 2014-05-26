//
//  nw_notification_manager_mac.m
//  content
//
//  Created by Jefry on 23/5/14.
//
//

#import <Cocoa/Cocoa.h>
#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"

#include "content/nw/src/nw_notification_manager_mac.h"

#if !defined(MAC_OS_X_VERSION_10_8) || \
MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_8
@interface NSUserNotificationCenter : NSObject
@end
@interface NSUserNotification : NSObject
@end
@implementation NSUserNotification
@end
#endif

@interface NWUserNotificationCenterDelegate : NSObject<NSUserNotificationCenterDelegate> {
}
@end
@implementation NWUserNotificationCenterDelegate

static NWUserNotificationCenterDelegate *singleton_ = nil;

+ (NWUserNotificationCenterDelegate *)defaultNWUserNotificationCenterDelegate {
    @synchronized(self) {
        if (singleton_ == nil)
            singleton_ = [[self alloc] init];
    }
    return singleton_;

}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center
     shouldPresentNotification:(NSUserNotification *)notification {
    return YES;
}
@end

namespace nw {
    bool NotificationManagerMac::AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params){
        NSUserNotification *notification = [[NSUserNotification alloc] init];
        [notification setTitle:base::SysUTF16ToNSString(params.title)];
        [notification setInformativeText:base::SysUTF16ToNSString(params.body)];
        //[notification setSubtitle:base::SysUTF8ToNSString(params.icon_url.spec())];
        //notification.actionButtonTitle = actionTitle;
        //notification.hasActionButton = YES;

        [notification setSoundName:@"NSUserNotificationDefaultSoundName"];

        [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:[NWUserNotificationCenterDelegate defaultNWUserNotificationCenterDelegate]];

        [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
        
        [notification release];
        
        return true;
    }
} // namespace nw
