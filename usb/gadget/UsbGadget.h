/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <android-base/file.h>
#include <android-base/properties.h>
#include <android-base/unique_fd.h>
#include <android-base/parseint.h>
#include <android-base/strings.h>
#include <aidl/android/hardware/usb/gadget/BnUsbGadget.h>
#include <aidl/android/hardware/usb/gadget/BnUsbGadgetCallback.h>
#include <aidl/android/hardware/usb/gadget/GadgetFunction.h>
#include <aidl/android/hardware/usb/gadget/IUsbGadget.h>
#include <aidl/android/hardware/usb/gadget/IUsbGadgetCallback.h>
#include <pixelusb/UsbGadgetAidlCommon.h>
#include <sched.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <utils/Log.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

namespace aidl {
namespace android {
namespace hardware {
namespace usb {
namespace gadget {

using ::aidl::android::hardware::usb::gadget::GadgetFunction;
using ::aidl::android::hardware::usb::gadget::IUsbGadgetCallback;
using ::aidl::android::hardware::usb::gadget::IUsbGadget;
using ::aidl::android::hardware::usb::gadget::Status;
using ::aidl::android::hardware::usb::gadget::UsbSpeed;
using ::android::base::GetProperty;
using ::android::base::SetProperty;
using ::android::base::ParseUint;
using ::android::base::unique_fd;
using ::android::base::ReadFileToString;
using ::android::base::Trim;
using ::android::base::WriteStringToFile;
using ::android::hardware::google::pixel::usb::addAdb;
using ::android::hardware::google::pixel::usb::addEpollFd;
using ::android::hardware::google::pixel::usb::getVendorFunctions;
using ::android::hardware::google::pixel::usb::kDebug;
using ::android::hardware::google::pixel::usb::kDisconnectWaitUs;
using ::android::hardware::google::pixel::usb::linkFunction;
using ::android::hardware::google::pixel::usb::MonitorFfs;
using ::android::hardware::google::pixel::usb::resetGadget;
using ::android::hardware::google::pixel::usb::setVidPid;
using ::android::hardware::google::pixel::usb::unlinkFunctions;
using ::ndk::ScopedAStatus;
using ::std::shared_ptr;
using ::std::string;

constexpr char kGadgetName[] = "11210000.dwc3";
constexpr char kProcInterruptsPath[] = "/proc/interrupts";
constexpr char kProcIrqPath[] = "/proc/irq/";
constexpr char kSmpAffinityList[] = "/smp_affinity_list";
#ifndef UDC_PATH
#define UDC_PATH "/sys/class/udc/11210000.dwc3/"
#endif
static MonitorFfs monitorFfs(kGadgetName);

#define SPEED_PATH UDC_PATH "current_speed"

#define BIG_CORE "7"
#define MEDIUM_CORE "4"

#define POWER_SUPPLY_PATH	"/sys/class/power_supply/usb/"
#define USB_PORT0_PATH		"/sys/class/typec/port0/"

#define CURRENT_MAX_PATH			POWER_SUPPLY_PATH	"current_max"
#define CURRENT_USB_TYPE_PATH			POWER_SUPPLY_PATH	"usb_type"
#define CURRENT_USB_POWER_OPERATION_MODE_PATH	USB_PORT0_PATH		"power_operation_mode"

struct UsbGadget : public BnUsbGadget {
    UsbGadget();

    // Makes sure that only one request is processed at a time.
    std::mutex mLockSetCurrentFunction;
    std::string mGadgetIrqPath;
    long mCurrentUsbFunctions;
    bool mCurrentUsbFunctionsApplied;
    UsbSpeed mUsbSpeed;

    ScopedAStatus setCurrentUsbFunctions(long functions,
            const shared_ptr<IUsbGadgetCallback> &callback,
            int64_t timeout, int64_t in_transactionId) override;

    ScopedAStatus getCurrentUsbFunctions(const shared_ptr<IUsbGadgetCallback> &callback,
	    int64_t in_transactionId) override;

    ScopedAStatus reset(const shared_ptr<IUsbGadgetCallback> &callback,
	    int64_t in_transactionId) override;

    ScopedAStatus getUsbSpeed(const shared_ptr<IUsbGadgetCallback> &callback,
	    int64_t in_transactionId) override;

    ScopedAStatus setVidPid(const char *vid,const char *pid);

    std::string mI2cClientPath;

  private:
    Status tearDownGadget();
    Status getUsbGadgetIrqPath();
    Status setupFunctions(long functions, const shared_ptr<IUsbGadgetCallback> &callback,
            uint64_t timeout, int64_t in_transactionId);
};

}  // namespace gadget
}  // namespace usb
}  // namespace hardware
}  // namespace android
}  // aidl
