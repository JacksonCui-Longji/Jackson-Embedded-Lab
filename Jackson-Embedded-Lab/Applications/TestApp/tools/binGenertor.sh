# dd    from to block counter
dd if=/dev/urandom of=./test.bin bs=1024 count=100

echo "crc32 with python is : "
python3 -c "
import zlib
with open('./test.bin','rb') as f:
    data = f.read()
print(hex(zlib.crc32(data)))
"