import re, struct, collections, sys
sys.path.insert(0,'/home/tim/gits/comfortzone_heatpump/tools')
from frames import load
def build(fn):
    """memory image per bank from the last reply of every register: {bank: {addr: byte}}"""
    img=collections.defaultdict(dict)
    for cmd,reg,pl in load(fn):
        if cmd!='r': continue
        bank,lo,hi=[int(x,16) for x in reg.split()]
        addr=lo|(hi<<8)
        for i,b in enumerate(pl): img[bank][addr+i]=b
    return img
if __name__=='__main__':
    img=build(sys.argv[1])
    for bank in sorted(img):
        a=sorted(img[bank]); print(f"bank {bank}: {len(a)} bytes, {a[0]:#06x}..{a[-1]:#06x}")
