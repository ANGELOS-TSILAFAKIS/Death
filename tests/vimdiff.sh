
#!/bin/sh

make debug
cp /bin/pwd /tmp/test/pwd1
cp /bin/pwd /tmp/test/pwd2
./war
xxd /tmp/test/pwd1 > pwd1.x
xxd /tmp/test/pwd2 > pwd2.x
vimdiff pwd1.x pwd2.x

