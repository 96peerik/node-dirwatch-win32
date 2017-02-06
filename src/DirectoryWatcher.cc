#include "DirectoryWatcher.h"
#include <windows.h>

using namespace v8;
using namespace std;

DirectoryWatcher::DirectoryWatcher(wstring dir): dir(dir)
{
  startThread();
}

DirectoryWatcher::~DirectoryWatcher()
{
  stopThread();
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
  HANDLE filehandle = FindFirstChangeNotificationW(
    dir.c_str(),
    TRUE,
    FILE_NOTIFY_CHANGE_FILE_NAME |
    FILE_NOTIFY_CHANGE_LAST_WRITE |
    FILE_NOTIFY_CHANGE_DIR_NAME |
    FILE_NOTIFY_CHANGE_LAST_WRITE); // watch file name changes

  if (filehandle == INVALID_HANDLE_VALUE)
  {
    lastError = GetLastError();
    uv_async_send(async);
    return;
  }

  bool _notified = false;

  while (isActive) {
    dwWaitStatus = WaitForSingleObject(filehandle, 100);
    if (!isActive) { 
      FindCloseChangeNotification(filehandle);
      return;
    }
    if (dwWaitStatus == WAIT_OBJECT_0) {
      _notified = true;
      FindNextChangeNotification(filehandle);
    } else if (dwWaitStatus == WAIT_TIMEOUT) {
      if (_notified) {
        _notified = false;
        lastError = 0;
        if (async->data)
          uv_async_send(async);
      }
    }
    else {
      lastError = GetLastError();
      if (async->data)
        uv_async_send(async);
    }
  }
}

//called in main thread
void WorkAsyncComplete(uv_async_t *handle) {
  if (handle->data == NULL) {
    return;
  }
  DirectoryWatcher *dw = (DirectoryWatcher*)handle->data;
  if (dw->lastError) {
    dw->emitLastError();
  } else {
    dw->emitChange();
  }
}

void DirectoryWatcher::stopThread() {
  async->data = NULL;
  uv_close((uv_handle_t *)async, OnClose);

  isActive = false;
  if (thd->joinable())
    thd->join();

  delete thd;
  thd = NULL;
}

void DirectoryWatcher::OnClose(uv_handle_t *handle) {
  delete handle;
}

void DirectoryWatcher::startThread() {
  async = new uv_async_t();
  uv_async_init(uv_default_loop(), async, (uv_async_cb)WorkAsyncComplete);
  async->data = this;
  thd = new std::thread(&DirectoryWatcher::threadMethod, this);
}

