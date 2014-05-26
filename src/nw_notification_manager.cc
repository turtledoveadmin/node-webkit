//
//  nw_notification_manager.cc
//  content
//
//  Created by Jefry on 23/5/14.
//
//

#include "content/nw/src/nw_notification_manager.h"
#if defined(OS_MACOSX)
#include "content/nw/src/nw_notification_manager_mac.h"
#endif
namespace nw {
    NotificationManager* NotificationManager::singleton_ = nullptr;
    
    NotificationManager::~NotificationManager(){
        singleton_ = nullptr;
    }
    
    NotificationManager* NotificationManager::getSingleton(){
        if(singleton_ == nullptr){
#if defined(OS_MACOSX)
            singleton_ = new NotificationManagerMac();
#endif
        }
        return singleton_;
    }
} // namespace nw
