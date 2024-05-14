CXXFILES='s1m.cxx sapi.cxx'
CXXFLAGS=''
clang++ ${CXXFLAGS} ${CXXFILES} -mmacosx-version-min=11.1 -std=c++11 -I . -I /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include -L /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib -lcurl `/usr/local/fltk4/bin/fltk-config --cxxflags` `/usr/local/fltk4/bin/fltk-config --ldflags` -o sg_arm64
arch -x86_64 clang++ -mmacosx-version-min=10.9 -std=c++11 ${CXXFLAGS} ${CXXFILES} -I . -I /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include -L /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib -lcurl `/usr/local/fltk_x64/bin/fltk-config --cxxflags` `/usr/local/fltk_x64/bin/fltk-config --ldflags` -o sg_x64
lipo sg_x64 sg_arm64 -create -output SeaGroup.app/Contents/MacOS/SeaGroup
rm sg_x64 sg_arm64
clang++ ${CXXFLAGS} ${CXXFILES} -mmacosx-version-min=10.9 -std=c++11 -I . -I /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include -L /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib -lcurl `/usr/local/fltk_x64/bin/fltk-config --cxxflags --ldflags`
x86_64-w64-mingw32-windres -O coff sg.rc -o sg.res
x86_64-w64-mingw32-g++ -O2 ${CXXFLAGS} ${CXXFILES} sg.res -I . `/usr/local/fltk_win64/bin/fltk-config --cxxflags` `/usr/local/fltk_win64/bin/fltk-config --ldflags` -o SeaGroup -static -DCURL_STATICLIB -I /usr/local/curl_win64p/include  /usr/local/curl_win64p/lib/libcurl.a /usr/local/openssl_win64/lib/libssl.a /usr/local/openssl_win64/lib/libcrypto.a -ladvapi32 -lcrypt32 -lgdi32 -lwldap32 -lws2_32 -lgdiplus