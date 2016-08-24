# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := /mnt/share/zeng/zevent_c/

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += /mnt/share/zeng/zbaselib/include

LOCAL_MODULE    := zevent
LOCAL_SRC_FILES := zevent_accepter.c	\
	zevent.c	\
	zevent_connector.c	\
	zevent_event.c	\
	zevent_looper.c	\
	zevent_pipe.c	\
	zevent_reactor_select.c	\
	zevent_timer.c

LOCAL_LDLIBS := -lzbase -L/mnt/share/zeng/zbaselib/libs/armeabi/

include $(BUILD_SHARED_LIBRARY)
