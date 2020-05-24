#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// structure of a block
typedef struct block
{
	int LRU;
	int dirty;
	int tag;
    char data[1024];
} block;

// Initialize cache, memory, and file
block **Cache;
char *Memory;
FILE* myFile;

// Function to take log base 2 of a number
int log2(int n) {
    int r=0;
    while (n>>=1) r++;
    return r;
}

// Initializes memory and cache
void init(int associativity, int numSets){
	// Allocate space for memory
	// Max size of memory is 16 MB
	Memory = (char*)malloc(sizeof(char) * 1024 * 1024 * 16);
	// Allocate space for cache
	Cache = (block **)malloc(sizeof(block *) * numSets);
	for(int i = 0; i < numSets; i++){
   		Cache[i] = (block *)malloc(sizeof(block) * associativity);
	}
	// Initialize cache fields
	for(int i=0; i < numSets; i++){
		for(int j=0; j < associativity; j++){
			Cache[i][j].LRU = -1;
			Cache[i][j].dirty = 0;
			Cache[i][j].tag = -1;
		}
	}
}

// Check for a hit
int checkHit(int associativity, int setIndex, int tag){
	int isHit = -1;
	int blockIndex;

	// Loop over set for block tag
	for(blockIndex=0; blockIndex < associativity; blockIndex++){
		if(Cache[setIndex][blockIndex].tag == tag){
			isHit = 1;
			printf("hit");
			break;
		}
	}
	if(isHit == -1){
		blockIndex = -1;
		printf("miss");
	}
	return blockIndex;
}

// Increment LRU data besides active block
void incLRU(int associativity, int setIndex, int indexLRU){
	// Reset active block LRU
	Cache[setIndex][indexLRU].LRU = 0;
	for(int j=0; j < associativity; j++){
		if(Cache[setIndex][j].LRU != -1 & j != indexLRU){
			Cache[setIndex][j].LRU++;
		}
	}
}

// Store data block into memory
void cacheToMem(int setIndex, int tagBits, int offsetBits, int blockSize, int blockIndex){
	// Calculate base address of block
	int storeAddress = (Cache[setIndex][blockIndex].tag << (24-tagBits)) + (setIndex << offsetBits);
	// Store block data into memory
	for(int i = 0; i < blockSize; i++){
		*(Memory + storeAddress + i) = Cache[setIndex][blockIndex].data[i];
	}
}

// Scan new data into cache starting from offset to ending accessSize later
void fileToCache(int setIndex, int offset, int accessSize, int blockIndex){
	for(int i = offset; i < accessSize + offset; i++){
		fscanf(myFile, "%02hhx", &Cache[setIndex][blockIndex].data[i]);
	}
}

// Store memory data into entire cache block
void memToCache(int setIndex, int tag, int tagBits, int offsetBits, int blockSize, int blockIndex){
	// Calculate base address to fetch
	int fetchAddress = (tag << 24-tagBits) + (setIndex << offsetBits);
	for(int i = 0; i < blockSize; i++){
		Cache[setIndex][blockIndex].data[i] = *(Memory + fetchAddress + i);
	}
}

// Checks for empty or LRU block, then writes dirty data to memory if needed
int checkLRU(int associativity, int setIndex, int tagBits, int offsetBits, int blockSize){
	int indexLRU = -1;
	int maxLRU = -1;
	int memWrt = 1;

	// Loop over set to find empty/LRU block
	for(int i=0; i < associativity; i++){
		// Check if block is empty
		if(Cache[setIndex][i].LRU == -1){
			indexLRU = i;
			memWrt = -1;
			break;
		}
		// Check for LRU block
		else{
			if(Cache[setIndex][i].LRU > maxLRU){
				maxLRU = Cache[setIndex][i].LRU;
				indexLRU = i;
			}
		}
	}
	// If old data is dirty and needs to be sent to memory
	if(memWrt == 1 & Cache[setIndex][indexLRU].dirty == 1){
		cacheToMem(setIndex, tagBits, offsetBits, blockSize, indexLRU);
	}

	return indexLRU;
}

	

void load(int currAddress, int accessSize, int associativity, int blockSize, 
			int setIndex, int tag, int offset, int tagBits, int offsetBits){
	// Check for tag for a hit
	int blockIndex = checkHit(associativity, setIndex, tag);
	printf(" ");

	// Check if hit
	if(blockIndex != -1){
		for(int i = offset; i < accessSize + offset; i++){
			printf("%02hhx", Cache[setIndex][blockIndex].data[i]);
		}
		printf("\n");
	}

	// Else no hit
	else{
		// Print data from memory
		for(int i = 0; i < accessSize; i++){
			printf("%02hhx", *(Memory + currAddress + i));
		}
		printf("\n");

		// Check for empty block or LRU block, and evacuate old data if needed
		blockIndex = checkLRU(associativity, setIndex, tagBits, offsetBits, blockSize);
		
		// Reset dirty bit and set tag for cache
		Cache[setIndex][blockIndex].dirty = 0;
		Cache[setIndex][blockIndex].tag = tag;

		// Store memory data into cache data
		memToCache(setIndex, tag, tagBits, offsetBits, blockSize , blockIndex);
	}
	// Increment LRU
	incLRU(associativity, setIndex, blockIndex);
}

void store(int currAddress, int accessSize, int associativity, int blockSize, 
			int setIndex, int tag, int offset, int tagBits, int offsetBits){
	// Check for tag for a hit
	int blockIndex = checkHit(associativity, setIndex, tag);
	printf("\n");


	// If there is a hit
	if(blockIndex != -1){
		// Set dirty bit
		Cache[setIndex][blockIndex].dirty = 1;
	}

	//If there is no hit
	else{
		// Check for empty block or LRU block, and evacuate old data if needed
		blockIndex = checkLRU(associativity, setIndex, tagBits, offsetBits, blockSize);

		// Store memory data into cache
		memToCache(setIndex, tag, tagBits, offsetBits, blockSize, blockIndex);

		// Set dirty bit and tag of cache
		Cache[setIndex][blockIndex].dirty = 1;
		Cache[setIndex][blockIndex].tag = tag;
	}
	// Scan data into cache
	fileToCache(setIndex, offset, accessSize, blockIndex);

	// Increment LRU
	incLRU(associativity, setIndex, blockIndex);
}

int main (int argc, char* argv[]) {
	// Declare variable to read in inputs
	int cacheSize, associativity, blockSize;

    // Buffer to store instruction (i.e. "load" or "store")
	char instruction_buffer[6];

    // Open the trace file in read mode
	myFile = fopen(argv[1], "r");

    // Read in the command line arguments
	sscanf(argv[2], "%d", &cacheSize);
	sscanf(argv[3], "%d", &associativity);
	sscanf(argv[4], "%d", &blockSize);

	// Declare ints to calculate necessary info
	int numSets, numBlocks, cacheSizePower, associativityPower, blockSizePower;
	int tagBits, setIndexBits, offsetBits;

	// Determine N where 2^N, needed since we cannot use powers function
	cacheSizePower = log2(cacheSize) + 10;
	associativityPower = log2(associativity);
	blockSizePower = log2(blockSize);

	// Calculate the number of sets and blocks
	numSets = 1 << ((cacheSizePower - blockSizePower) - associativityPower);
	numBlocks = 1 << (cacheSizePower - blockSizePower);

	// Calculate the bits needed for setIndex, offset, and tag
	setIndexBits = log2(numSets);
	offsetBits = blockSizePower;
	tagBits = 24 - setIndexBits - offsetBits;

	// Initialize the cache and memory
	init(associativity, numSets);

    // Keep reading the instruction until end of file
	while(fscanf(myFile,"%s", &instruction_buffer)!=EOF) {
		// Initialize ints for input and tag, setIndex, offset
		int currAddress, accessSize, tag, setIndex, offset;

        // Read the address and access size info
		fscanf(myFile, "%x", &currAddress);
		fscanf(myFile, "%d", &accessSize);

		// Calculate tag, setIndex, and offset
		offset = ((1 << offsetBits) -1) & currAddress;
		tag = currAddress >> (24 - tagBits);
		unsigned mask = ((1 << 24) -1) >> (tagBits + offsetBits) << offsetBits;
		setIndex = (currAddress & mask) >> offsetBits;
		
		// Check if load
		if(instruction_buffer[0]=='l'){    // If load
            // Print the load line in the same format as trace file
			printf("load 0x%x ", currAddress);
			load(currAddress, accessSize, associativity, blockSize, setIndex, tag, offset, tagBits, offsetBits);
		}
		// Else store
        else{
            // Print the store line in the same format as trace file
            printf("store 0x%x ", currAddress);
			store(currAddress, accessSize, associativity, blockSize, setIndex, tag, offset, tagBits, offsetBits);
		}
	}

	return EXIT_SUCCESS;
}
