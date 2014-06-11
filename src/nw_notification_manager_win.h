//
//  nw_notification_manager_win.h
//  content
//
//  Created by Jefry on 27/5/14.
//
//

#ifndef CONTENT_NW_NOTIFICATION_MANAGER_WIN_H_
#define CONTENT_NW_NOTIFICATION_MANAGER_WIN_H_

#include "content/nw/src/nw_notification_manager.h"
class StatusTray;
class StatusIcon;

namespace nw {
class NotificationManagerWin : public NotificationManager{
  // The global presentation of system tray.
  StatusTray* status_tray_;

  // The icons on the system tray.
  StatusIcon* status_icon_;

  // number of notification in the queue
  int notification_count_;

  // decrement the status_icon_count_, if the value is 0 remove the status_icon_ from the tray
  bool ReleaseNotification();

  // Click observer.
  friend class TrayObserver;
  TrayObserver* status_observer_;

  // variable to store the latest notification data, windows can only show 1 notification
  int render_process_id_, render_view_id_, notification_id_;

  // dispatch the events from the latest notification
  bool DesktopNotificationPostClick() {
    return NotificationManager::DesktopNotificationPostClick(render_process_id_, render_view_id_, notification_id_);
  }
  bool DesktopNotificationPostClose(bool by_user) {
    return NotificationManager::DesktopNotificationPostClose(render_process_id_, render_view_id_, notification_id_, by_user);
  }
  bool DesktopNotificationPostDisplay() {
    return NotificationManager::DesktopNotificationPostDisplay(render_process_id_, render_view_id_, notification_id_);
  }
  bool DesktopNotificationPostError(const string16& message) {
    return NotificationManager::DesktopNotificationPostError(render_process_id_, render_view_id_, notification_id_, message);
  }

  // internal function for AddDesktopNotification
  virtual bool AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params,
    const int render_process_id, const int render_view_id, const bool worker, const std::vector<SkBitmap>* bitmaps) OVERRIDE;

public:
  explicit NotificationManagerWin();
  virtual ~NotificationManagerWin();
  virtual bool AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params,
    const int render_process_id, const int render_view_id, const bool worker) OVERRIDE{
    return AddDesktopNotification(params, render_process_id, render_view_id, worker, NULL);
  }
  virtual bool CancelDesktopNotification(int render_process_id, int render_view_id, int notification_id) OVERRIDE;
};

} // namespace nw

#endif // CONTENT_NW_NOTIFICATION_MANAGER_WIN_H_
