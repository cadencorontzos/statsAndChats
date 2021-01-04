#include <string>
#include <iostream>
#include "gram.hh"
#include <ctime>
#include <cstdlib>

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

int hashValue(std::string key, int modulus) {
  int hashValue = 0;
  for (char c: key) {
    // Horner's method for computing the value.
    hashValue = (32*hashValue + charToInt(c)) % modulus;
  }
  return hashValue;
}

namespace gram {

  bucket* buildBuckets(int howMany) {
    bucket* bs = new bucket[howMany];
    for (int i=0; i<howMany; i++) {
      bs[i].first = nullptr;
    }
    return bs;
  }

  //rehashed our hashtable to maintain loadfactor
  void rehash(dict* D){

    //we want to keep track of the old stuff so we can put it into the new buckets
    int oldNumBuckets = D->numBuckets;
    bucket* oldTable = D->buckets;

    //makes the new buckets and updates numBuckets
    D->numBuckets = primeAtLeast(2*D->numBuckets);
    D->buckets = buildBuckets(D->numBuckets);

    //we iterate through the old buckets here
    for(int i = 0; i < oldNumBuckets; i++){

      gram* oldGram = oldTable[i].first;

      //we go through the grams in the old bucket and put them into their new buckets
      while(oldGram!=nullptr){
	int newIndex = hashValue(oldGram->words,D->numBuckets);
	gram* currentFirstGram = D->buckets[newIndex].first;
	gram* nextGram = oldGram->next;

	//we just insert the oldGram as the first gram of it's new bucket.
	if(currentFirstGram==nullptr){
	  D->buckets[newIndex].first = oldGram;
	  oldGram->next = nullptr;
	}
	else{
	  oldGram->next = D->buckets[newIndex].first;
	  D->buckets[newIndex].first = oldGram;
	}
	oldGram = nextGram;

      }
    }
    //reallocates the space from our old buckets
    delete [] oldTable;
  }

  //builds a dict, with all the defaults set
  dict* build(int initialSize, int loadFactor) {
    srand(time(0));
    dict* newD = new dict;
    newD->numEntries = 0;
    newD->loadFactor = loadFactor;
    newD->numBuckets = primeAtLeast(initialSize);
    newD->buckets = buildBuckets(newD->numBuckets);
    return newD;
  }

  //gets a random follower of a word
  std::string get(dict* D, std::string ws) {

    //we find the appropriate bucket, iterate through til we find our word
    int hashIndex= hashValue(ws,D->numBuckets);
    gram* currentGram = D->buckets[hashIndex].first;
    while(currentGram!=nullptr and currentGram->words != ws){
      currentGram = currentGram->next;
    }

    //we pick a follower # "randomly" and iterate to it
    int randInt = std::rand() % currentGram->number;
    follower* currentFollower = currentGram->followers;

    while(currentFollower!=nullptr and randInt!=0){
      currentFollower = currentFollower->next;
      randInt--;
    }

    //returns that follower
    return currentFollower->word;
  }

  std::string get(dict* D, std::string w1, std::string w2) {
    return get(D,w1+" "+w2);
  }

  //adds a word gram and it's follower to the hashtable
  void add(dict* D, std::string ws, std::string fw) {
    //if we are goint to exceed the load factor, we immediately rehash
    if((D->numEntries+1)/D->numBuckets > D->loadFactor){

      rehash(D);

    }
    //creates the new entry in case we need to add it
    follower* newFollower = new follower{fw,nullptr};
    gram* newGram = new gram{ws,1,newFollower,nullptr};
    int bucketIndex = hashValue(ws,D->numBuckets);
    gram* currentGram = D->buckets[bucketIndex].first;

    //if this is our first entry
    if(currentGram == nullptr){
      D->buckets[bucketIndex].first = newGram;
      D->numEntries++;
      return;

    }

    gram* prevGram = currentGram;
    bool alreadyInTheDict = false;
    //iterates through the entries in the dict, looking for where the new entry goes
    // (or if the entry is already there)
    while(!alreadyInTheDict and  currentGram!=nullptr ){
      if(currentGram->words == ws){
	alreadyInTheDict = true;
     	delete newGram;
      }else{
	prevGram = currentGram;
	currentGram = currentGram->next;
      }
    }

    //if the gram is aleady in the dict, we just add the new follower (if needed)
    if(alreadyInTheDict){
      follower* currentFollower = currentGram->followers;
      follower* prevFollower = nullptr;
      while(currentFollower!=nullptr){
	if(currentFollower->word == fw){
	  delete newFollower;
	  return;
	}
	prevFollower = currentFollower;
	currentFollower = currentFollower->next;
      }

      if(prevFollower == nullptr){
	currentGram->followers = newFollower;
	currentGram->number++;
	return;

      }
      prevFollower->next = newFollower;
      currentGram->number++;
    }
    
    else{
      //if the gram is not already in there, we just add it
      D->numEntries++;
      prevGram->next = newGram;
      
    }
  }
  
  void add(dict* D, std::string w1, std::string w2, std::string fw) {
     add(D,w1+" "+w2,fw);
  }

  //reallocates space
  void destroy(dict *D) {

    //goes through all the buckets
    for(int i = 0; i< D->numBuckets; i++){

      //goes through all the grams in the buckets
      gram* currentGram = D->buckets[i].first;
      while(currentGram!=nullptr){
	gram* toBeDeleted = currentGram;
	follower* currentFollower = currentGram->followers;

	//deletes all the grams followers
	while(currentFollower !=nullptr){
	  follower* followerToDelete = currentFollower;
	  currentFollower = currentFollower->next;
	  delete followerToDelete;

	}
	//deletes the gram
	currentGram = currentGram->next;
	delete toBeDeleted;

      }




    }
    //deletes D and its buckets
    delete [] D->buckets;
    delete D;

  }  
} 

