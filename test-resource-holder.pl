#!/usr/bin/perl
# -*- mode: cperl; cperl-indent-level: 4 -*-
# $ test-resource-holder.pl $
#
# Author: Tomi Ollila -- too ät iki piste fi
#
#	Copyright (c) 2016 Tomi Ollila
#	    All rights reserved
#
# Created: Fri 01 Jan 2016 20:46:11 EET too
# Last modified: Sat 02 Jan 2016 20:59:08 +0200 too

use 5.8.1;
use strict;
use warnings;

use Socket qw/AF_UNIX SOCK_STREAM/;

$ENV{'PATH'} = '/sbin:/usr/sbin:/bin:/usr/bin';

if (! -x 'test-resource-holder' or
    -M 'test-resource-holder' > -M 'resource-holder.c') {
    system qw/sh resource-holder.c/;
}

# note: close-on-exec set on C & P
socketpair C, P, AF_UNIX, SOCK_STREAM, 0;

$SIG{CHLD} = sub { wait; print "exit $?\n"; exit 0; };


if (fork == 0) {
    open STDIN, '>&', C;
    open STDOUT, '>&', C;
    exec
      #qw/gdb -ex run/,
      './test-resource-holder';
    die 'not reached'
}
close C;

sub sr($$@)
{
    my $line = shift;
    syswrite P, $_[0] . "\n";
    shift;
    alarm 5;
    foreach (@_) {
	$line++;
	my $l = <P> || 'eof';
	print $l;
	chomp $l;
	die "mismatch at $line: expected '$_' got '$l'\n\n"
	  unless $_ eq $l;
    }
    read P, $_, 4;
    unless ($_ eq '>>> ') {
	my $m = <P>;
	chomp $m;
	die "mismatch at $line: expected '>>> ' got '$_$m'\n\n";
    }
    alarm 0;
    print "\n";
}
read P, $_, 4;
if ($_ eq 'GNU ') {
    while (<P>) {
	print $_;
	last if /Starting program:/;
    }
    read P, $_, 4;
}
print "\n"

;sr __LINE__, 'a 1  a 2 4  a 3 3  a 4 3  a a 1  r 2 2'
  ,'   1   6   2     122..3'
  ,'   2   6   0  l  33444a'

;sr __LINE__, 'a t'
  ,'   1   6   0     122333'
  ,'   2   5   0  l  444at'

;sr __LINE__, 'a e 4  a f 4  a g 4  a h 4 a i 3'
  ,'   1   6   0     122333'
  ,'   2   6   0     444ate'
  ,'   3   6   0     eeefff'
  ,'   4   6   0     fggggh'
  ,'   5   6   0  l  hhhiii'

;sr __LINE__, 'r g 1  r e 1  r 4 1'
  ,'   1   6   0     122333'
  ,'   2   6   1     44.ate'
  ,'   3   6   1     ee.fff'
  ,'   4   6   1     fggg.h'
  ,'   5   6   0  l  hhhiii'

;sr __LINE__, 'a m 3'
  ,'   1   6   0     122333'
  ,'   2   6   0     44atee'
  ,'   3   6   0     effffg'
  ,'   4   6   0     gghhhh'
  ,'   5   6   0  l  iiimmm'

;sr __LINE__, 'r h 3  r f 3  r e 2'
  ,'   1   6   0     122333'
  ,'   2   6   1     44ate.'
  ,'   3   6   4     .f...g'
  ,'   4   6   3     ggh...'
  ,'   5   6   0  l  iiimmm'

;sr __LINE__, 'a q 1'
  ,'   1   6   0     122333'
  ,'   2   6   0     44atef'
  ,'   3   6   0     ggghii'
  ,'   4   5   0  l  immmq'
  ,'   5   0   0     '

;sr __LINE__, 'a r 7'
  ,'   1   6   0     122333'
  ,'   2   6   0     44atef'
  ,'   3   6   0     ggghii'
  ,'   4   6   0     immmqr'
  ,'   5   6   0  l  rrrrrr'

;sr __LINE__, 'r i 9  r 4 4  r 2 1'
  ,'   1   6   3     12.3..'
  ,'   2   6   4     ..at..'
  ,'   3   6   6     ......'
  ,'   4   6   1     .mmmqr'
  ,'   5   6   0  l  rrrrrr'

;sr __LINE__, 'a x 1'
  ,'   1   6   0     123atm'
  ,'   2   6   0     mmqrrr'
  ,'   3   5   0  l  rrrrx'
  ,'   4   0   0     '


__END__
;syswrite STDOUT, "Enter 'q' to quit\n>>> ";

while (<STDIN>) {
    syswrite P, $_;
    select undef, undef, undef, 0.2;
    sysread P, $_, 512;
    syswrite STDOUT, $_;
}
