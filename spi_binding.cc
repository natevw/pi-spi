#include <node.h>
#include <node_version.h>
#include <node_buffer.h>

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
    assert(args.Length() == 7);
    assert(args[0]->IsNumber());
    assert(args[1]->IsNumber());
    assert(args[2]->IsNumber());
    assert(args[3]->IsNumber());
    assert(node::Buffer::HasInstance(args[4]) || args[4]->IsNull());
    assert(args[5]->IsNumber());
    assert(args[6]->IsFunction());
    
    int fd = args[0]->ToInt32()->Value();
    uint32_t speed = args[1]->ToUint32()->Value();
    uint8_t mode = args[2]->ToUint32()->Value();
    uint8_t order = args[3]->ToUint32()->Value();
    printf("fd: %i, speed: %u, mode: %i, order: %i\n", fd, speed, mode, order);
    
    /*
    Local<Object> writebuf = args[4]->ToObject();
    node::Buffer::Data(writebuf);
    */
    
    // TODO: implement using SPI_IOC_MESSAGE
    // see https://raw.github.com/torvalds/linux/master/Documentation/spi/spidev_test.c
    // http://lxr.free-electrons.com/source/include/uapi/linux/spi/spidev.h#L53
    // https://www.kernel.org/doc/Documentation/spi/spidev
    
    
    size_t len = 0;
    (void)len;
    
    // TODO: this should be in uv_work_cb, guarded by a mutex
    /*
    int ret = 0;
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    ret = ioctl(fd, SPI_IOC_WR_LSB_FIRST, &order);
    struct spi_ioc_transfer msg = {
		.tx_buf = (uintptr_t)tx,
		.rx_buf = (uintptr_t)rx,
		.len = len,
		.speed_hz = speed,
	};
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &msg);
    */
    
    // TODO: this should be in uv_work_cb_after
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