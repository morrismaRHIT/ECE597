if [ $# -lt 4 ]; then
	echo "usage: <i2c bus> <i2c address> <low temp> <high temp>"
	exit 0
fi

I2CBUS=$1
ADDR=$2
LOW=$3
HIGH=$4

i2cset -y $I2CBUS $ADDR 2 $LOW

i2cset -y $I2CBUS $ADDR 3 $HIGH
