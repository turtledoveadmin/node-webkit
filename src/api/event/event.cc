//
//  event.cc
//  content
//
//  Created by Jefry on 18/6/14.
//
//

#include "content/nw/src/api/event/event.h"
#include "base/values.h"
#include "content/nw/src/api/dispatcher_host.h"
#include "ui/gfx/screen.h"


namespace nwapi {

EventListener::EventListener(int id,
  const base::WeakPtr<DispatcherHost>& dispatcher_host,
  const base::DictionaryValue& option) : Base(id, dispatcher_host, option) {

}

EventListener::~EventListener() {
  for (std::map<int, BaseEvent*>::iterator i = listerners_.begin(); i != listerners_.end(); i++) {
    delete i->second;
  }
}

} // namespace nwapi
