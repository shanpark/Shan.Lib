# Shan.Lib
C++ Class Library for C++11

### Shan.JSON
 Header only library for Parsing & Generating JSON.
 - Easy & Simple way to handle JSON.
 - Easy I/O from/to stream or string.
 - MessagePack encoding support.
 - STL Compatible object(map), array(vector), string.

    [Shan.JSON Manual](http://progtrend.blogspot.kr/p/shanjson.html)

<pre><code>#include "json/json.h"
...

shan::json::object json("{\"key\":\"value\", \"encoded\":true}"); // parsing & constructing object.

cout << json << endl; // {"key":"value","encoded":true}
cout << json["encoded"]->bool_val() << endl; // true

std::string jt = json.str(); // generating JSON text from object.

// For more information, please refer to the following URL
//  URL: http://progtrend.blogspot.kr/p/shanjson.html
</code></pre>

### Shan.Net
 Header only Network Framework.
 - An asynchronous event-driven network framework.
 - TCP, SSL, UDP support.

 Dependancy
 - Asio 1.10.8 (standalone)
 - openssl 1.1.0e

 <pre><code>#include "net/net.h"

// *** Please refer to the test sources...

// channel event handler class
class serv_ch_handler : public tcp_channel_handler {
public:
  virtual void channel_connected(tcp_channel_context_base* ctx) override { // a client connected
    cout << "serv_ch_handler::channel_connected(" << ctx->channel_id() << ") called" << endl;

    auto data = std::make_shared<unix_time>(3000);
    ctx->write(data);
  }

  virtual void channel_read(tcp_channel_context_base* ctx, shan::object_ptr& data) override { // some data has read from client.
    cout << "serv_ch_handler::channel_read() called" << endl;
  }

  virtual void channel_disconnected(tcp_channel_context_base* ctx) override { // a client has been disconnected.
    cout << "serv_ch_handler::channel_disconnected() called:" << ++c << endl;
  }

  virtual void exception_caught(tcp_channel_context_base* ctx, const std::exception& e) override { // an exception occurred...
    cout << "serv_ch_handler::exception_caught() - " << e.what() << endl;
  }

  ...
};


...

// in main()
shan::net::tcp_server serv;
serv.add_channel_handler(new channel_coder());   // when serv is released
serv.add_channel_handler(new serv_ch_handler()); // handlers will be released too.
serv.start(10999); // worker thread start

serv.wait_stop(); // wait
</code></pre>
