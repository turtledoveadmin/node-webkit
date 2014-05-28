//
//  nw_notification_manager_win.cc
//  content
//
//  Created by Jefry on 27/5/14.
//
//

#include "content/nw/src/nw_notification_manager_win.h"

namespace nw {
	bool NotificationManagerWin::AddDesktopNotification(const content::ShowDesktopNotificationHostMsgParams& params,
		const int render_process_id, const int render_view_id, const bool worker){
		NOTIFYICONDATA nid = {};
		nid.cbSize = sizeof(nid);
		nid.hWnd = NULL;// window_->GetNativeWindow();
		nid.uID = params.notification_id;
		nid.uFlags = NIF_INFO | NIF_TIP | NIF_SHOWTIP;// | NIF_ICON;
		nid.dwInfoFlags = NIIF_LARGE_ICON | NIIF_INFO;//NIIF_USER;

		lstrcpyn(nid.szInfoTitle, std::wstring(params.title.begin(), params.title.end()).c_str(), ARRAYSIZE(nid.szInfoTitle));
		lstrcpyn(nid.szInfo, std::wstring(params.body.begin(), params.body.end()).c_str(), ARRAYSIZE(nid.szInfo));
		lstrcpyn(nid.szTip, std::wstring(params.body.begin(), params.body.end()).c_str(), ARRAYSIZE(nid.szTip));

		//LoadIconMetric(NULL, MAKEINTRESOURCE(IDI_APPLICATION), LIM_SMALL, &(nid.hIcon));
		//LoadIconMetric(NULL, MAKEINTRESOURCE(IDI_INFORMATION), LIM_SMALL, &(nid.hBalloonIcon));
		return Shell_NotifyIcon(NIM_ADD, &nid);
	}

	bool NotificationManagerWin::CancelDesktopNotification(int render_process_id, int render_view_id, int notification_id){
		return false;
	}
} // namespace nw
