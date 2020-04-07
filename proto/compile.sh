#!/bin/bash
if [ -z $1 ]; then
    echo "USAGE: $0 <proto_name>"
else
    protoc --cpp_out=. $1
    mv *.h ../include/paxoslib/proto/
    mv *.cc ../src/proto
fi
