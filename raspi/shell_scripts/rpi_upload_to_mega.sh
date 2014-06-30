# Upload a new program the Arduino Mega (Ahab board) attached to the Raspberry
# PI

NEW_PROGRAM="$1"
#NEW_PROGRAM=/tmp/build7222941961883286658.tmp/skoonercode.cpp.hex

if [[ -z "$1" ]] ; then
    echo "Cannot upload nothing"
    exit;
fi

avrdude \
    -v \
    -c wiring \
    -patmega2560 \
    -cwiring \
    -P/dev/ttyACM0 \
    -b115200 \
    -D \
    -Uflash:w:${NEW_PROGRAM}:i

