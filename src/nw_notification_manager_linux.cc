//
//  nw_notification_manager_linux.cc
//  content
//
//  Created by Jefry on 11/6/14.
//
//


#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_view_host.h"
#include "content/nw/src/browser/native_window.h"
#include "content/nw/src/nw_package.h"
#include "content/nw/src/nw_shell.h"

#include "ui/gfx/image/image.h"

#include "content/nw/src/nw_notification_manager_linux.h"

namespace nw {

NotificationManagerLinux::NotificationManagerLinux() {
}

NotificationManagerLinux::~NotificationManagerLinux() {
}

bool NotificationManagerLinux::AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params,
  const int render_process_id, const int render_view_id, const bool worker, const std::vector<SkBitmap>* bitmaps) {

  content::RenderViewHost* host = content::RenderViewHost::FromID(render_process_id, render_view_id);
  if (host == nullptr)
    return false;

  content::Shell* shell = content::Shell::FromRenderViewHost(host);

  if (bitmaps == NULL) {
    // called from public function, save the params
    DesktopNotificationParams desktop_notification_params;
    desktop_notification_params.params_ = params;
    desktop_notification_params.render_process_id_ = render_process_id;
    desktop_notification_params.render_view_id_ = render_view_id;

    // download the icon image first
    content::WebContents::ImageDownloadCallback imageDownloadCallback = base::Bind(&NotificationManager::ImageDownloadCallback);
    int id = shell->web_contents()->DownloadImage(params.icon_url, true, 0, imageDownloadCallback);
    desktop_notification_params_[id] = desktop_notification_params;

    // wait for the image download callback
    return true;
  }

  // if we reach here, it means the function is called from image download callback
  // set the default notification icon as the app icon
  gfx::Image icon = shell->window()->app_icon();

  // try to get the notification icon image given by image download callback
  if (bitmaps->size())
    icon = gfx::Image::CreateFrom1xBitmap(bitmaps->at(0));

  // TODO implement the linux notification and add listener to the notification events
  NOTIMPLEMENTED();
  return false;
}

bool NotificationManagerLinux::CancelDesktopNotification(int render_process_id, int render_view_id, int notification_id) {
  // TODO implement linux cancel notification
  NOTIMPLEMENTED();
  return false;
}
} // namespace nw
