## Overview
This is a very basic custom-built encryption algorithm written in C++ and designed to be used via the command line. It works by iterating over every byte in a given input file and changing the positions of the bits within each byte to hide its original value. The order in which bits are swapped is determined by a user-defined encryption key (a string of text), making it possible to decrypt the encrypted text provided that the original encryption key is known.

**Disclaimer: This program was created out of personal interest and therefore isn't fit for actually encrypting sensitive data. I cannot be held responsible for any damage caused by using this program. The encryption is probably very easy to break, as discussed below.**

## Usage
```
bitshift [-dh] <key> <input-file> <output-file>
```
Use the ```-h``` option for help.

## Illustration
Assume that we want to encrypt the simple string ```good``` using the encryption key ```mysecret```. The program begins by seeding the random number generator of C++ using the hashcode of the encryption key. The message itself is treated as a series of bytes based on their ASCII values:
```
g (103)  o (111)  o (111)  d (100)
01100111 01101111 01101111 01100100
```
We then visit each byte in turn and scramble the order of bits within each one. The location each bit is moved to is calculated pseudorandomly, which means we will always get the same results each time we run the program provided we keep using the original encryption key (```mysecret``` in this example). This causes the transformation shown below - notice how each byte still has the same number of zeroes and ones as it did originally.
```
Plaintext:  01100111 01101111 01101111 01100100
Cyphertext: 10011110 01111101 11101101 00010110
```
The resulting cyphertext does not resemble the original message. It can be decrypted again by using the same key that was originally used to encrypt it.

## Integrity
Because this algorithm works by moving bits around within each byte, it is most effective at encrypting bytes that have a roughly equal number of high bits (1s) and low bits (0s). The table below illustrates how many unique combinations of bytes there are for varying numbers of high bits:

```
r nCr
-----
0 1
1 8
2 28
3 56
4 70
5 56
6 28
7 8
8 1
```
where n is the number of bits in one byte (8), r is the number of high bits (bits set to 1) and ```nCr = n! / ((n-r)! * r!)``` (the total number of unique combinations).

For example, there are 8 ways in which we can define a byte with 1 high bit, as indicated by the table above:
````
10000000
01000000
00100000
00010000
00001000
00000100
00000010
00000001
```
In the same way, there are 28 ways to define a byte with 2 high bits, 56 ways to define one with 3 high bits and so on. The table also shows that bytes containing only zeroes (```00000000```) or only ones (```11111111```) can only be organised in one way, as common sense would suggest.

This means that bytes with exactly 4 high bits are the most secure because they can be scrambled into one of 69 other combinations when encrypted with this algorithm. Meanwhile, bytes containing only zeroes or only ones won't change when encrypted, because the order of the bits will be unaffected no matter how they're swapped around. If an encrypted file uses either of these bytes for padding, this padding will remain visible in the encrypted text, potentially acting as a clue for reverse engineers. Furthermore, bytes with fewer possible combinations according to the table above wouldn't need as much work when reversing the encryption through brute force.
