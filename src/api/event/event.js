//
//  event.js
//  content
//
//  Created by Jefry on 18/6/14.
//
//

function EventListener() {
  nw.allocateObject(this, {});
}

EventListener.prototype.handleEvent = function(ev) {
  var jsonArray = [];
  for (var i=1; i<arguments.length; i ++) {
    jsonArray.push(JSON.parse(arguments[i]));
  }
  this[ev].apply(this, jsonArray);
}

exports.EventListener = EventListener;
