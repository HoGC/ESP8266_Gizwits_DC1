#!/bin/bash

echo "gen_misc.sh version 20150511"
echo ""

boot=new
echo "boot mode: $boot"
app=2
echo "app:$app"
spi_speed=40
echo "spi speed: $spi_speed MHz"
spi_mode=QIO
echo "spi mode: $spi_mode"
spi_size_map=2
echo "spi_size_map:$spi_size_map"
make clean
make COMPILE=gcc BOOT=$boot APP=2 SPI_SPEED=$spi_speed SPI_MODE=$spi_mode SPI_SIZE_MAP=$spi_size_map gaver=$1
make clean
make COMPILE=gcc BOOT=$boot APP=1 SPI_SPEED=$spi_speed SPI_MODE=$spi_mode SPI_SIZE_MAP=$spi_size_map gaver=$1

echo $pStr"Finish!"
