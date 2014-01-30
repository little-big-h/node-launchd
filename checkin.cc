#include <node.h>
#include <v8.h>
#include <launch.h>
#include <iostream>
#include <sys/event.h>
#include <fstream>

using namespace v8;

Handle<Value> Method(const Arguments& args) {
	HandleScope scope;
	if (args.Length() != 1) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
	}
	char* handleName = strdup(std::string(*v8::String::Utf8Value(args[0]->ToString())).c_str());

	int result = -1;
	launch_data_t checkinRequest = launch_data_new_string(LAUNCH_KEY_CHECKIN);
	launch_data_t checkinResponse = launch_msg(checkinRequest);
    
	switch (launch_data_get_type(checkinResponse)) {
	case LAUNCH_DATA_ERRNO: {
		int error = launch_data_get_errno(checkinResponse);
		char message[128];
		snprintf(message, 128, "Launchd data error %d" , error);
		ThrowException(Exception::Error(String::New(message)));
		return scope.Close(Undefined());
		break;
	}

	case LAUNCH_DATA_DICTIONARY: {
		launch_data_t sockets_dict = launch_data_dict_lookup(checkinResponse, LAUNCH_JOBKEY_SOCKETS);
		if (NULL == sockets_dict) {
			ThrowException(Exception::Error(String::New("No sockets found to answer requests on!")));
			return scope.Close(Undefined());
		} else if (launch_data_dict_get_count(sockets_dict) > 1) {
			ThrowException(Exception::Error(String::New("Some sockets will be ignored!")));
			return scope.Close(Undefined());
		}

		launch_data_t listening_fd_array = launch_data_dict_lookup(sockets_dict, handleName);
		if (listening_fd_array == NULL) {
			ThrowException(Exception::Error(String::New("No known sockets found to answer requests on!")));
			return scope.Close(Undefined());
		}

		launch_data_t this_listening_fd = launch_data_array_get_index(listening_fd_array, 0);
		result = launch_data_get_fd(this_listening_fd);
		break;
	}
	default: {
		ThrowException(Exception::Error(String::New("Unknown Error")));
		return scope.Close(Undefined());
		break;
	}
	}
	free(handleName);
	return scope.Close(Integer::New(result));
}

void init(Handle<Object> exports) {
	exports->Set(String::NewSymbol("getSocketFileDescriptorForName"),
							 FunctionTemplate::New(Method)->GetFunction());
}

NODE_MODULE(checkin, init)
