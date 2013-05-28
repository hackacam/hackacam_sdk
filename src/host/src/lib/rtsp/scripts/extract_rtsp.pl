#!/usr/bin/perl
use warnings;
use strict;

# Extract RTSP messages from a wireshark export file

my $filename = $ARGV[0] || die "Usage: extract_rtsp.pl <pcap_file.txt> > result.txt\n";
open my $file, '<', $filename or die "Cannot open $filename\n";
my $outp = '';
my $prev = '';
my $rtsp;
my $pkt_line_num = 0;
my $line_num = 0;
while (my $line = <$file>) {
    $pkt_line_num = 0 if $line =~ m{^No\.}; 
    $rtsp = $line =~ m{RTSP} if $pkt_line_num == 1;
    if ($rtsp && $line =~ m{^([\da-f]{4})\s}) {
        $line_num = hex($1);
        next if $line_num < 0x30;
        my @line = split ' ', substr($line, 6, 3*16);
        @line = @line[6 .. $#line] if $line_num == 0x30;
        for my $byte (@line) {
            my $char = chr(hex($byte));
            $char = "\\r" if $char eq "\r";
            $char = "\\n" if $char eq "\n";
            $outp .= $char;
            if ($char eq '\n' && $prev eq '\r') {
                print $outp, "\n";
                $outp = '';
            }
            $prev = $char;
        }
    } elsif ($rtsp && $line_num && $line =~m{^\s*$}) {
        print "\n";
        $line_num = 0;
    }
    $pkt_line_num++;
}
print $outp, "\n" if $outp ne '';
