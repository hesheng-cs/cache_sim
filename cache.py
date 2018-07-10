#!/usr/bin/python
import math

replace_policy = ('LRU', 'FIFO') 

class Cache(object):
    
    def __init__(self, cache_size, ways, block_size, rp="LRU"):
        self.block_size = block_size
        self.ways = ways
        self.sets = cache_size/(block_size * ways)
        self.address_bits = 64
        self.offset_bits = int(math.log(block_size, 2))
        self.index_bits = int(math.log(self.sets, 2))
        self.tag_bits = self.address_bits - (self.offset_bits + self.index_bits)
        self.cache_array = [[] for x in range(self.sets)]
        if rp in replace_policy:
          self.rp_policy = rp
        else:
          print "Unsuppoted replace policy:", rp
          raise Exception
        print '==========================================='        
        print 'Total cache size: %d bytes.' % cache_size
        print 'Block size: %d bytes.' % block_size
        print '%d lines per set.' % ways
        print 'Number of blocks: %d' % (cache_size/block_size)
        print 'Number of sets: %d' % self.sets
        print 'Number of of tag bits: %d' % self.tag_bits
        print 'Number of index bits: %d' % self.index_bits
        print 'Number of bits for byte offset: %d' % self.offset_bits
        print 'Replace policy:', self.rp_policy 

    def calculate_hits(self, address_list):
        hits = 0
        misses = 0
        print('Address\t Hit/Miss')

        for address in address_list:
            addr = int(address, 16)
            set_num = (addr >> self.offset_bits) % self.sets
            tag = addr >> (self.offset_bits + self.index_bits)
            # If there is a hit, make the current tag the most recent
            if tag in self.cache_array[set_num]:
                result = "Hit"
                hits += 1
                if self.rp_policy == 'LRU':
                  self.lru_update(set_num, tag)
            # If there is a Miss, insert current tag and adjust LRU
            else:
                result = "Miss"
                misses += 1
                if self.rp_policy in ('LRU', 'FIFO'):
                    self.insert_line(set_num, tag)

            print(address , result)
            print self.cache_array
        print('Hits:', hits, 'Misses:', misses)

    # Adjusts the LRU in the set if there was a hit
    def lru_update(self, set_num, tag):
        if self.cache_array[set_num][-1] != tag:
            self.cache_array[set_num].remove(tag)
            self.cache_array[set_num].append(tag)
        #idx = self.cache_array[set_num].index(tag)

        #while idx < self.ways-1:
        #    self.cache_array[set_num][idx] = self.cache_array[set_num][idx+1]
        #    idx += 1
        #self.cache_array[set_num][-1] = tag

    # Inserts line and adjusts LRU. Leftmost element in cache_array is the LRU.
    # Compatible with FIFO as leftmost element in cache_array is the First In.
    def insert_line(self, set_num, tag):
        # If set is full, insert tag at the end of the array and shift all
        # elements to the left, getting rid of very left element
        if len(self.cache_array[set_num]) == self.ways:
            # If direct-mapped, just insert into the set, don't Remove & Append
            if self.ways == 1:
                self.cache_array[set_num][0] = tag
            else:
                self.cache_array[set_num].remove(self.cache_array[set_num][0])
                self.cache_array[set_num].append(tag)
                #for i in range(self.ways-1):
                #    self.cache_array[set_num][i] = self.cache_array[set_num][i+1]
                #self.cache_array[set_num][self.ways-1] = tag
            # Insert at first empty space
        else:
            self.cache_array[set_num].append(tag)
