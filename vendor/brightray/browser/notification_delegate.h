// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef BRIGHTRAY_BROWSER_NOTIFICATION_DELEGATE_H_
#define BRIGHTRAY_BROWSER_NOTIFICATION_DELEGATE_H_

#include <string>

#include "base/macros.h"

namespace brightray {

class NotificationDelegate {
 public:
  NotificationDelegate();
  NotificationDelegate(std::string notification_id);
  virtual ~NotificationDelegate() {}

  // The native Notification object is destroyed.
  virtual void NotificationDestroyed();

  // Failed to send the notification.
  virtual void NotificationFailed() {}

  virtual void NotificationClick();
  virtual void NotificationClosed();
  virtual void NotificationDisplayed();

  const std::string notificationId() { return notification_id_; }

 private:
  const std::string notification_id_;

DISALLOW_COPY_AND_ASSIGN(NotificationDelegate);
};

}  // namespace brightray

#endif  // BROWSER_NOTIFICATION_DELEGATE_H_
