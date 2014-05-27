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

namespace nw {

	class NotificationManagerWin : public NotificationManager{
	public:
		explicit NotificationManagerWin(){}
		virtual ~NotificationManagerWin(){}
		virtual bool AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params);
	};

} // namespace nw

#endif // CONTENT_NW_NOTIFICATION_MANAGER_WIN_H_
