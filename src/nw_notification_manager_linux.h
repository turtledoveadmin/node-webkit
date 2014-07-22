//
//  nw_notification_manager_linux.h
//  content
//
//  Created by Jefry on 11/6/14.
//
//

#ifndef CONTENT_NW_NOTIFICATION_MANAGER_LINUX_H_
#define CONTENT_NW_NOTIFICATION_MANAGER_LINUX_H_

#include "content/nw/src/nw_notification_manager.h"
#include <libnotify/notify.h>

namespace nw {
class NotificationManagerLinux : public NotificationManager {

  std::map<int, NotifyNotification*>  mNotificationIDmap;
  static void onClose(NotifyNotification *notif);


  // internal function for AddDesktopNotification
  virtual bool AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params,
    const int render_process_id, const int render_view_id, const bool worker, const std::vector<SkBitmap>* bitmaps) OVERRIDE;

public:
  explicit NotificationManagerLinux();
  virtual ~NotificationManagerLinux();
  virtual bool AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params,
    const int render_process_id, const int render_view_id, const bool worker) OVERRIDE {
    return AddDesktopNotification(params, render_process_id, render_view_id, worker, NULL);
  }
  virtual bool CancelDesktopNotification(int render_process_id, int render_view_id, int notification_id) OVERRIDE;
};

} // namespace nw

#endif // CONTENT_NW_NOTIFICATION_MANAGER_LINUX_H_
