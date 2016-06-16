#include <node.h>
#include <nan.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

#ifdef WIN32      // this substitution should work since we're not using return value…
#define snprintf(a,b,...) _snprintf_s(a,b,_TRUNCATE,__VA_ARGS__)
#endif

#if __linux__
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#endif


using namespace v8;

uv_mutex_t spiAccess;

class SpiTransfer : public Nan::AsyncWorker {
  public:
      SpiTransfer(Nan::Callback *cb, int fd, uint32_t speed, uint8_t mode, uint8_t order, v8::Handle<v8::Value> _writebuf, size_t readcount) :
        Nan::AsyncWorker(cb), fd(fd), speed(speed), mode(mode), order(order), readcount(readcount)
      {
          size_t writelen;
          char* writedata;
          if (_writebuf->IsObject()) {
              Local<Object> writebuf = _writebuf->ToObject();
              writelen = node::Buffer::Length(writebuf);
              assert(writelen <= 0xffffffff /*std::numeric_limits<T>::max()*/);
              writedata = node::Buffer::Data(writebuf);
          } else {
              writelen = 0;
              writedata = NULL;
          }
          
          buflen = (readcount > writelen) ? readcount : writelen /* std::max(readcount,writelen) */;
          buffer = (uint8_t*)malloc(buflen);
          if (writelen) memcpy(buffer, writedata, writelen);
          if (readcount > writelen) memset(buffer+writelen, 0, readcount-writelen);
          
          //printf("fd: %i, speed: %u, mode: %i, order: %i\n", fd, speed, mode, order);
          //printf("writelen: %u, readcount: %u, buflen=%u\n", (uint32_t)writelen, readcount, buflen);
      }
      ~SpiTransfer() {}
      
      void Execute () {
          // see https://raw.github.com/torvalds/linux/master/Documentation/spi/spidev_test.c
          // http://lxr.free-electrons.com/source/include/uapi/linux/spi/spidev.h#L53
          // https://www.kernel.org/doc/Documentation/spi/spidev
          int ret = 0;
      #ifdef SPI_IOC_MESSAGE
          uv_mutex_lock(&spiAccess);
          ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
          if (ret != -1) {
              ret = ioctl(fd, SPI_IOC_WR_LSB_FIRST, &order);
              if (ret != -1) {
                  struct spi_ioc_transfer msg = {};
                  msg.tx_buf = (uintptr_t)buffer;
                  msg.rx_buf = (uintptr_t)buffer;
                  msg.len = buflen;
                  msg.speed_hz = speed;
                  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &msg);
              }
          }
          uv_mutex_unlock(&spiAccess);
      #else
        #ifdef __GNUC__
          #warning "Building without SPI support"
        #else
          #pragma message("Building without SPI support")
        #endif
          (void)fd;
          (void)speed;
          (void)mode;
          (void)order;
          (void)readcount;
          (void)buflen;
          (void)buffer;
          ret = -1;
          errno = ENOSYS;
      #endif
          err = (ret == -1) ? errno : 0;
    }
      
    void HandleOKCallback () {
        Nan::HandleScope();
        
        Local<Value> e;
        if (err) {
            char msg[1024];
            snprintf(msg, sizeof(msg), "SPI error: %s (errno %i)", strerror(err), err);
            e = Nan::Error(msg);
        } else {
            e = Nan::Null();
        }
        
        Local<Value> d;
        if (!err && readcount) {
            d = Nan::NewBuffer((char*)buffer, readcount).ToLocalChecked();
        } else {
            d = Nan::Null();
        }
        
        Nan::TryCatch try_catch;
        if (0 && err) {
            Local<Value> v[] = {e};
            callback->Call(1, v);
        } else {
            Local<Value> v[] = {e,d};
            callback->Call(2, v);
        }
        
        if (try_catch.HasCaught()) {
            Nan::FatalException(try_catch);
        }
    };
    
  private:
    int fd;
    uint32_t speed;
    uint8_t mode;
    uint8_t order;
    uint32_t readcount;
    size_t buflen;
    uint8_t* buffer;
    
    int err;
};

NAN_METHOD(Transfer) {
    // (fd, speed, mode, order, writebuf, readcount, cb)
    assert(info.Length() == 7);
    assert(info[0]->IsNumber());
    assert(info[1]->IsNumber());
    assert(info[2]->IsNumber());
    assert(info[3]->IsNumber());
    assert(info[4]->IsNull() || node::Buffer::HasInstance(info[4]));
    assert(info[5]->IsNumber());
    assert(info[6]->IsFunction());
    
    int fd = info[0]->Int32Value();
    uint32_t speed = info[1]->Uint32Value();
    uint8_t mode = info[2]->Uint32Value();
    uint8_t order = info[3]->Uint32Value();
    v8::Handle<v8::Value> writebuf = info[4];
    size_t readcount = info[5]->Uint32Value();
    Nan::Callback* cb = new Nan::Callback(info[6].As<Function>());
    
    Nan::AsyncQueueWorker(new SpiTransfer(cb, fd, speed, mode, order, writebuf, readcount));
}

NAN_MODULE_INIT(InitAll) {
    uv_mutex_init(&spiAccess);      // no matching `uv_mutex_destroy` but there's no module deinit…
    NAN_EXPORT(target, Transfer);
}

NODE_MODULE(spi_binding, InitAll)
