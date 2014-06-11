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

class NotificationManagerMac : public NotificationManager {

  // internal function for AddDesktopNotification
  virtual bool AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params,
  const int render_process_id, const int render_view_id, const bool worker, const std::vector<SkBitmap>* bitmaps) OVERRIDE;

public:
  explicit NotificationManagerMac();
  virtual ~NotificationManagerMac(){}
  virtual bool AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params,
    const int render_process_id, const int render_view_id, const bool worker) OVERRIDE;
  virtual bool CancelDesktopNotification(int render_process_id, int render_view_id, int notification_id) OVERRIDE;

};

} // namespace nw

#endif // CONTENT_NW_NOTIFICATION_MANAGER_MAC_H_
