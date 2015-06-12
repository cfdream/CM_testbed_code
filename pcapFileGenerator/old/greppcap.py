#!/bin/env python

'''
This tool finds data in a pcap using a user-specified regular expression. It then writes all matching session packets to an individual pcap file.

While similar to ngrep, it is not meant to be a replacement. Unlike ngrep, exported pcap data can be used for session replaying and to analyze the context of a session before and after a matched packet. 

Requires: scapy (http://www.secdev.org/projects/scapy/)

"I am rarely happier than when spending an entire day programming my computer to perform automatically a task that it would otherwise take me a good ten seconds to do by hand."
 - D.N. Adams ("Last Chance to See").

'''

import getopt
import logging
import re
from scapy.all import conf
from scapy.all import PcapReader
from scapy.all import wrpcap
import sys

__author__ = 'Jason Avery <jason.avery@gmail.com>'
__date__ = '2012-07-12'
__version__ = '1.1.0'
__license__ = {'type': 'GNU GPL v3', 'owner': 'Jason Avery', 'year': '2012', 'url': 'http://www.gnu.org/copyleft/gpl.html'}
__source__ = 'http://code.google.com/p/py-greppcap/'


def usage():
  '''usage() - I shouldn't have to explain this one.'''
  print('''
Usage: greppcap.py [options] "search regex" pcapfiles
Options:           [-h|--help]:  Usage help
                   [-v|--negated]:  Negated match
                   [-p|--ports]:  Restricts matches to comma delimited port list
                   [-l|--log]:  Specify a file to log to
                   [-Q|--quiet]:  Only reports errors
                   [-D|--debug]:  Reports everything
                   [-V|--version]:  This script's version

This script searches the payload data of an IP packet in a libpcap file with a given regular expression and writes the matched stream to a new pcap file with a similar filename.
''')
  return


class GrepPcap():
  '''GrepPcap(loglevel=logging.ERROR, logfile=None)

Easiest way to call this is like:

import greppcap
gp = greppcap.GrepPcap()
matches = gp.MatchPcap('google','google.pcap')
matches = gp.WritePcap(matches, 'google.pcap')

For logging, use the "logging" module's settings.  Ex:

gp = greppcap.GrepPcap(loglevel=logging.INFO, logfile='google-matches.txt')

'''

  def __init__(self, loglevel=logging.ERROR, logfile=None):
    # Set up event reporting
    logging.basicConfig(filename=logfile, format='[%(asctime)s %(levelname)s] %(message)s', datefmt='%H:%M:%S', level=loglevel)
    self.debug = logging.debug
    self.warn = logging.warn
    self.error = logging.error
    self.info = logging.info


  def MatchPcap(self, regex, file, negated=False, ports=[]):
    '''MatchPcap(regex, file, negated=False, ports=[])

Matches the given regex with the given pcap file. Returns a matchMap dictionary of sessions that matched.

regex:  Any ol' valid regex will do.
file:  A libpcap file.
negated:  If true, finds sessions that do NOT match the regex.
ports:  A list of ports to restrict the matching to.
'''
  
    try:
      regex = re.compile(r'(?s)%s' % regex)
    except:
      self.error('Invalid regular expression: %s' % regex)
      raise Exception('Invalid regular expression.')
  
    try:
      # scapy.utils.PcapReader
      self.debug('Reading: %s' % file)
      pcap = PcapReader(file)
    except:
      # yes, logging.exception should be used here, but it doesn't add any value
      self.error('Not a valid pcap file: %s' % file)
      raise
  
    # matchMap format:
    # {3: {'proto': 6, 'host1': ('1.2.3.4',1024), 'host2': ('9.8.7.6',80)}}
    matchMap = {}
    newid = 1
  
    # Build a list of streams that match the search regex
    while pcap:
      try:
        packet = pcap.read_packet()
        match = {}
        matchedStream = False
  
        # Skip if the session's ports aren't in the allowed port list (-p).
        try:
          if ports and not (packet[2].sport in ports or packet[2].dport in ports):
            continue
        except AttributeError:
          # Continue; weren't any ports at all (ip.proto not in (6,17))
          pass
        except IndexError:
          # Wasn't even IP, skip it
          pass
  
        # Perform match
        try:
          rawpacket = packet[3].build()
          # for some reason, re.match doesn't work, yet re.findall does.
          if regex.findall(rawpacket):
            matchedStream = True
            #most verbose: self.debug('matched\n%s' % str(rawpacket))
  
          if matchedStream or negated:
            # Run the list backwards in hope of matching early rather than matching at the end of the entire list.
            ids = matchMap.keys()
            ids.reverse()
            unknownStream = True
            for id in ids:
              try:
            # Assuming we'll never see a packet with same src and dst
            # TCP,UDP layers referred to by index offset for code simplicity
            # would do this as one if statement, but seperating it helps exit early and save cpu cycles
                if (packet['IP'].src,packet[2].sport) in (matchMap[id]['host1'],matchMap[id]['host2']):
                  if (packet['IP'].dst,packet[2].dport) in (matchMap[id]['host1'],matchMap[id]['host2']):
                    unknownStream = False
            # This avoids source port reuse problems, causing session collisions
            # unknownStream is True if its a known session yet tcp syn flag is set.
                    if packet['IP'].proto == 6 and packet['TCP'].sprintf('%flags%') == 'S':
                      unknownStream = True
                    break
              except AttributeError:
                # most likely the session isn't tcp/udp so scapy throws AttributeError if no sport/dport exists.  Try without it instead.
                if matchMap[id]['proto'] == packet['IP'].proto:
                  if packet['IP'].src in (matchMap[id]['host1'], matchMap[id]['host2']): 
                    if packet['IP'].dst in (matchMap[id]['host1'], matchMap[id]['host2']):
                      unknownStream = False
                      break
  
          # if its not negated and its a newly matched stream, OR negated and an unknown, add it to matchMap.  if its negated and matched later, it gets deleted before the end
          if (matchedStream and unknownStream and not negated) or (negated and unknownStream and not matchedStream):
            matchMap[newid] = {}
            # Personal preference of mine:  printing matches here rather than when the function finishes gives the user a feeling things are happening, rather than get the messages all at once at the end of the call.
            # This is doubly so when dealing with massive 1g+ pcap files
            try:
              matchMap[newid] = {'proto': packet['IP'].proto, 'host1': (packet['IP'].src,packet[2].sport), 'host2': (packet['IP'].dst,packet[2].dport)}
              self.info('Match #%d: Proto %d, IPs %s:%d, %s:%d' % (newid,matchMap[newid]['proto'],matchMap[newid]['host1'][0],matchMap[newid]['host1'][1],matchMap[newid]['host2'][0],matchMap[newid]['host2'][1]))
            except AttributeError:
              matchMap[newid] = {'proto': packet['IP'].proto, 'host1': packet['IP'].src, 'host2': packet['IP'].dst} 
              self.info('Match #%d: Proto %d, IPs %s, %s' % (newid,matchMap[newid]['proto'],matchMap[newid]['host1'],matchMap[newid]['host2']))
            newid += 1
          elif matchedStream and negated and not unknownStream:
            # Flag the session as matching regex to NOT keep.
            # If deleted now, it would just come back from the next related packet
            matchMap[id]['delete'] = True

        except IndexError:
          pass # no raw layer, nothing to search
      except TypeError:
        break
  
    if negated:
      for id in matchMap.keys():
        try:
          if matchMap[id]['delete']:
            del matchMap[id]
            self.info('Match #%d matched, removed from result.' % id)
        except KeyError:
          pass
      # rebuilding the sequential id's here might get confusing with the prior-printed messages.  probably best to avoid it.

    pcap.close()
    del pcap
    return matchMap
  
  
  def WritePcap(self, matchMap, file, outputFilename=None):
    '''WritePcap(matchMap, file, outputFilename=None

Writes the matched pcap sessions in matchMap found in file to separate pcap files.

matchMap:  The output from MatchPcap.
file:  The pcap file you matched on previously.
outputFilename:  Allows you to specify the prefix on the output pcaps.

'''
    try:
      if not matchMap.keys():
        self.debug('matchMap is empty! No matches from greppcap?')
        raise
    except:
      self.debug('Not a valid matchMap.')
      raise
  
    try:
      pcap = PcapReader(file)
      if not outputFilename:
        # There's probably some python fu way to do this.  I have the regex fu.
        try:
          filename = re.findall(r'^(?is)[./]?(?:[^/]+/)*([^/]+)(?:\.[^\./]+)$', file)[0]
        except:
          # base filename was too crazy to figure out, go with a default one
          filename = 'greppcap'
      else:
        filename = outputFilename
    except:
      self.error('Not a valid pcap file: %s' % file)
      raise
  
    self.debug('matchMap: %s' % matchMap)
    self.debug('Writing pcaps...')
  
    # Open file handle on a pcap and append the packet to the right pcap.
    while pcap:
      try:
        packet = pcap.read_packet()
        writePacket = False
        for id in matchMap.keys(): 
          try:
            if (packet['IP'].src,packet[2].sport) in (matchMap[id]['host1'],matchMap[id]['host2']):
              if (packet['IP'].dst,packet[2].dport) in (matchMap[id]['host1'],matchMap[id]['host2']):
                writePacket = True
          except AttributeError:
            if matchMap[id]['proto'] == packet['IP'].proto:
              if packet['IP'].src in (matchMap[id]['host1'], matchMap[id]['host2']):
                if packet['IP'].dst in (matchMap[id]['host1'], matchMap[id]['host2']):
                  writePacket = True
          except IndexError:
            continue # not IP
          if writePacket:
            # Create/append the packet to a pcap file and close the handler.
            # Doing it this way avoids hitting any open file handler limit (resource.getrlimit(resource.RLIMIT_NOFILE))
            try:
              wrpcap('%s_match%d.pcap' % (filename,id),packet,append=True,sync=True)
            except IOError as e:
              self.error('OS limitation prevented completion of %s_match%d.pcap.  Error: %s' % (filename,id,e))
            break
      except TypeError:
        break # end of pcap
  
    # Now nicely announce the completion of pcaps.
    for id in matchMap.keys():
      matchMap[id]['pcap'] = '%s_match%d.pcap' % (filename,id)
      try:
        self.info('Wrote IP proto %d %s:%d <> %s:%d into %s' % (matchMap[id]['proto'],matchMap[id]['host1'][0],matchMap[id]['host1'][1],matchMap[id]['host2'][0],matchMap[id]['host2'][1],matchMap[id]['pcap']))
      except:
        self.info('Wrote IP proto %d %s <> %s into %s' % (matchMap[id]['proto'],matchMap[id]['host1'],matchMap[id]['host2'],matchMap[id]['pcap']))
  
    return matchMap


def main():
  '''main() - Handles command line arguments and runs the pcap searching functions.  Also, a small puppy appears somewhere off the coast of Norway and promptly drowns.  You monster.'''

  if not sys.argv[1:]:
    usage()
    sys.exit(0)

  try:
    optswitch, optlist = getopt.getopt(sys.argv[1:], 'Dhl:p:QVv', ['debug','help','log=','negated','ports=','quiet', 'version'])
  except getopt.GetoptError:
    usage()
    sys.exit(2)

  # Default log verbosity
  loglevel = logging.INFO
  logfile = None

  # Match these first
  for opt in optswitch:
    if opt[0] in ('-h', '--help'):
      usage()
      sys.exit(0)
    elif opt[0] in ('-V', '--version'):
      print('This is greppcap.py version %s (%s).' % (__version__, __date__))
      sys.exit(0)
    elif opt[0] in ('-D', '--version'):
      loglevel = logging.DEBUG
    elif opt[0] in ('-Q', '--quiet'):
      loglevel = logging.ERROR

  if len(optlist) < 2:
    usage()
    sys.exit(0)

  negated = False
  ports = []

  for opt in optswitch:
    if opt[0] in ('-v', '--negated'):
      negated = True
    elif opt[0] in ('-p', '--ports'):
      portslist = re.split(',', str(opt[1]))
      # convert values to int type
      for port in portslist:
        ports.append(int(port))
      del portslist
    elif opt[0] in ('-l', '--log'):
      logfile = opt[1]

  matchRegex = optlist[0]
  inputFiles = optlist[1:]

  gp = GrepPcap(logfile=logfile, loglevel=loglevel)

  for file in inputFiles:
    gp.info('Starting: %s' % file)
    try:
      matchMap = gp.MatchPcap(matchRegex, file, negated=negated, ports=ports)
      matchMap = gp.WritePcap(matchMap, file)
    except:
      gp.warn('No results from: %s' % file)
    gp.info('Finished: %s' % file)

  return


if __name__ == '__main__': main()
