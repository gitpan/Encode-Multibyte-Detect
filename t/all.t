#!/usr/bin/perl

use strict;
use warnings;

use Test::More qw(no_plan);

BEGIN {
    use_ok('Encode::Multibyte::Detect');
}

use Encode::Multibyte::Detect qw(:all);

no warnings 'utf8';

use utf8;

my @strings = (
    {text => "проверка"                                         , enc => 'utf-8', valid => 1, strict => 1},
    # bad unicode char
    {text => "пров\x{d800}ерка"                                 , enc => 'utf-8', valid => 1, strict => 0},
);

no utf8;

push @strings, (
    {text => "проверка"                                         , enc => 'utf-8', valid => 1, strict => 1},
    # unfinished sequence
    {text => "проверк\x{d0}"                                    , enc => 'utf-8', valid => 0, strict => 0},
    # broken sequence
    {text => "проверк\x{d0}0"                                   , enc => 'utf-8', valid => 0, strict => 0},
    # bad unicode char
    {text => "проверка\x{fb}\x{bf}\x{bf}\x{bf}\x{bf}"           , enc => 'utf-8', valid => 1, strict => 0},

    {text => "\x{ac}\x{64}\x{a6}\x{72}\x{b2}\x{c5}"             , enc => 'big-5', valid => 1},
    # bad sequence
    {text => "\x{ac}\x{84}\x{a6}\x{72}\x{b2}\x{c5}"             , enc => 'big-5', valid => 0},
);

for my $string (@strings) {
    if ($string->{enc} eq 'utf-8') {
        if ($string->{strict}) {
            ok(detect($string->{text}) eq 'utf-8');
            ok(detect($string->{text}, strict => 1) eq 'utf-8');
            ok(is_valid_utf8($string->{text}));
            ok(is_strict_utf8($string->{text}));
        }
        elsif ($string->{valid}) {
            ok(detect($string->{text}) eq 'utf-8');
            ok(detect($string->{text}, strict => 1) eq '');
            ok(is_valid_utf8($string->{text}));
            ok(!is_strict_utf8($string->{text}));
        }
        else {
            ok(detect($string->{text}) eq '');
            ok(detect($string->{text}, strict => 1) eq '');
            ok(!is_valid_utf8($string->{text}));
            ok(!is_strict_utf8($string->{text}));
        }

        ok(!is_valid_big5($string->{text}));
    }
    elsif ($string->{enc} eq 'big-5') {
        if ($string->{valid}) {
            ok(detect($string->{text}) eq 'big-5');
            ok(is_valid_big5($string->{text}));
        }
        else {
            ok(detect($string->{text}) eq '');
            ok(!is_valid_big5($string->{text}));
        }

        ok(!is_valid_utf8($string->{text}));
    }
}

1;
