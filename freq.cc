//
// freq.cc
//
// This implements the operations for the unordered dictionary data
// structure with the type `freq::dict*` for Project 1 of the Spring
// 2020 offering of CSCI 221.  Specifically, we define the functions
// for a bucket hash table that stores a collection of words and their
// integer counts.
//
// It is described more within the README for this project.
//
// The functions it defines include
//    * `int hashValue(std::string,int)`: give the hash location for a key
//    * `freq::dict* freq::build(int,int)`: build a word count dictionary 
//    * `int freq::totalCount(freq::dict*)`: get the total word count
//    * `int freq::numKeys(freq::dict*)`: get number of words
//    * `void freq::increment(freq::dict*,std::string)`: bump a word's count 
//    * `int freq::getCount(freq::dict*,std::string)`: get the count for a word
//    * `freq::entry* freq::dumpAndDestroy(freq::dict*)`: get the word counts, sorted by frequency
//    * `void freq::rehash(freq::dict*)`: expand the hash table
//
// The top four are implemented already, the other four need to be written.
//

#include <string>
#include <iostream>
#include "freq.hh"


// * * * * * * * * * * * * * * * * * * * * * * *
//
// HELPER FUNCTIONS FOR CHOOSING HASH TABLE SIZE
//

// isPrime(n)
//
// Return whether or not the given integer `n` is prime.
//
bool isPrime(int n) {
  // Handle the obvious cases, including even ones.
  if ((n <= 2) || (n % 2 == 0)) {
    return (n == 2);
  }
  // Try several odd divisors.
  int d = 3;
  while (d*d <= n) {
    if (n % d == 0) {
      // It has a divisor. It's not prime.
      return false;
    }
    d += 2;
  }
  // No divisors. It's prime.
  return true;
}

// primeAtLeast(n)
//
// Return the smallest prime number no smaller
// than `n`.
//
int primeAtLeast(int n) {
  if (n <= 2) {
    return 2;
  }
  int p = 3;
  while (p < n || !isPrime(p)) {
    p += 2;
  }
  return p;
}

// * * * * * * * * * * * * * * * * * * * * * * *
//
// HELPER FUNCTIONS FOR COMPUTING THE HASH VALUE
//

// charToInt(c):
//
// Returns an integer between 0 and 31 for the given character. Pays
// attention only to letters, the contraction quote, "stopper" marks,
// and space.
//
//
int charToInt(char c) {
  if (c >= 'a' && c <= 'z') {
    return c - 'a' + 1;
  } else if (c == '.') {
    return 27;
  } else if (c == '!') {
    return 28;
  } else if (c == '?') {
    return 29;
  } else if (c == '\'') {
    return 30;
  } else if (c == ' ') {
    return 31;
  } else {
    return 0;
  }
}

// hashValue(key,tableWidth):
//
// Returns an integer from 0 to tableWidth-1 for the given string
// `key`. This serves as a hash function for a table of size
// `tableWidth`.  
//
// This method treats the string as a base-32 encoding of the integer
// it computes, modulo `tableWidth`. It relies on `charToInt` defined
// just above.
//
int hashValue(std::string key, int modulus) {
  int hashValue = 0;
  for (char c: key) {
    // Horner's method for computing the value.
    hashValue = (32*hashValue + charToInt(c)) % modulus;
  }
  return hashValue;
}


// * * * * * * * * * * * * * * * * * * * * * * *
//
// Operations on freq::dict, and other support functions.
//
namespace freq {

//    * `int hashValue(std::string,int)`: give the hash location for a key
//    * `freq::dict* freq::build(int,int)`: build a word count dictionary 
//    * `int freq::totalCount(freq::dict*)`: get the total word count
//    * `int freq::numKeys(freq::dict*)`: get number of words
//    * `void freq::increment(freq::dict*,std::string)`: bump a word's count 
//    * `int freq::getCount(freq::dict*,std::string)`: get the count for a word
//    * `freq::entry* freq::dumpAndDestroy(freq::dict*)`: get the word counts, sorted by frequency
//    * `void freq::rehash(freq::dict*)`: expand the hash table
  // buildBuckets(howMany):
  //
  // Return an array of buckets of length `howMany`.
  //
  bucket* buildBuckets(int howMany) {
    bucket* bs = new bucket[howMany];
    for (int i=0; i<howMany; i++) {
      bs[i].first = nullptr;
    }
    return bs;
  }

  // build(initialSize,loadFactor):
  //
  // Build a word count dictionary that is roughly the given size, and
  // maintains the given load factor in its hash table.
  //
  dict* build(int initialSize, int loadFactor) {
    dict* newD = new dict;
    newD->numIncrements = 0;
    newD->numEntries    = 0;
    newD->loadFactor    = loadFactor; 
    newD->numBuckets    = primeAtLeast(initialSize);
    newD->buckets       = buildBuckets(newD->numBuckets);
    return newD;
  }

  // numKeys(D):
  //
  // Gives back the number of entries stored in the dictionary `D`.
  //
  int numKeys(dict* D) {
    return D->numEntries;
  }

  // totalCount(D):
  //
  // Gives back the total of the counts of all the entries in `D`.
  //
  int totalCount(dict* D) {
    return D->numIncrements;
  }


  // getCount(D,w):
  //
  // Gets the count associated with the word `w` in `D`.
  //
  int getCount(dict* D, std::string w) {

    int bucketIndex = hashValue(w,D->numBuckets);
    entry* currentEntry=D->buckets[bucketIndex].first;
    if(currentEntry==nullptr){
      return 0;
    }

    do{
      if(currentEntry->word == w){
        return currentEntry->count;
      }
      currentEntry = currentEntry->next;
    }while(currentEntry!=nullptr);
    return 0;
  }

  // rehash(D):
  //
  // Roughly doubles the hash table of `D` and places its entries into
  // that new structure.
  //
  void rehash(dict* D) {

    //we want to keep track of the old stuff to put them into the new buckets
    int oldNumBuckets = D->numBuckets;
    bucket* oldTable = D->buckets;

    //creates the new buckets and updates numBuckets
    D->numBuckets = primeAtLeast(2*D->numBuckets);
    D->buckets = buildBuckets(D->numBuckets);

    //iterates through the old buckets
    for(int i = 0 ; i < oldNumBuckets; i++){
      entry* currentEntry = oldTable[i].first;
      while(currentEntry!=nullptr){

	//we get the newBucket index and see if that bucket is empty
	entry* nextEntry = currentEntry->next;
	int newBucketIndex = hashValue(currentEntry->word,D->numBuckets);

	//if it is empty, we just put the old entry at the beginning 
	if(D->buckets[newBucketIndex].first ==nullptr){
	  D->buckets[newBucketIndex].first = currentEntry;
	  currentEntry->next= nullptr;
	}
	//if it isn't empty, we stitch the old entry at the beginning of the list
	else{
	  currentEntry->next = D->buckets[newBucketIndex].first;
	  D->buckets[newBucketIndex].first = currentEntry;
	}
	currentEntry= nextEntry;

      }
      
    }
    //reallocates space from the old table
    delete [] oldTable;
  }

  // increment(D,w):
  //
  // Adds one to the count associated with word `w` in `D`, possibly
  // creating a new entry.
  //
  void increment(dict* D, std::string w) {

    //if we are over the load factor, we rehash
   if((D->numEntries+1)/D->numBuckets > D->loadFactor){
     rehash(D);
   }

   //we increment increments, and find the index of our bucket
    D->numIncrements++;
    int bucketIndex = hashValue(w,D->numBuckets);

    //we create a new entry in case we need it
    entry* currentEntry=D->buckets[bucketIndex].first;
    entry* newEntry = new entry;
    newEntry->word = w;
    newEntry->count = 1;

    //if the bucket is empty, we put the new entry there
    if(currentEntry==nullptr){
      D->buckets[bucketIndex].first = newEntry;
      newEntry->next = nullptr;
      D->numEntries++;
    }
    //if the bucket isn't empty
    else{
      entry* prevEntry = nullptr;
      do{
	//if we find the entry, we increment it's count
	if(currentEntry->word == w){
	  currentEntry->count++;
	  delete newEntry;
	  return;
	  
	}
	prevEntry = currentEntry;
	currentEntry = currentEntry->next;
      }while(currentEntry!=nullptr);

      //otherwise we just add the new entry at the end of this bucket's list
      prevEntry->next = newEntry;
      newEntry->next =nullptr;
      D->numEntries++;
    }
  }

  // dumpAndDestroy(D):
  //
  // Return an array of all the entries stored in `D`, sorted from
  // most to least frequent.
  //
  // Deletes all the heap-allocated components of `D`.
  //
  entry* dumpAndDestroy(dict* D) {
    //we iterate through the entries 
    entry* es = new entry[D->numEntries];
    int nextOpenSpot = 0;
    entry* currentEntry;
    for(int i = 0; i< D->numBuckets ; i++){
      currentEntry=D->buckets[i].first;
      while(currentEntry!=nullptr){
	es[nextOpenSpot] = *currentEntry;
	int iterator = nextOpenSpot;
	//here we use a simplified version of insetion sort so that the array is in desending order (with respect to count)
	while(iterator >0 and es[iterator-1].count < es[iterator].count){
	  //swaps the entry we just added and the one before it	  
	  entry placeHolder = es[iterator-1];
	  es[iterator-1] = es[iterator];
	  es[iterator] = placeHolder;
	  iterator--;
	}
	nextOpenSpot++;
	entry* toBeDeleted = currentEntry;
	currentEntry= currentEntry->next;
	delete toBeDeleted;
      }
    }
    //reallocate space
    delete [] D->buckets;
    delete D;
    return es;
  }
} // end namespace freq

