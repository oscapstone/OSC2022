def req2size(val, a):
    if val < 16:
        print(val,": ", hex(32))
    else:
        print(val,": ",hex((val + 16 + (a - 1)) & ~(a-1)))

for i in range(0, 64):
    req2size(i, 16)

