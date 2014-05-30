//
//  nw_notification_manager.cc
//  content
//
//  Created by Jefry on 23/5/14.
//
//

#include "content/nw/src/nw_notification_manager.h"
#include "content/public/browser/render_view_host.h"
#if defined(OS_MACOSX)
#include "content/nw/src/nw_notification_manager_mac.h"
#elif defined(OS_WIN)
#include "content/nw/src/nw_notification_manager_win.h"
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
#elif defined(OS_WIN)
			singleton_ = new NotificationManagerWin();
#endif
        }
        return singleton_;
    }
    
    bool NotificationManager::DesktopNotificationPostClick(int render_process_id, int render_view_id, int notification_id){
        content::RenderViewHost* host = content::RenderViewHost::FromID(render_process_id, render_view_id);
        if (host == nullptr)
            return false;
        
        host->DesktopNotificationPostClick(notification_id);
        return true;
    }

	bool NotificationManager::DesktopNotificationPostClose(int render_process_id, int render_view_id, int notification_id, bool by_user){
		content::RenderViewHost* host = content::RenderViewHost::FromID(render_process_id, render_view_id);
		if (host == nullptr)
			return false;

		host->DesktopNotificationPostClose(notification_id, by_user);
		return true;
	}
	
	bool NotificationManager::DesktopNotificationPostDisplay(int render_process_id, int render_view_id, int notification_id){
		content::RenderViewHost* host = content::RenderViewHost::FromID(render_process_id, render_view_id);
		if (host == nullptr)
			return false;

		host->DesktopNotificationPostDisplay(notification_id);
		return true;
	}

	bool NotificationManager::DesktopNotificationPostError(int render_process_id, int render_view_id, int notification_id, const string16& message){
		content::RenderViewHost* host = content::RenderViewHost::FromID(render_process_id, render_view_id);
		if (host == nullptr)
			return false;

		host->DesktopNotificationPostError(notification_id, message);
		return true;
	}
} // namespace nw
