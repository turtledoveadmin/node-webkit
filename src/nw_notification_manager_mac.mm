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

- (void) userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification
{
    //NSRunAlertPanel([notification title], [notification informativeText], @"Ok", nil, nil);
    NSNumber *render_process_id = [notification.userInfo objectForKey:@"render_process_id"];
    NSNumber *render_view_id = [notification.userInfo objectForKey:@"render_view_id"];
    NSNumber *notification_id = [notification.userInfo objectForKey:@"notification_id"];
    
    nw::NotificationManager::getSingleton()->DesktopNotificationPostClick(render_process_id.intValue,
                                                                          render_view_id.intValue,
                                                                          notification_id.intValue);
}


@end

namespace nw {
    bool NotificationManagerMac::AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params,
                                                        const int render_process_id, const int render_view_id, const bool worker){
        NSUserNotification *notification = [[NSUserNotification alloc] init];
        [notification setTitle:base::SysUTF16ToNSString(params.title)];
        [notification setInformativeText:base::SysUTF16ToNSString(params.body)];
        //[notification setSubtitle:base::SysUTF8ToNSString(params.origin.spec())];
        notification.hasActionButton = YES;
        
        notification.userInfo  = @{ @"render_process_id" : [NSNumber numberWithInt:render_process_id],
                                    @"render_view_id" : [NSNumber numberWithInt:render_view_id],
                                    @"notification_id" : [NSNumber numberWithInt:params.notification_id],
                                  };
        
        [notification setSoundName:@"NSUserNotificationDefaultSoundName"];

        [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:[NWUserNotificationCenterDelegate defaultNWUserNotificationCenterDelegate]];

        [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
        
        [notification release];
        
        return true;
    }
    
    bool NotificationManagerMac::CancelDesktopNotification(int render_process_id, int render_view_id, int notification_id){
        for (NSUserNotification *notification in [[NSUserNotificationCenter defaultUserNotificationCenter] deliveredNotifications]) {
            NSNumber *current_notification_id = [notification.userInfo objectForKey:@"notification_id"];
            if(current_notification_id.intValue == notification_id){
                [[NSUserNotificationCenter defaultUserNotificationCenter] removeDeliveredNotification:notification];
                return true;
            }
        }
        return false;
    }
} // namespace nw
