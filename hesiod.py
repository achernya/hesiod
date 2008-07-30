#!/usr/bin/env python

"""
Present both functional and object-oriented interfaces for executing
lookups in Hesiod, Project Athena's service name resolution protocol.
"""

from _hesiod import bind, resolve

from pwd import struct_passwd
from grp import struct_group

class HesiodParseError(Exception):
    pass

class Lookup(object):
    """
    A Generic Hesiod lookup
    """
    def __init__(self, hes_name, hes_type):
        self.results = resolve(hes_name, hes_type)
        self.parseRecords()
    
    def parseRecords(self):
        pass

class FilsysLookup(Lookup):
    def __init__(self, name):
        Lookup.__init__(self, name, 'filsys')
    
    def parseRecords(self):
        Lookup.parseRecords(self)
        
        self.filsys = []
        self.multiRecords = (len(self.results) > 1)
        
        for result in self.results:
            priority = 0
            if self.multiRecords:
                result, priority = result.rsplit(" ", 1)
                priority = int(priority)
            
            parts = result.split(" ")
            type = parts[0]
            if type == 'AFS':
                self.filsys.append(dict(type=type,
                                        location=parts[1],
                                        mode=parts[2],
                                        mountpoint=parts[3],
                                        priority=priority))
            elif type == 'NFS':
                self.filsys.append(dict(type=type,
                                        remote_location=parts[1],
                                        server=parts[2],
                                        mode=parts[3],
                                        mountpoint=parts[4],
                                        priority=priority))
            elif type == 'ERR':
                self.filsys.append(dict(type=type,
                                        message=parts[1],
                                        priority=priority))
            elif type == 'UFS':
                self.filsys.append(dict(type=type,
                                        device=parts[1],
                                        mode=parts[2],
                                        mountpoint=parts[3],
                                        priority=priority))
            elif type == 'LOC':
                self.filsys.append(dict(type=type,
                                        location=parts[1],
                                        mode=parts[2],
                                        mountpoint=parts[3],
                                        priority=priority))
            else:
                raise HesiodParseError('Unknown filsys type: %s' % type)

class PasswdLookup(Lookup):
    def __init__(self, name):
        Lookup.__init__(self, name, 'passwd')
    
    def parseRecords(self):
        self.passwd = struct_passwd(self.results[0].split(':'))

class UidLookup(PasswdLookup):
    def __init__(self, uid):
        Lookup.__init__(self, uid, 'uid')

class GroupLookup(Lookup):
    def __init__(self, group):
        Lookup.__init__(self, group, 'group')
    
    def parseRecords(self):
        group_info = self.results[0].split(':')
        group_info[3] = group_info[3].split(',') if group_info[3] != '' else []
        
        self.group = struct_group(group_info)

class GidLookup(GroupLookup):
    def __init__(self, gid):
        Lookup.__init__(self, gid, 'gid')

__all__ = ['bind', 'resolve',
           'Lookup', 'FilsysLookup', 'PasswdLookup', 'UidLookup',
           'GroupLookup', 'GidLookup',
           'HesiodParseError']
