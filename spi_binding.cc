#include <node.h>
#include <node_version.h>
#include <node_buffer.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

#if __linux__
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#endif


using namespace v8;

uv_mutex_t spiAccess;


// HT http://kkaefer.github.io/node-cpp-modules/#calling-async
struct Baton {
    uv_work_t request;
    Persistent<Function> callback;
    int err;
    
    int fd;
    uint32_t speed;
    uint8_t mode;
    uint8_t order;
    uint32_t readcount;
    uint32_t buflen;
    uint8_t buffer[0];      // allocated larger
};

void _Transfer(uv_work_t* req) {
    Baton* baton = static_cast<Baton*>(req->data);
    
    // see https://raw.github.com/torvalds/linux/master/Documentation/spi/spidev_test.c
    // http://lxr.free-electrons.com/source/include/uapi/linux/spi/spidev.h#L53
    // https://www.kernel.org/doc/Documentation/spi/spidev
    int ret = 0;
#ifdef SPI_IOC_MESSAGE
    uv_mutex_lock(&spiAccess);
    ret = ioctl(baton->fd, SPI_IOC_WR_MODE, &baton->mode);
    if (ret != -1) {
        ret = ioctl(baton->fd, SPI_IOC_WR_LSB_FIRST, &baton->order);
        if (ret != -1) {
            struct spi_ioc_transfer msg = {
                /*.tx_buf = */ (uintptr_t)baton->buffer,
                /*.rx_buf = */ (uintptr_t)baton->buffer,
                /*.len = */ baton->buflen,
                /*.speed_hz = */ baton->speed,
                
                // avoid "missing initializer" warnings…
                /*.delay_usecs = */ 0,
                /*.bits_per_word = */ 0,
                /*.cs_change = */ 0,
                /*.pad = */ 0,
            };
            ret = ioctl(baton->fd, SPI_IOC_MESSAGE(1), &msg);
        }
    }
    uv_mutex_unlock(&spiAccess);
#else
#warning "Building without SPI support"
    ret = -1;
    errno = ENOSYS;
#endif
    baton->err = (ret == -1) ? errno : 0;
}

void Finish_Transfer(uv_work_t* req, int status) {
    (void)status;           // AFAICT, only used w/uv_cancel
    HandleScope scope;
    Baton* baton = static_cast<Baton*>(req->data);
    
    Local<Value> e;
    if (baton->err) {
        char* msg;
        asprintf(&msg, "SPI error: %s (errno %i)", strerror(baton->err), baton->err);
        e = Exception::Error(String::New(msg));
        free(msg);
    } else {
        e = Local<Value>::New(Null());
    }
    
    Local<Value> d;
    if (!baton->err && baton->readcount) {
        // via http://deadhorse.me/nodejs/2012/10/10/c_addon_in_nodejs_buffer.html
        // and http://www.samcday.com.au/blog/2011/03/03/creating-a-proper-buffer-in-a-node-c-addon/
        node::Buffer* b = node::Buffer::New((char*)baton->buffer, baton->readcount);        // in node 0.10.x this is a SlowBuffer
        Local<Object> globalObj = Context::GetCurrent()->Global();
        Local<Function> bufferConstructor = Local<Function>::Cast(globalObj->Get(String::New("Buffer")));       // …which we wrap.
        // TODO: it looks like handle_ might go away soon.
        // c.f. https://groups.google.com/d/msg/nodejs/GmHqobrM_TA/SaaP3oHVFCoJ for lead on this and other 0.11.x changes
        Handle<Value> v[] = {b->handle_, Integer::New(baton->readcount), Integer::New(0)};
        d = bufferConstructor->NewInstance(3, v);
    } else {
        d = Local<Value>::New(Null());
    }
    
    TryCatch try_catch;
    if (0 && baton->err) {
        Local<Value> v[] = {e};
        baton->callback->Call(Context::GetCurrent()->Global(), 1, v);
    } else {
        Local<Value> v[] = {e,d};
        baton->callback->Call(Context::GetCurrent()->Global(), 2, v);
    }
    baton->callback.Dispose();
    delete[] baton;
    
    if (try_catch.HasCaught()) {
        node::FatalException(try_catch);
    }
}



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
    baton->request.data = baton;
    baton->callback = Persistent<Function>::New(Local<Function>::Cast(args[6]));
    baton->fd = args[0]->ToInt32()->Value();
    baton->speed = args[1]->ToUint32()->Value();
    baton->mode = args[2]->ToUint32()->Value();
    baton->order = args[3]->ToUint32()->Value();
    baton->readcount = readcount;
    baton->buflen = buflen;
    if (writelen) memcpy(baton->buffer, writedata, writelen);
    if (readcount > writelen) memset(baton->buffer+writelen, 0, readcount-writelen);
    //printf("fd: %i, speed: %u, mode: %i, order: %i\n", baton->fd, baton->speed, baton->mode, baton->order);
    //printf("writelen: %u, readcount: %u, buflen=%u\n", (uint32_t)writelen, readcount, buflen);
    
    uv_queue_work(uv_default_loop(), &baton->request, _Transfer, Finish_Transfer);
    
    return scope.Close(Undefined(isolate));
}

void init(Handle<Object> exports) {
    uv_mutex_init(&spiAccess);      // no matching `uv_mutex_destroy` but there's no module deinit…
    exports->Set(
        String::NewSymbol("transfer"),
        FunctionTemplate::New(Transfer)->GetFunction()
    );
}

NODE_MODULE(spi_binding, init)
