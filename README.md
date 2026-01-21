# SimbaFc
ESP32 Flight controller

Init Sequ ---- low level

02 7e b6 //soft reset

01 02 00 //Error register

02 7e 11  //acc power up

02 7e 15  //gyr pwr up

02 7e 19 //mag pwr up

01 03 00 // pmu stat 0x15 means all normal mode

01 40 00 // acc config
01 41 00 // acc range
01 42 00 // gyr config
01 43 00 // gyr range
01 44 00 // mag config // 0x0B for 800 read per sec see page 64 to know how t configure the magnetometer

02 4c 00 // this sets mag 