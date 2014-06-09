//
//  nw_notification_manager_win.cc
//  content
//
//  Created by Jefry on 27/5/14.
//
//

#include "chrome/browser/status_icons/status_icon.h"
#include "chrome/browser/status_icons/status_icon_observer.h"
#include "chrome/browser/status_icons/status_tray.h"

#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_view_host.h"
#include "content/nw/src/browser/native_window.h"
#include "content/nw/src/nw_package.h"
#include "content/nw/src/nw_shell.h"

#include "ui/gfx/image/image.h"
#include "base/strings/utf_string_conversions.h"

#include "content/nw/src/nw_notification_manager_win.h"

namespace nw {
	class TrayObserver : public StatusIconObserver {
	public:
		TrayObserver(NotificationManagerWin* tray)
			: tray_(tray) {
		}

		virtual ~TrayObserver() {
		}

		virtual void OnStatusIconClicked() OVERRIDE{

		}
		
		virtual void OnBalloonEvent(int event) OVERRIDE{
			switch (event) {
			  case NIN_BALLOONHIDE:
				  tray_->DesktopNotificationPostClose(true);
				  tray_->ReleaseNotification();
				  break;
			  case NIN_BALLOONTIMEOUT:
				  tray_->DesktopNotificationPostClose(false);
				  tray_->ReleaseNotification();
				  break;
			  case NIN_BALLOONSHOW:
				  tray_->DesktopNotificationPostDisplay();
				  break;
			}
		}

		virtual void OnBalloonClicked() OVERRIDE {
			tray_->DesktopNotificationPostClick();
			tray_->ReleaseNotification();
		}
	private:
		NotificationManagerWin* tray_;
	};

	NotificationManagerWin::NotificationManagerWin() {
		status_tray_ = StatusTray::Create();
		status_icon_ = nullptr;
		notification_count_ = 0;
	}

	bool NotificationManagerWin::ReleaseNotification() {
		if (notification_count_ > 0) {
			if (notification_count_ == 1) {
				status_icon_->RemoveObserver(status_observer_);
				delete status_observer_;

				status_tray_->RemoveStatusIcon(status_icon_);
				status_icon_ = NULL;
			}
			notification_count_--;
			return true;
		}
		return false;
	}


	NotificationManagerWin::~NotificationManagerWin(){
		ReleaseNotification();

		if (status_tray_){
			delete status_tray_;
			status_tray_ = NULL;
		}
	}

	bool NotificationManagerWin::AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params,
		const int render_process_id, const int render_view_id, const bool worker, const std::vector<SkBitmap>* bitmaps){
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
			content::WebContents::ImageDownloadCallback imageDownloadCallback = base::Bind(&NotificationManagerWin::ImageDownloadCallback);
			int id = shell->web_contents()->DownloadImage(params.icon_url, true, 0, imageDownloadCallback);
			desktop_notification_params_[id] = desktop_notification_params;

			// wait for the image download callback
			return true;
		}

		// if we reach here, it means the function is called from image download callback
		render_process_id_ = render_process_id;
		render_view_id_ = render_view_id;
		notification_id_ = params.notification_id;

		// set the default notification icon as the app icon
		gfx::Image icon = shell->window()->app_icon();

		// status_icon_ is null, it means we need to create and adds it to the tray
		if (status_icon_ == nullptr) {
			nw::Package* package = shell->GetPackage();
			status_icon_ = status_tray_->CreateStatusIcon(StatusTray::NOTIFICATION_TRAY_ICON,
				*(shell->window()->app_icon().ToImageSkia()), UTF8ToUTF16(package->GetName()));
			status_observer_ = new TrayObserver(this);
			status_icon_->AddObserver(status_observer_);
		}
		// add the counter
		notification_count_++;
		// try to get the notification icon image given by image download callback
		if (bitmaps->size())
			icon = gfx::Image::CreateFrom1xBitmap(bitmaps->at(0));

		//show the baloon
		bool result = status_icon_->DisplayBalloon(icon.IsEmpty() ? gfx::ImageSkia() : *icon.ToImageSkia(), params.title, params.body);
		if (!result) {
			DesktopNotificationPostError(L"DisplayBalloon fail");
			ReleaseNotification();
		}
		
		return result;
	}

	bool NotificationManagerWin::CancelDesktopNotification(int render_process_id, int render_view_id, int notification_id){
		//windows  can only have 1 notification, cannot delete existing notification
		return true;
	}
} // namespace nw
