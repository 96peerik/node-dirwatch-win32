#include <uv.h>
#include <thread>
#include <windows.h>
#include "V8Utils.h"

struct EventWorkerWork {
  uv_async_t  async;
  bool isActive;
  wstring dir;
  DWORD err;
  void* owner;
  std::thread thd;
};

namespace EventWorker {
  HANDLE dwChangeHandles[2];
  static void Run(wstring dir, void* owner);

  static void close_cb(uv_handle_t* handle) {
    free(handle);
  };

  //called in main thread
  void WorkAsyncComplete(uv_async_t *handle) {
    std::thread* thd;
    EventWorkerWork *work = static_cast<EventWorkerWork *>(handle->data);
    if (work->err) {
      V8Utils::EmitError(work->owner, V8Utils::GetErrorStdStr(work->err));
      work->err = 0;
    }
    if (work->isActive) {
      V8Utils::EmitChange(work->owner);
    }
    else {
      thd = &work->thd;
      thd->join();
      uv_close((uv_handle_t*)handle, close_cb);
    }
  }

  static void runThread(EventWorkerWork* work) {
    DWORD dwWaitStatus;
    dwChangeHandles[1] = CreateEventW(NULL, FALSE, FALSE, L"ExitEvent");
    dwChangeHandles[0] = FindFirstChangeNotificationW(
      work->dir.c_str(),
      TRUE,
      FILE_NOTIFY_CHANGE_FILE_NAME |
      FILE_NOTIFY_CHANGE_LAST_WRITE |
      FILE_NOTIFY_CHANGE_DIR_NAME |
      FILE_NOTIFY_CHANGE_LAST_WRITE); // watch file name changes

    if (dwChangeHandles[0] == INVALID_HANDLE_VALUE)
    {
      work->err = GetLastError();
      work->isActive = false;
      uv_async_send(&work->async);
    }

    bool _notified = false;
    while (work->isActive) {
      dwWaitStatus = WaitForMultipleObjects(2, dwChangeHandles, false, 10);
      if (dwWaitStatus == WAIT_OBJECT_0 + 0) {
        _notified = true;
        FindNextChangeNotification(dwChangeHandles[0]);
      }
      else if (dwWaitStatus == WAIT_OBJECT_0 + 1) {
        FindCloseChangeNotification(dwChangeHandles[0]);
        return;
      }
      else if (dwWaitStatus == WAIT_TIMEOUT) {
        if (_notified) {
          _notified = false;
          uv_async_send(&work->async);
          FindNextChangeNotification(dwChangeHandles[0]);
        }
      }
      else {
        work->isActive = false;
        work->err = GetLastError();
        uv_async_send(&work->async);
        FindCloseChangeNotification(dwChangeHandles[0]);
        return;
      }
    }
  }

  static void Stop() {
    SetEvent(dwChangeHandles[1]);
  }


  static void Run(wstring dir, void* owner) {
    EventWorkerWork* work = new EventWorkerWork();
    work->isActive = true;
    work->dir = dir;
    work->async.data = work;
    work->owner = owner;
    work->err = NULL;
    
    uv_async_init(uv_default_loop(), &work->async, (uv_async_cb)WorkAsyncComplete);
   // uv_unref((uv_handle_t *)&work->async);
    work->thd = std::thread(runThread, work);
  }
};

