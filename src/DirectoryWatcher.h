#pragma once
#include <nan.h>
#include <v8.h>
#include "EventWorker.h"
#include "V8Utils.h"

#define _WIN32_FUSION 0x0100

using namespace v8;
using namespace std;

static string *moduleFilename;

class DirectoryWatcher : public Nan::ObjectWrap
{
private:
  ISupportErrorInfo * supportErrorInfo = NULL;
public:
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

  static void EmitChange(void* owner, int cmd);

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
