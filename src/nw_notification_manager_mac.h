//
//  nw_notification_manager_mac.h
//  content
//
//  Created by Jefry on 23/5/14.
//
//

#ifndef CONTENT_NW_NOTIFICATION_MANAGER_MAC_H_
#define CONTENT_NW_NOTIFICATION_MANAGER_MAC_H_

#include "content/nw/src/nw_notification_manager.h"

namespace nw {
    
    class NotificationManagerMac : public NotificationManager{
    public:
        explicit NotificationManagerMac(){}
        virtual ~NotificationManagerMac(){}
        virtual bool AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params,
                                            const int render_process_id, const int render_view_id, const bool worker);
        virtual bool CancelDesktopNotification(int render_process_id, int render_view_id, int notification_id);

    };
    
} // namespace nw

#endif // CONTENT_NW_NOTIFICATION_MANAGER_MAC_H_
