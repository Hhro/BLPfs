# Bell-LaPadula file system (BLPfs)

We adopted Bell-LaPadula model into a filesystem, 
and we think this is super safe. Isn't it?

## Usage

* `print([arg])`: prints argument(integer or string) to output. 
```
> print("abcd\n")
abcd
> print(1)
1> 
```
* `createfile([filename], [size], [level])`: creates file with given filesize and file level. 
```
> createfile("ABCD", 255, 1)
```

* `write([filename], [buf], [pos])`: writes buf into file from given position. 
```
> write("ABCD", "print(\"hello world\")", 0)
```

* `size([filename])`: gets filesize
```
> print(size("ABCD"))
256>
```

* `read([filename], [pos], [len])`: reads file from given position.
```
> print(read("ABCD", 0, 6))
print(>
```

* `execute([filename])`: executes script file, and gets output of the script as return value
```
> print(execute("ABCD"))
hello world>
```

* `remove([filename])`: removes file
```
> remove("ABCD")
```
