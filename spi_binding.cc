#include <node.h>
#include <node_version.h>
//#include <node_buffer.h>

using namespace v8;

Handle<Value> Transfer(const Arguments& args) {
#if NODE_VERSION_AT_LEAST(0, 11, 0)
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
#else
#define isolate
    HandleScope scope;
#endif
    
    // (fd, speed, mode, order, writebuf, readcount, cb)
    
    // TODO: implement using SPI_IOC_MESSAGE
    // see https://raw.github.com/torvalds/linux/master/Documentation/spi/spidev_test.c
    
    Local<Function> cb = Local<Function>::Cast(args[6]);
    const unsigned argc = 1;
    Local<Value> argv[argc] = { Local<Value>::New(Exception::Error(String::New("Not implemented"))) };
    cb->Call(Context::GetCurrent()->Global(), argc, argv);
    
    return scope.Close(Undefined(isolate));
}

void init(Handle<Object> exports) {
    exports->Set(
        String::NewSymbol("transfer"),
        FunctionTemplate::New(Transfer)->GetFunction()
    );
}

NODE_MODULE(spi_binding, init)