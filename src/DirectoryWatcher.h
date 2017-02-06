#pragma once
#include <nan.h>
#include <v8.h>
#include "V8Utils.h"
#include <thread>

#define _WIN32_FUSION 0x0100

using namespace v8;
using namespace std;

static string *moduleFilename;

class DirectoryWatcher : public Nan::ObjectWrap {
private:
  ISupportErrorInfo * supportErrorInfo = NULL;
  wstring dir;
  uv_async_t *async;
  void startThread();
  void stopThread();
  void threadMethod();
  static void OnClose(uv_handle_t *handle);
public:
  std::thread *thd;
  DWORD lastError;
  bool isActive;
  void emitChange();
  void emitLastError();

  static void Init(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target, Handle<Object> module){

    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("DirectoryWatcher").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("DirectoryWatcher").ToLocalChecked(),
      Nan::GetFunction(tpl).ToLocalChecked());
  }

  static inline DirectoryWatcher* Unwrap(Nan::NAN_METHOD_ARGS_TYPE info) {
    return Nan::ObjectWrap::Unwrap<DirectoryWatcher>(info.This());
  }

  static NAN_METHOD(New) {
    if (info.IsConstructCall()) {
      wstring dir;
      dir = V8Utils::v8StrToWStr(info[0]->ToString());

      DirectoryWatcher *obj = new DirectoryWatcher(dir);
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    }
    else {
      const int argc = 1;
      v8::Local<v8::Value> argv[argc] = { info[0] };
      v8::Local<v8::Function> cons = Nan::New(constructor());
      info.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
  }

  static inline Nan::Persistent<v8::Function> & constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }

public:
  DirectoryWatcher(wstring dir);
  ~DirectoryWatcher();
};  

NODE_MODULE(DW, DirectoryWatcher::Init);
