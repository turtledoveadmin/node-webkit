// Copyright (c) 2012 Intel Corp
// Copyright (c) 2012 The Chromium Authors
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy 
// of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell co
// pies of the Software, and to permit persons to whom the Software is furnished
//  to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in al
// l copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IM
// PLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNES
// S FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
//  OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WH
// ETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
//  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "content/nw/src/api/app/app.h"
#include "content/nw/src/api/dispatcher_host.h"
#include "content/nw/src/api/event/event.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/values.h"
#include "content/nw/src/api/api_messages.h"
#include "content/nw/src/breakpad_linux.h"
#include "content/nw/src/browser/native_window.h"
#include "content/nw/src/browser/net_disk_cache_remover.h"
#include "content/nw/src/nw_package.h"
#include "content/nw/src/nw_shell.h"
#include "content/nw/src/shell_browser_context.h"
#include "content/common/view_messages.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_process_host.h"
#include "ui/gfx/screen.h"
#include "ui/gfx/display_observer.h"

using base::MessageLoop;
using content::Shell;
using content::ShellBrowserContext;
using content::RenderProcessHost;

namespace nwapi {

namespace {

// Get render process host.
RenderProcessHost* GetRenderProcessHost() {
  RenderProcessHost* render_process_host = NULL;
  std::vector<Shell*> windows = Shell::windows();
  for (size_t i = 0; i < windows.size(); ++i) {
    if (!windows[i]->is_devtools()) {
      render_process_host = windows[i]->web_contents()->GetRenderProcessHost();
      break;
    }
  }

  return render_process_host;
}

void GetRenderProcessHosts(std::set<RenderProcessHost*>& rphs) {
  RenderProcessHost* render_process_host = NULL;
  std::vector<Shell*> windows = Shell::windows();
  for (size_t i = 0; i < windows.size(); ++i) {
    if (!windows[i]->is_devtools()) {
      render_process_host = windows[i]->web_contents()->GetRenderProcessHost();
      rphs.insert(render_process_host);
    }
  }
}

}  // namespace
  
std::string DisplayToJSON(const gfx::Display& display) {
  std::stringstream ret;
  gfx::Rect rect = display.bounds();

  ret << "{\"id\":" << display.id();
  
  ret << ",\"bounds\":{\"x\":" << rect.x()
  << ", \"y\":" << rect.y()
  << ", \"width\":" << rect.width()
  << ", \"height\":" << rect.height() << "}";
  
  rect = display.work_area();
  ret << ",\"work_area\":{\"x\":" << rect.x()
  << ", \"y\":" << rect.y()
  << ", \"width\":" << rect.width()
  << ", \"height\":" << rect.height() << "}";
  
  ret << ",\"scaleFactor\":" << display.device_scale_factor();
  ret << ",\"isBuiltIn\":" << (display.IsInternal() ? "true" : "false");
  ret << "}";
  
  return ret.str();
}
  
class JavaScriptDisplayObserver : BaseEvent, public gfx::DisplayObserver {
  friend class EventListener;
  EventListener* object_;
  gfx::Screen* screen_;
  
  // Called when the |display|'s bound has changed.
  virtual void OnDisplayBoundsChanged(const gfx::Display& display) OVERRIDE {
    base::ListValue arguments;
    arguments.AppendString(DisplayToJSON(display));
    object_->dispatcher_host()->SendEvent(object_, "onDisplayBoundsChanged", arguments);
  }
  
  // Called when |new_display| has been added.
  virtual void OnDisplayAdded(const gfx::Display& new_display) OVERRIDE {
    base::ListValue arguments;
    arguments.AppendString(DisplayToJSON(new_display));
    object_->dispatcher_host()->SendEvent(object_, "onDisplayAdded", arguments);

  }
  
  // Called when |old_display| has been removed.
  virtual void OnDisplayRemoved(const gfx::Display& old_display) OVERRIDE {
    base::ListValue arguments;
    arguments.AppendString(DisplayToJSON(old_display));
    object_->dispatcher_host()->SendEvent(object_, "onDisplayRemoved", arguments);
  }

  static const int id;
  
  JavaScriptDisplayObserver(EventListener* object) : object_(object), screen_(NULL){
  }
  
  virtual ~JavaScriptDisplayObserver() {
    if(screen_)
      screen_->RemoveObserver(this);
  }
  
public:
  void setScreen(gfx::Screen* screen) {
    if(screen_) screen_->RemoveObserver(this);
    screen_ = screen;
    if(screen_) screen_->AddObserver(this);
  }
};
  
const int JavaScriptDisplayObserver::id = EventListener::getUID();

// static
void App::Call(const std::string& method,
               const base::ListValue& arguments) {
  if (method == "Quit") {
    Quit();
    return;
  } else if (method == "CloseAllWindows") {
    CloseAllWindows();
    return;
  } else if (method == "CrashBrowser") {
    int* ptr = NULL;
    *ptr = 1;
  }
  NOTREACHED() << "Calling unknown method " << method << " of App";
}


// static
void App::Call(Shell* shell,
               const std::string& method,
               const base::ListValue& arguments,
               base::ListValue* result) {
  if (method == "GetDataPath") {
    ShellBrowserContext* browser_context =
      static_cast<ShellBrowserContext*>(shell->web_contents()->GetBrowserContext());
    result->AppendString(browser_context->GetPath().value());
    return;
  }else if (method == "GetArgv") {
    nw::Package* package = shell->GetPackage();
    CommandLine* command_line = CommandLine::ForCurrentProcess();
    CommandLine::StringVector args = command_line->GetArgs();
    CommandLine::StringVector argv = command_line->original_argv();

    // Ignore first non-switch arg if it's not a standalone package.
    bool ignore_arg = !package->self_extract();
    for (unsigned i = 1; i < argv.size(); ++i) {
      if (ignore_arg && argv[i] == args[0]) {
        ignore_arg = false;
        continue;
      }

      result->AppendString(argv[i]);
    }

    return;
  } else if (method == "ClearCache") {
    ClearCache(GetRenderProcessHost());
    return;
  } else if (method == "GetPackage") {
    result->AppendString(shell->GetPackage()->package_string());
    return;
  } else if (method == "SetCrashDumpDir") {
    std::string path;
    arguments.GetString(0, &path);
    result->AppendBoolean(SetCrashDumpPath(path.c_str()));
    return;
  } else if (method == "GetScreens") {
    std::stringstream ret;
    const std::vector<gfx::Display>& displays = gfx::Screen::GetNativeScreen()->GetAllDisplays();
    
    if (displays.size() == 0) {
      result->AppendString("{}");
      return;
    }
    
    for (size_t i=0; i<displays.size(); i++) {
      if(i!=0) ret << ",";
      ret << DisplayToJSON(displays[i]);
    }
    result->AppendString("["+ret.str()+"]");
    return;
  } else if (method == "SetScreenChangeCallback") {
    int object_id = 0;
    arguments.GetInteger(0, &object_id);
    EventListener* event_listener = FindDispatcherHost(shell->web_contents()->GetRenderViewHost())->GetApiObject<EventListener>(object_id);
    JavaScriptDisplayObserver* listener = event_listener->AddListener<JavaScriptDisplayObserver>();
    if (listener) listener->setScreen(gfx::Screen::GetNativeScreen());
    result->AppendBoolean(listener != NULL);
    return;
  }

  NOTREACHED() << "Calling unknown sync method " << method << " of App";
}

// static
void App::CloseAllWindows(bool force, bool quit) {
  std::vector<Shell*> windows = Shell::windows();

  for (size_t i = 0; i < windows.size(); ++i) {
    // Only send close event to browser windows, since devtools windows will
    // be automatically closed.
    if (!windows[i]->is_devtools()) {
      // If there is no js object bound to the window, then just close.
      if (force || windows[i]->ShouldCloseWindow(quit))
        // we used to delete the Shell object here
        // but it should be deleted on native window destruction
        windows[i]->window()->Close();
    }
  }
  if (force) {
    // in a special force close case, since we're going to exit the
    // main loop soon, we should delete the shell object asap so the
    // render widget can be closed on the renderer side
    windows = Shell::windows();
    for (size_t i = 0; i < windows.size(); ++i) {
      if (!windows[i]->is_devtools())
        delete windows[i];
    }
  }
}

// static
void App::Quit(RenderProcessHost* render_process_host) {
  // Send the quit message.
  int no_use;
  if (render_process_host) {
    render_process_host->Send(new ViewMsg_WillQuit(&no_use));
  }else{
    std::set<RenderProcessHost*> rphs;
    std::set<RenderProcessHost*>::iterator it;

    GetRenderProcessHosts(rphs);
    for (it = rphs.begin(); it != rphs.end(); it++) {
      RenderProcessHost* rph = *it;
      DCHECK(rph != NULL);

      rph->Send(new ViewMsg_WillQuit(&no_use));
    }
    CloseAllWindows(true);
  }
  // Then quit.
  MessageLoop::current()->PostTask(FROM_HERE, MessageLoop::QuitClosure());
}

// static
void App::EmitOpenEvent(const std::string& path) {
  std::set<RenderProcessHost*> rphs;
  std::set<RenderProcessHost*>::iterator it;

  GetRenderProcessHosts(rphs);
  for (it = rphs.begin(); it != rphs.end(); it++) {
    RenderProcessHost* rph = *it;
    DCHECK(rph != NULL);

    rph->Send(new ShellViewMsg_Open(path));
  }
}

// static
void App::EmitReopenEvent() {
  std::set<RenderProcessHost*> rphs;
  std::set<RenderProcessHost*>::iterator it;

  GetRenderProcessHosts(rphs);
  for (it = rphs.begin(); it != rphs.end(); it++) {
    RenderProcessHost* rph = *it;
    DCHECK(rph != NULL);

    rph->Send(new ShellViewMsg_Reopen());
  }
}

void App::ClearCache(content::RenderProcessHost* render_process_host) {
  render_process_host->Send(new ShellViewMsg_ClearCache());
  nw::RemoveHttpDiskCache(render_process_host->GetBrowserContext(),
                          render_process_host->GetID());
}

}  // namespace nwapi
