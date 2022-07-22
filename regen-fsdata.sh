#!/bin/sh

if [ ! -f makefsdata ]; then
    # Doing this outside cmake as we don't want it cross-compiled but for host
    echo Compiling makefsdata
    gcc -o makefsdata -Ipico-sdk/lib/lwip/src/include -Ipico-sdk/lib/lwip/contrib/ports/unix/port/include -I. -DMAKEFS_SUPPORT_DEFLATE=1 -DMAKEFS_SUPPORT_DEFLATE_ZLIB=1 pico-sdk/lib/lwip/src/apps/http/makefsdata/makefsdata.c -lz
fi

echo Regenerating fsdata.c
./makefsdata -defl:9 -svr:picow
echo Done
