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
  Nan::Callback *handler;
  uv_async_t async;
  HANDLE dwChangeHandles[2];
  void startThread();
  void stopThread();
  void threadMethod();
public:
  std::thread thd;
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

  void EmitChange(int cmd);

  static NAN_METHOD(New) {
    if (info.IsConstructCall()) {
      wstring dir;
      dir = V8Utils::v8StrToWStr(info[0]->ToString());
      Nan::Callback *callback = new Nan::Callback(info[1].As<v8::Function>());

      DirectoryWatcher *obj = new DirectoryWatcher(dir, callback);
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
  DirectoryWatcher(wstring dir, Nan::Callback *handler);
  ~DirectoryWatcher();
};  

NODE_MODULE(DW, DirectoryWatcher::Init);
