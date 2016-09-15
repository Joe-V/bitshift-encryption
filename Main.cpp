#include <iostream>
#include <random>
#include <string>
#include <fstream>
using namespace std;

#define HELP_TEXT "Bitshift is a custom encryption program that pseudorandomly swaps individual bits in each \
byte of a file to encrypt it. It uses a symmetric encryption key to determine the order in which bits are \
swapped within each byte, meaning the same key is used to both encrypt and decrypt the message. Strings intended \
for encryption are read from a given input file and the resulting cyphertext is written to a given output file.\n\n\
Usage:\n\
\tbitshift [-dh] <key> <input-file> <output-file>\n\n\
Arguments:\n\
\t-d - denotes that the given string is cyphertext that should be decrypted\n\
\t-h - displays this help message and exits\n\n\
Sample use:\n\
\tbitshift helloworld source.txt dest.txt - encrypts the contents of source.txt using the key \"helloworld\", \
writing the result to dest.txt\n\
\tbitshift -d helloworld dest.txt recovered.txt - decrypts the cyphertext in dest.txt to recover the original \
plaintext, through use of the '-d' option and the same key that was used before\n\n\
This program reads and writes files as binary data and can therefore be used against all types of files, not just \
text-based ones. To use an encryption key that includes spaces, wrap the entire string in double quotation marks \
(\"...\")."

class IndexGenerator {
private:
	// Starts with all bits set to 0. Bits are set to 1 as their associated
	// indexes (0-7) are generated and returned.
	unsigned char* indexRecord;
public:
	IndexGenerator() {
		this->indexRecord = new unsigned char(0b00000000);
	}
	~IndexGenerator() {
		delete this->indexRecord;
		this->indexRecord = nullptr;
	}
	// Pseudorandomly selects an index number between 0 and 7 and returns it. When called multiple times, this
	// method is guaranteed to return all of the numbers in the range 0 to 7 exactly once. After all 8 values in
	// this range have been returned, the index record is reset and the numbers in this range can be returned
	// again, potentially in a different order to before.
	uint16_t nextIndex() {
		if ((*indexRecord) == 0b11111111) *indexRecord = 0b00000000; // If all values from 0 to 7 have been returned, reset the record.
		uint16_t r;
		do {
			r = rand() % 8; // Continuously select random values in the range [0,7]...
		} while ((*indexRecord & (0b00000001 << r)) > 0); // ...until we find one we haven't returned yet.
		*indexRecord |= 0b00000001 << r;
		return r;
	}
};

// Given the index of a bit (0-7) from the source character and another index of a bit from the
// destination character, this method copies the value of the given bit in the source character
// to the given bit in the destination character. The source and destination characters are
// modified in-place (the original references are modified).
void process(char* sourceChar, char* destChar, uint16_t bitFromSource, uint16_t bitToDest) {
	unsigned char targetBit = (((*sourceChar) & (0b00000001 << bitFromSource)) > 0 ? 0b1 : 0b0); // The value of the targetted bit in the source character (binary 0 or 1).
	(*destChar) |= (targetBit << bitToDest); // Set the associated bit in the destination character to this value (logical OR).
}

int main(int argc, char* argv[]) {
	bool decrypt = false;

	int curArg = 1;
	while (curArg < argc && argv[curArg][0] == '-') {
		switch (argv[curArg][1]) {
		case ('d') : // Enable decryption mode (treat the input file as cyphertext).
			decrypt = true;
			break;
		case ('h') : // Display help and exit.
			cout << HELP_TEXT << endl;
			return 0;
		default: // Unrecognised option - exit with an error.
			cerr << "Unknown option " << argv[curArg] << " (note that arguments are case sensitive)." << endl;
			return 2;
		}
		curArg++;
	}

	if (argc - curArg < 3) {
		cerr << "Usage: bitshift [-dh] <key> <input-file> <output-file>" << endl << "Try `bitshift -h` for help" << endl;
		return 1;
	}

	size_t keyHash = std::hash<std::string>()(argv[curArg]);
	srand(keyHash);

	ifstream ins(argv[curArg + 1], ios::in | ios::binary);
	if (!ins.is_open()) {
		cerr << "The input file `" << argv[curArg + 1] << "` cannot be read." << endl;
		return 2;
	}
	ofstream outs(argv[curArg + 2], ios::out | ios::binary);

#ifdef _DEBUG
	cout << (decrypt ? "Decryption mode" : "Encryption mode") << endl;
#endif

	IndexGenerator* sourceGen = new IndexGenerator(); // Used to select source bits from each byte of the input file.
	IndexGenerator* destGen = new IndexGenerator(); // Used to select destination bits from each byte of the output file.

	// This is the main loop where the encryption/decryption takes place, on a byte-by-byte basis.
	// inC is used to hold the current character from the input file.
	// outC holds the encrypted/decrypted version of this character and is written to the output file after each iteration.
	// We reset outC to 0 at the start of each iteration to clear it of any data from the previous iteration.
	for (char *inC = new char(), *outC = new char(); ins.read(inC, 1); (*outC) = 0b00000000) {
		// Cycle through each bit in the current byte.
		for (int i = 0; i < 8; i++) {
			// Pseudorandomly generate two index numbers in the range 0 to 7 inclusive.
			uint16_t
				indexA = sourceGen->nextIndex(),
				indexB = destGen->nextIndex();

			if (!decrypt) {
				process(inC, outC, indexA, indexB);
			} else {
				// To decrypt, we swap the values generated by the source and destination generators.
				process(inC, outC, indexB, indexA);
			}
		}
		// Write the encrypted/decrypted character to the output file.
		outs.write(outC, 1);
#ifdef _DEBUG
		cout << (*inC) << " (" << (int)(*inC) << ")" << "\t->\t" << (*outC) << " (" << (int)(*outC) << ")" << endl;
#endif
	}

	ins.close();
	outs.close();

#ifdef _DEBUG
	cout << "Press Enter to exit.";
	cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
#endif
	return 0;
}