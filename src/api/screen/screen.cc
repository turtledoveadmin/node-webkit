//
//  screen.c
//  content
//
//  Created by Jefry on 19/6/14.
//
//

#include "base/values.h"
#include "content/nw/src/api/screen/screen.h"
#include "content/nw/src/api/dispatcher_host.h"
#include "content/nw/src/api/event/event.h"
#include "content/nw/src/nw_shell.h"
#include "ui/gfx/display_observer.h"
#include "ui/gfx/screen.h"

namespace nwapi {
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
    object_->dispatcher_host()->SendEvent(object_, "displayBoundsChanged", arguments);
  }
  
  // Called when |new_display| has been added.
  virtual void OnDisplayAdded(const gfx::Display& new_display) OVERRIDE {
    base::ListValue arguments;
    arguments.AppendString(DisplayToJSON(new_display));
    object_->dispatcher_host()->SendEvent(object_, "displayAdded", arguments);
    
  }
  
  // Called when |old_display| has been removed.
  virtual void OnDisplayRemoved(const gfx::Display& old_display) OVERRIDE {
    base::ListValue arguments;
    arguments.AppendString(DisplayToJSON(old_display));
    object_->dispatcher_host()->SendEvent(object_, "displayRemoved", arguments);
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
void Screen::Call(DispatcherHost* dispatcher_host,
               const std::string& method,
               const base::ListValue& arguments,
               base::ListValue* result) {

  if (method == "GetScreens") {
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
    EventListener* event_listener = dispatcher_host->GetApiObject<EventListener>(object_id);
    JavaScriptDisplayObserver* listener = event_listener->AddListener<JavaScriptDisplayObserver>();
    if (listener) listener->setScreen(gfx::Screen::GetNativeScreen());
    result->AppendBoolean(listener != NULL);
    return;
  }
  
}

} // namespace nwapi
