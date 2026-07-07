#!/usr/bin/perl
#
# 雑誌I/O形式のダンプリストをモトローラSフォーマットに変換する
#	チェックサムは行末に ;XX
#   チェックサムの無い行はチェックしないで素通し
#
use strict;
use warnings;

while (my $line = <STDIN>) {
    chomp($line);
    next if $line =~ /^#/;
    
    next unless $line =~ /^([0-9A-Fa-f]{4})[:\s]\s*([0-9A-Fa-f]{2}(?:\s+[0-9A-Fa-f]{2})*)(?:\s*;\s*([0-9A-Fa-f]{2}))?/;

    my $addr = $1;
    my $data = $2;
    my $orig_cs_str = $3;

    my @bytes = map { hex($_) } split(/\s+/, $data);

    if (defined $orig_cs_str) {
        my $orig_cs = hex($orig_cs_str);
        my $sum = 0;
        map { $sum += $_ } @bytes;

        if (($sum & 0xFF) != $orig_cs) {
            warn "Checksum mismatch at $addr\n";
            next;
        }
    }

    my $count = scalar(@bytes) + 3;
    my $s1_sum = $count + hex(substr($addr, 0, 2)) + hex(substr($addr, 2, 2));
    map { $s1_sum += $_ } @bytes;
    my $s1_cs = (0xFF - ($s1_sum & 0xFF)) & 0xFF;

    printf("S1%02X%s%s%02X\n", $count, uc($addr), uc(join('', split(/\s+/, $data))), $s1_cs);
}

print "S9030000FC\n";

exit 0;
