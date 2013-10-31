#include <node.h>
#include <node_version.h>
//#include <node_buffer.h>

using namespace v8;

Handle<Value> Transfer(const Arguments& args) {
#if !!NODE_VERSION_AT_LEAST(0, 11, 0)
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
#else
    HandleScope scope;
#endif
    // (fd, speed, mode, order, writebuf, readcount, cb)
    return scope.Close(String::New("Hello, World."));
}

void init(Handle<Object> exports) {
    exports->Set(
        String::NewSymbol("transfer"),
        FunctionTemplate::New(Transfer)->GetFunction()
    );
}

NODE_MODULE(spi_binding, init)