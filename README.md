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