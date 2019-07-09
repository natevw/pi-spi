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


uv_mutex_t spiAccess;

class SpiTransfer : public Nan::AsyncWorker {
  public:
      SpiTransfer(Nan::Callback *cb, int fd, uint32_t speed, uint8_t mode, uint8_t order, v8::Local<v8::Value> _writebuf, size_t readcount) :
        Nan::AsyncWorker(cb, "pi-spi:transfer"), fd(fd), speed(speed), mode(mode), order(order), readcount(readcount)
      {
          size_t writelen;
          char* writedata;
          if (_writebuf->IsObject()) {
              v8::Local<v8::Object> writebuf =  Nan::To<v8::Object>(_writebuf).ToLocalChecked();
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
      ~SpiTransfer() {
        if (buffer) free(buffer);
      }
      
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
        
        v8::Local<v8::Value> e;
        if (err) {
            char msg[1024];
            snprintf(msg, sizeof(msg), "SPI error: %s (errno %i)", strerror(err), err);
            e = Nan::Error(msg);
        } else {
            e = Nan::Null();
        }
        
        v8::Local<v8::Value> d;
        if (!err && readcount) {
            d = Nan::NewBuffer((char*)buffer, readcount).ToLocalChecked();
            buffer = NULL;    // `d` has now taken ownership of the memory
        } else {
            d = Nan::Null();
        }
        
        Nan::TryCatch try_catch;
        if (0 && err) {     // NOTE: since `d` is always initialized, we can always use `else` branch…
            v8::Local<v8::Value> v[] = {e};
            callback->Call(1, v, async_resource);
        } else {
            v8::Local<v8::Value> v[] = {e,d};
            callback->Call(2, v, async_resource);
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
    
    int fd = Nan::To<int32_t>(info[0]).FromJust();
    uint32_t speed = Nan::To<uint32_t>(info[1]).FromJust();
    uint8_t mode = Nan::To<uint32_t>(info[2]).FromJust();
    uint8_t order = Nan::To<uint32_t>(info[3]).FromJust();
    v8::Local<v8::Value> writebuf = info[4];
    size_t readcount = Nan::To<uint32_t>(info[5]).FromJust();
    Nan::Callback* cb = new Nan::Callback(info[6].As<v8::Function>());
    
    Nan::AsyncQueueWorker(new SpiTransfer(cb, fd, speed, mode, order, writebuf, readcount));
}

NAN_MODULE_INIT(InitAll) {
    uv_mutex_init(&spiAccess);      // no matching `uv_mutex_destroy` but there's no module deinit…
    NAN_EXPORT(target, Transfer);
}

NODE_MODULE(spi_binding, InitAll)
