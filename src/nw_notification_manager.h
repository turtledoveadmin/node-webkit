//
//  nw_notification_manager.h
//  content
//
//  Created by Jefry on 23/5/14.
//
//

#ifndef CONTENT_NW_NOTIFICATION_MANAGER_H_
#define CONTENT_NW_NOTIFICATION_MANAGER_H_

#include "base/basictypes.h"
#include "content/public/common/show_desktop_notification_params.h"

namespace nw {
    
    class NotificationManager{
    private:
        static NotificationManager *singleton_;
    public:
        virtual ~NotificationManager();
        static NotificationManager* getSingleton();
        virtual bool AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params) = 0;
    };

} // namespace nw

#endif // CONTENT_NW_NOTIFICATION_MANAGER_H_
