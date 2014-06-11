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

protected:
  explicit NotificationManager();

  // icon image download callback
  static void ImageDownloadCallback(int id, int http_status, const GURL& image_url, const std::vector<SkBitmap>& bitmaps, const std::vector<gfx::Size>& size);
  struct DesktopNotificationParams {
    content::ShowDesktopNotificationHostMsgParams params_;
    int render_process_id_;
    int render_view_id_;
    bool worker_;
  };

  // map used to stored desktop notification params used by ImageDownloadCallback
  std::map<int, DesktopNotificationParams> desktop_notification_params_;

  // internal function for AddDesktopNotification
  virtual bool AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params,
    const int render_process_id, const int render_view_id, const bool worker, const std::vector<SkBitmap>* bitmaps);

public:
  virtual ~NotificationManager();
  static NotificationManager* getSingleton();
  virtual bool AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params,
    const int render_process_id, const int render_view_id, const bool worker) = 0;
  virtual bool CancelDesktopNotification(int render_process_id, int render_view_id, int notification_id) = 0;

  bool DesktopNotificationPostClick(int render_process_id, int render_view_id, int notification_id);
  bool DesktopNotificationPostClose(int render_process_id, int render_view_id, int notification_id, bool by_user);
  bool DesktopNotificationPostDisplay(int render_process_id, int render_view_id, int notification_id);
  bool DesktopNotificationPostError(int render_process_id, int render_view_id, int notification_id, const string16& message);

};

} // namespace nw

#endif // CONTENT_NW_NOTIFICATION_MANAGER_H_
