//
//  screen.h
//  content
//
//  Created by Jefry on 19/6/14.
//
//

#ifndef CONTENT_NW_SRC_API_SCREEN_SCREEN_H_
#define CONTENT_NW_SRC_API_SCREEN_SCREEN_H_

#include "base/basictypes.h"

#include <string>

namespace nwapi {

class DispatcherHost;
class Screen {
public:
    
  static void Call(DispatcherHost* dispatcher_host,
                    const std::string& method,
                    const base::ListValue& arguments,
                    base::ListValue* result);
    
private:
  Screen();
  DISALLOW_COPY_AND_ASSIGN(Screen);
};
  
}  // namespace nwapi



#endif //CONTENT_NW_SRC_API_SCREEN_SCREEN_H_
