import fcntl
import struct
import os

CMD = {
    'read': 0,
    'write': 1,
    'remove': 2,
    'size': 3,
    'execute': 4,
    'createfile': 5
}

def check_fd_open(fd: int) -> bool:
    return fcntl.fcntl(fd, fcntl.F_GETFD) != -1

def read(fd: int, filename: bytearray, pos: int, length: int) -> bytearray:
    if len(filename) > 8:
        return bytearray()
    filename = filename.ljust(8, b'\x00')
    msg = bytearray()
    msg.append(CMD['read'])
    msg.extend(filename)
    msg.extend(struct.pack('<II', pos, length))

    if os.write(fd, msg) != len(msg):
        raise Exception('Something is wrong')
    
    length = struct.unpack('<I', os.read(fd, 4))[0]
    return bytearray(os.read(fd, length))

def write(fd: int, filename: bytearray, pos: int, data: bytearray) -> int:
    if len(filename) > 8:
        return 0
    filename = filename.ljust(8, b'\x00')

    if pos + len(data) >= 0x100:
        return 0
    
    msg = bytearray()
    msg.append(CMD['write'])
    msg.extend(filename)
    msg.extend(struct.pack('<II', pos, len(data)))
    msg.extend(data)

    if os.write(fd, msg) != len(msg):
        raise Exception('Something is wrong')

    return 0

def remove(fd: int, filename: bytearray) -> int:
    if len(filename) > 8:
        return 0
    filename = filename.ljust(8, b'\x00')
    
    msg = bytearray()
    msg.append(CMD['remove'])
    msg.extend(filename)

    if os.write(fd, msg) != len(msg):
        raise Exception('Something is wrong')

    return 0


def size(fd: int, filename: bytearray) -> int:
    if len(filename) > 8:
        return 0
    filename = filename.ljust(8, b'\x00')
    
    msg = bytearray()
    msg.append(CMD['size'])
    msg.extend(filename)

    if os.write(fd, msg) != len(msg):
        raise Exception('Something is wrong')

    return struct.unpack('<I', os.read(fd, 4))[0]


def execute(fd: int, filename: bytearray) -> bytearray:
    if len(filename) > 8:
        return 0
    filename = filename.ljust(8, b'\x00')
    
    msg = bytearray()
    msg.append(CMD['execute'])
    msg.extend(filename)

    if os.write(fd, msg) != len(msg):
        raise Exception('Something is wrong')

    length = struct.unpack('<I', os.read(fd, 4))[0]
    return bytearray(os.read(fd, length))

def createfile(fd: int, filename: bytearray, size: int, flevel: int) -> int:
    if len(filename) > 8:
        return 0
    filename = filename.ljust(8, b'\x00')
    if not (0 <= size and size <= 0x100):
        return 0
    if not (0 <= flevel and flevel <= 3):
        return 0
    
    msg = bytearray()
    msg.append(CMD['createfile'])
    msg.extend(filename)
    msg.extend(struct.pack('<II', size, flevel))

    if os.write(fd, msg) != len(msg):
        raise Exception('Something is wrong')

    return 0