#include <node.h>
#include <node_version.h>
#include <node_buffer.h>

using namespace v8;


// HT http://kkaefer.github.io/node-cpp-modules/#calling-async
struct Baton {
    uv_work_t request;
    Persistent<Function> callback;
    int result;
    int errno;
    
    int fd;
    uint32_t speed;
    uint8_t mode;
    uint8_t order;
    uint32_t buflen;
    uint8_t buffer[0];      // allocated larger
};



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
    assert(args[4]->IsNull() || node::Buffer::HasInstance(args[4]));
    assert(args[5]->IsNumber());
    assert(args[6]->IsFunction());
    
    uint32_t readcount = args[5]->ToUint32()->Value();
    
    size_t writelen;
    char* writedata;
    if (args[4]->IsObject()) {
        Local<Object> writebuf = args[4]->ToObject();
        writelen = node::Buffer::Length(writebuf);
        assert(writelen <= 0xffffffff /*std::numeric_limits<T>::max()*/);
        writedata = node::Buffer::Data(writebuf);
    } else {
        writelen = 0;
        writedata = NULL;
    }
    
    uint32_t buflen = (readcount > writelen) ? readcount : writelen /* std::max(readcount,writelen) */;
    
    Baton* baton = (Baton*)new uint8_t[sizeof(Baton)+buflen];
    baton->fd = args[0]->ToInt32()->Value();
    baton->speed = args[1]->ToUint32()->Value();
    baton->mode = args[2]->ToUint32()->Value();
    baton->order = args[3]->ToUint32()->Value();
    baton->buflen = buflen;
    if (writelen) memcpy(baton->buffer, writedata, writelen);
    printf("fd: %i, speed: %u, mode: %i, order: %i\n", baton->fd, baton->speed, baton->mode, baton->order);
    printf("writelen: %lu, readcount: %u, buflen=%u\n", writelen, readcount, buflen);
    
    // TODO: this should be in uv_work_cb, guarded by a mutex
    // see https://raw.github.com/torvalds/linux/master/Documentation/spi/spidev_test.c
    // http://lxr.free-electrons.com/source/include/uapi/linux/spi/spidev.h#L53
    // https://www.kernel.org/doc/Documentation/spi/spidev
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