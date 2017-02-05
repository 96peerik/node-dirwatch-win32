#include "DirectoryWatcher.h"
#include <windows.h>
#include <iostream>

using namespace v8;
using namespace std;

DirectoryWatcher::DirectoryWatcher(wstring dir, Nan::Callback *handler): handler(handler), dir(dir)
{
  startThread();
}

DirectoryWatcher::~DirectoryWatcher()
{
  handler = NULL;
}

void DirectoryWatcher::emitChange() {
  Nan::ObjectWrap *sender = (Nan::ObjectWrap*)this;
  Nan::HandleScope scope;
  v8::Local<v8::Value> argv[1] = {
    Nan::New("change").ToLocalChecked(),
  };
  Nan::MakeCallback(sender->handle(), "emit", 1, argv);
}

void DirectoryWatcher::emitLastError() {
  string message = V8Utils::GetErrorStdStr(lastError);
  Nan::ObjectWrap *sender = (Nan::ObjectWrap*)this;
  Nan::HandleScope scope;
  v8::Local<v8::Value> argv[2] = {
    Nan::New("error").ToLocalChecked(),
    Nan::Error(Nan::New<String>(message).ToLocalChecked())
  };
  Nan::MakeCallback(sender->handle(), "emit", 2, argv);
}

void DirectoryWatcher::threadMethod() {
  DWORD dwWaitStatus;
  dwChangeHandles[1] = CreateEventW(NULL, FALSE, FALSE, L"ExitEvent");
  dwChangeHandles[0] = FindFirstChangeNotificationW(
    dir.c_str(),
    TRUE,
    FILE_NOTIFY_CHANGE_FILE_NAME |
    FILE_NOTIFY_CHANGE_LAST_WRITE |
    FILE_NOTIFY_CHANGE_DIR_NAME |
    FILE_NOTIFY_CHANGE_LAST_WRITE); // watch file name changes

  if (dwChangeHandles[0] == INVALID_HANDLE_VALUE)
  {
    lastError = GetLastError();
    uv_async_send(&async);
    isActive = false;
    return;
  }

  bool _notified = false;
  while (isActive) {
    dwWaitStatus = WaitForMultipleObjects(2, dwChangeHandles, false, 10);
    if (dwWaitStatus == WAIT_OBJECT_0 + 0) {
      _notified = true;
      FindNextChangeNotification(dwChangeHandles[0]);
    }
    else if (dwWaitStatus == WAIT_OBJECT_0 + 1) {
      lastError = 0;
      FindCloseChangeNotification(dwChangeHandles[0]);
      return;
    }
    else if (dwWaitStatus == WAIT_TIMEOUT) {
      if (_notified) {
        _notified = false;
        lastError = 0;
        uv_async_send(&async);
        FindNextChangeNotification(dwChangeHandles[0]);
      }
    }
    else {
      isActive = false;
      lastError = GetLastError();
      uv_async_send(&async);
      FindCloseChangeNotification(dwChangeHandles[0]);
      return;
    }
  }
}

//called in main thread
void WorkAsyncComplete(uv_async_t *handle) {
  DirectoryWatcher *dw = (DirectoryWatcher*)handle->data;
  std::thread* thd = &dw->thd;
  
  if (dw->lastError) {
    dw->emitLastError();
  }
  
  if (dw->isActive) {
    dw->emitChange();
  }
  else {
    thd->join();
    handle->data = NULL;
    uv_close((uv_handle_t*)handle, NULL);
  }
}


void DirectoryWatcher::stopThread() {
  isActive = false;
  SetEvent(dwChangeHandles[1]);
}


void DirectoryWatcher::startThread() {
  if (async.data == this) return;
  isActive = true;
  uv_async_init(uv_default_loop(), &async, (uv_async_cb)WorkAsyncComplete);
  async.data = this;
  // uv_unref((uv_handle_t *)&work->async);
  thd = std::thread(&DirectoryWatcher::threadMethod, this);
}

