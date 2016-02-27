/* -*- c-file-style: "k&r"; c-basic-offset: 2; indent-tabs-mode: nil -*- */

#include <node.h>
#include <v8.h>
#include <launch.h>
#include <iostream>
#include <sys/event.h>
#include <fstream>

using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Boolean;
using v8::Object;
using v8::String;
using v8::Value;

namespace {

  launch_data_t checkinRequest, checkinResponse;

  void runningUnderLaunchd(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    Local<Boolean> underLaunchd = Boolean::New(isolate, launch_data_get_type(checkinResponse) != LAUNCH_DATA_ERRNO);
    args.GetReturnValue().Set(underLaunchd);
  }

  void Method(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    if (args.Length() != 1) {
      isolate->ThrowException(Exception::TypeError(
                                String::NewFromUtf8(isolate, "Wrong number of arguments")));
      return;
    }

    switch (launch_data_get_type(checkinResponse)) {
    case LAUNCH_DATA_ERRNO: {
      int error = launch_data_get_errno(checkinResponse);
      char message[128];
      snprintf(message, 128, "Launchd data error %d" , error);
      isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, message)));
      break;
    }

    case LAUNCH_DATA_DICTIONARY: {
      launch_data_t sockets_dict = launch_data_dict_lookup(checkinResponse, LAUNCH_JOBKEY_SOCKETS);
      if (sockets_dict != NULL) {
        if (launch_data_dict_get_count(sockets_dict) > 1) {
          isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Some sockets will be ignored!")));
        } else {

          char* handleName = strdup(std::string(*v8::String::Utf8Value(args[0]->ToString())).c_str());
          launch_data_t listening_fd_array = launch_data_dict_lookup(sockets_dict, handleName);
          free(handleName);

          if (listening_fd_array == NULL) {
            isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "No known sockets found to answer requests on!")));
          } else {
            launch_data_t this_listening_fd = launch_data_array_get_index(listening_fd_array, 0);
            int fd = launch_data_get_fd(this_listening_fd);
            Local<Number> port = Number::New(isolate, fd);
            args.GetReturnValue().Set(port);
          }
        }
        break;
      }
    }

    default: {
      isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Unknown Error")));
      break;
    }
    }

  }

  void init(Local<Object> exports) {
    checkinRequest = launch_data_new_string(LAUNCH_KEY_CHECKIN);
    checkinResponse = launch_msg(checkinRequest);

    NODE_SET_METHOD(exports, "runningUnderLaunchd", runningUnderLaunchd);
    NODE_SET_METHOD(exports, "getSocketFileDescriptorForName", Method);
  }

  NODE_MODULE(checkin, init)

}
