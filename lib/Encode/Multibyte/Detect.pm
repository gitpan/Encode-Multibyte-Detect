package Encode::Multibyte::Detect;

use strict;
use warnings;

our $VERSION = '1.00';

use base 'Exporter';

our @EXPORT = ();
our @EXPORT_OK = qw(detect is_valid_utf8 is_strict_utf8 is_valid_big5);

our %EXPORT_TAGS = ( all => [@EXPORT_OK] );

require XSLoader;
XSLoader::load('Encode::Multibyte::Detect');

sub detect {
    my ($str, %opts) = @_;

    return 'utf-8' if is_strict_utf8($str);
    return 'big-5' if is_valid_big5($str);

    return '' if $opts{strict};

    return 'utf-8' if is_valid_utf8($str);

    return '';
}

1;

=head1 NAME

Encode::Multibyte::Detect - detect multibyte encoding

=head1 SYNOPSIS

    use Encode::Multibyte::Detect qw(:all);
    use Encode;

    $enc = detect($octets);
    if ($enc) {
        $string = decode($enc, $octets);
    }

    is_valid_utf8($octets);
    is_strict_utf8($octets);
    is_valid_big5($octets);

=head1 REPOSITORY

The source code for the Encode::Multibyte::Detect is held in a public git repository
on Github: L<https://github.com/Silencer2K/perl-enc-multibyte>

=head1 AUTHOR

Aleksandr Aleshin F<E<lt>silencer@cpan.orgE<gt>>

=head1 COPYRIGHT

This software is copyright (c) 2012 by Aleksandr Aleshin.

This is free software; you can redistribute it and/or modify it under
the same terms as the Perl 5 programming language system itself.

=cut
