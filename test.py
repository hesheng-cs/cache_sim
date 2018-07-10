#!/usr/bin/python
from cache import Cache

def run():
    addresses = ['0000', '0004', '000C', '2200', '0008','00D0', '00E0', '1130', '0028',
                 '113C', '2204', '0010', '0020', '0004', '0040', '2208', '0008',
                 '00A0', '0004', '1104', '0028', '000C', '0084', '000C', '3390',
                 '00B0', '1100', '0028', '0064', '0070', '00D0', '0008', '3394']

    q1 = Cache(128, 1, 16)
    q1.calculate_hits(addresses)

    q2 = Cache(128, 4, 16)
    q2.calculate_hits(addresses)

    q3 = Cache(128, 4, 16)
    q3.calculate_hits(addresses)

    q4 = Cache(128, 8, 16)
    q4.calculate_hits(addresses)
    
if __name__ == '__main__':
    run()
