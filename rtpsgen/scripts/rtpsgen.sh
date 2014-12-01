#!/bin/sh

dir="`dirname \"$0\"`"

java_exec=java

java -version &>/dev/null

if [ $? != 0 ]; then
    [ -z "$JAVA_HOME" ] && { echo "Java binary cannot be found. Please, make sure its location is in the PATH environment variable or set JAVA_HOME environment variable."; exit 1; }
    java_exec="${JAVA_HOME}/bin/java"
fi

exec $java_exec -jar "$dir/fastrtpsgen.jar" "$@"

