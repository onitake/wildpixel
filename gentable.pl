#!/usr/bin/perl

# Fast & tiny integer math library for AVR µCs table generator script
# Copyright © 2017 Gregor Riepl <onitake@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
#     Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#    
#     Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

use strict;
use warnings;
use IO::File;
use Getopt::Long;
use Math::Trig;

my ($ofile, $var, $start, $end) = ('tiny_table.h', 'SIN_TABLE', 0.0, pi / 2.0);
sub usage($) {
	print("Usage: gentable [-o <output_file>]\n");
	print("-o  Names the output header file (default: $ofile)\n");
	print("-h  This help\n");
	exit(1) if ($_[0]);
}
GetOptions(
	"output=s" => \$ofile,
	"help" => sub { usage(1); },
) or usage(1);

print("Writing to $ofile\n");
my $out = IO::File->new($ofile, 'w') || die("Can't open $ofile");

for my $steps (8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384) {
	my $scale = $steps - 1;
	print($out "#define ${var}_${steps} {");
	for (my $i = 0; $i < $steps; $i++) {
		my $a = sin($i / $steps * pi / 2);
		printf($out "%.0f,", ($a * $scale));
	}
	print($out "}\n");
}

$out->close();
