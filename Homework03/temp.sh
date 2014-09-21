if [ $# -lt 2 ]; then
	echo "usage: <i2c bus> <i2c address>"
	exit 0
fi
BUS=$1
ADDR=$2

NUM=$(i2cget -y $BUS $ADDR 0)

FAHR=$((NUM*9/5+32))
echo $((FAHR))
