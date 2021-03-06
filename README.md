# KHealthCertificate

Parsing of digital vaccination, test and recovery certificates.

## Supported Formats

The following certificate formats can be parsed:

* Digital Infrastructure for Vaccination Open Credentialing (DIVOC), the system used for Indian vaccination certificates (see https://divoc.egov.org.in/).
* EU "Digital Green Certificate" (DCG) vaccination, test and recovery certificates (see https://github.com/eu-digital-green-certificates).
* SMART Heath Cards (SHC) vaccination certificates (see https://smarthealth.cards/), in use some areas of North America.

Formats with limited support:
* Danish domestic certificates. Those are largely similar to EU DGC certificates but can contain more than one vaccination/recovery element.
So far only the first of those elements is extracted.
* ICAO VDS-NC. Used for international travel e.g. in Australia and Japan. Certificates can be read, but only the most recent vaccination
element is extracted. Signatures are not verified yet.
* NL COVID-19 CoronaCheck, the Dutch domestic system (see https://coronacheck.nl/nl/). Certificates are read, but the short-lived digital
codes offered by the official apps cannot be produced, limiting this to the use of the long-valid paper form and thus degrading privacy
compared to the official app.

## Use Case

Warning: This is merely meant to check what is in your own trusted certificates.
While this attempts to validate the signatures of all supported certificates, there is no guarantee this is
implemented correctly.
