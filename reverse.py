import os,io,sys
dn =lambda x, n: format(x, 'b').zfill(n)
alt = 0xe5
print("**** not flipped")
print(dn(alt,8))
alt = 0x2c
print("**** flipped")
print(dn(alt,8))
alt = 0x3c
print("**** flipped")
print(dn(alt,8))

