//
//  event.h
//  content
//
//  Created by Jefry on 18/6/14.
//
//

#ifndef CONTENT_NW_SRC_API_EVENT_EVENT_H_
#define CONTENT_NW_SRC_API_EVENT_EVENT_H_


#include "base/basictypes.h"

#include "content/nw/src/api/base/base.h"
#include "ui/gfx/display_observer.h"

#include <map>

namespace nwapi {
  
class BaseEvent {
  friend class EventListener;
  DISALLOW_COPY_AND_ASSIGN(BaseEvent);
  
protected:
  BaseEvent(){}
  virtual ~BaseEvent(){}
};
  
class EventListener : public Base {
  std::map<int, BaseEvent*> listerners_;
  
public:
  EventListener(int id,
                const base::WeakPtr<DispatcherHost>& dispatcher_host,
                const base::DictionaryValue& option);
  
  virtual ~EventListener();
  
  static int getUID() {
    static int id = 0;
    return ++id;
  }
  
  template<typename T> T* AddListener() {
    std::map<int, BaseEvent*>::iterator i = listerners_.find(T::id);
    if (i==listerners_.end()) {
      T* listener_object = new T(this);
      listerners_[T::id] = listener_object;
      return listener_object;
    }
    return NULL;
  }
  
  template<typename T> bool RemoveListener() {
    std::map<int, BaseEvent*>::iterator i = listerners_.find(T::id);
    if (i!=listerners_.end()) {
      delete i->second;
      listerners_.erase(i);
      return true;
    }
    return false;
  }
private:
  DISALLOW_COPY_AND_ASSIGN(EventListener);
};

} // namespace nwapi

#endif //CONTENT_NW_SRC_API_EVENT_EVENT_H_
