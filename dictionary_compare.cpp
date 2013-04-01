/* Paul Gentemann
 * CS 463
 * File Name : dictionary_test.cpp
 * Last Modified : Fri 22 Mar 2013 04:11:28 PM AKDT
 * Description : Reads file(s) and compares words with dictionaries. Set c++11
 * flag to compile. -std=c++11
 */

#include <vector> 
using std::vector;
#include <string>
using std::string;
#include <cstdlib>         // For exit
#include <fstream>
using std::ofstream; using std::ifstream;
#include <iostream>
using std::cout; using std::endl; using std::ios;
#include <algorithm>       // For for_each
#include <unordered_map>   // For unique words


// This does what tolower should have done from the beginning.
void lower(char &c) { c = tolower(c); }

// Changes punctuation marks, except "'", to blank spaces.
void depunct(char &c) { if (ispunct(c) && (c != '\'')) c = ' '; }

// Fills string vector with words from ifstream, set lower to 't' if you need
// to ignore case. 
void fill(ifstream &in, vector<string> &f) 
{
   string word;
   while (!in.eof())
   {
      getline(in, word);
      int pos = word.find(',');        
      f.push_back(word.substr(0, pos));
      // rank.push_back(word.substr(pos, word.size()));
   }
}

// Searches dictionary on a given string, returning 1 if it matches.
int search_count(const std::vector<string> &d, const string &s)
{  
   if (std::binary_search(d.begin(), d.end(), s))
      return 1;
   return 0;
}


// MAIN
int main(int argc, char * argv[])
{
   if (argc < 2) 
   {
      cout << "Usage: dtest [file ..]" << endl;
      exit(1);
   }

   // Open the dictionaries.
   ifstream dictionary, brown, oed, uk;
   dictionary.open("/usr/share/dict/american-english", ios::binary);
   brown.open("data/top_words/brown_top_135_words.txt", ios::binary);
   oed.open("data/top_words/oed_top_100_words.txt", ios::binary);
   uk.open("data/top_words/uk_top_1000_words.txt", ios::binary);

   const int DICTS = 3;
   vector<string> dvec;                // Standard Dictionary.
   vector<string> top [DICTS];         // Top-n-words Dictionaries.

   while (!dictionary.eof())
   {
      string word;
      getline(dictionary, word);
      for_each(word.begin(), word.end(), lower); // ignore case.
      dvec.push_back(word);
   }

   fill(brown, top[0]); //, rank[0]); // Rank analysis?
   fill(oed, top[1]); //, rank[1]);
   fill(uk, top[2]); //, rank[2]);

   dictionary.close(); // Done reading the dictionaries.
   brown.close();
   oed.close();
   uk.close();

   // Check for errors in creating vectors, and shrink to fit.
   if (dvec.empty()) 
      exit(1); 
   dvec.shrink_to_fit();            // -std=c++11. 
   for (int i = 0; i < DICTS; ++i)
   {
      if (top[i].empty())
         exit(1);
      top[i].shrink_to_fit(); 
   }


   // Now go through files.
   for (int i = 1; i < argc; ++i)
   {
      cout << "Processing: " << argv[i] << " ";
      // Load the source file. If it fails, go to next source.
      ifstream corpus(argv[i]);
      if (!corpus.is_open()) continue;       // Skip to next corpus.

      ofstream analysis("analysis.txt", ios::binary | ios::app); 

      // Counting tools:
      string word;
      int wordcount = 0,                     // Total words in text.
          matchdic = 0,
          matchcount [DICTS] = {0,0,0};      // Words matched in top_n
      // Words paired up with # of occurrences
      std::unordered_map<string,int> unique[2]; // 0=matched, 1=unmatched.

      // Analysis
      while (corpus >> word)
      {
         ++wordcount;
         for_each(word.begin(), word.end(), lower);   // Case insensitive
         for_each(word.begin(), word.end(), depunct); // Punct insensitive
         int pos = word.find(' ');        // Clean up after depunct.
         // If there are none, we are ready to search the dictionaries.
         if (pos == string::npos)
         {
            // If word has already been searched, it is either already in
            // the Dictionary, or known to be not. So increment unique counter.
            if (unique[0].count(word) > 0)
            {
               unique[0][word]++;   
               ++matchdic;
            }
            else if (unique[1].count(word) > 0)
               unique[1][word]++;   
            else     // Else, figure out where the word belongs.
            {
               int occurred = search_count(dvec, word);
               unique[1 - occurred].emplace(word, 1);
               matchdic += occurred;
            }

            // Now check the top_n (faster) dictionaries.
            for (int j = 0; j < DICTS; ++j)
               matchcount[j] += search_count(top[j], word);
         }
         else  // Otherwise, we need to search the subparts.
         {
            while (pos > 0)
            {
               ++wordcount;
               string part = word.substr(0, pos);

               // Repeat above code, but on the split-up words.
               if (unique[0].count(part) > 0)
               {
                  unique[0][part]++;   
                  ++matchdic;
               }
               else if (unique[1].count(part) > 0)
                  unique[1][part]++;   
               else     
               {
                  int occurred = search_count(dvec, part);
                  unique[1 - occurred].emplace(part, 1);
                  matchdic += occurred;
               }
               for (int j = 0; j < DICTS; ++j)
                  matchcount[j] += search_count(top[j], part);

               // Chomp word and find next gap.
               word = word.substr(pos, word.size());
               pos = word.find(' ');
            }
         }
      }

      cout << "...processed!" << endl;
      /* 
      // Troubleshooting Values
      cout << "words: " << wordcount << endl;
      cout << "total matches: " << matchdic << endl;
      cout << "unique matches: " << unique[0].size() << endl;
      cout << "unique nonmatches: " << unique[1].size() << endl;
      cout << "brown matches: " << matchcount[0] << endl;
      cout << "oed matches: " << matchcount[1] << endl; 
      cout << "uk matches: " << matchcount[2] << endl;
      */

      /*
      // Troubleshooting Unique Counts
      for (auto& q: unique[0]) 
         cout << q.first << " happened " << q.second << " times." << endl;
      for (auto q : unique[1])
         cout << q.first << " happened " << q.second << " times." << endl;
      */

      // Output: 
      // <file>, <total words>, <dictionary matches>, <unique matches>, 
      // <brown top 135 matches>, <oed top 100 matches>, <uk top 1000 matches>
      analysis << argv[i] << ", " << wordcount << ", " << matchdic << ", " 
         << unique[0].size() << ", " << matchcount[0] << ", " << matchcount[1] 
         << ", " << matchcount[2] << endl;

      corpus.close();
      analysis.close();
   }
   return 0;
}
