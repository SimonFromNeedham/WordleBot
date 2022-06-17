/* High Level Algorithm Description
   1. Load in a list of all possible words ("words") from wordlewords.txt
   2. Maintain another list of feasible solutions that updates after each guess ("validWords")
   3. To guess: consider each word in the word list ("wordlewords.txt") a potential guess
   4. For each guess, exhaustively determine the average number of words it will eliminate
   5. Guess the word that, on average, eliminates the highest number of words from contention
   6. If the solution is guessed, the computer wins. If not, eliminate impossible words ("updateValidWords")
   7. Repeat steps 3-6 until the wordle is guessed (average: 3.1 guesses, max: 5 guesses)
*/

/* Important Notes
   1. The natural runtime of this algorithm is O(n^3). We lower that runtime in two ways:
      a) By maintaining a hashmap of guess+result : word, we remove redundant computations when guessing
      b) By far the most complex calculation is the first guess --> calculate it once and store in guess1.txt
   2. Whenever you change the word list, make sure to clear guess1.txt so a new first guess can be calculated
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <iterator>
#include <algorithm>
#include <random>
using namespace std;


bool SAMPLE = true;
int SAMPLE_SIZE = 100;

// Setting true = easy mode
bool LABEL_ALL_DUPES = false;

// Global list of all possible wordles
unordered_set<string> words;

// The pre-calculated optimal first guess
string firstGuess;

// First = guess+result, Second = word list
unordered_map<string, unordered_set<string> > guessWordMap;

// First = guess+word, second = result
unordered_map<string, string> guessWordResult;


int loadWords();
int loadGuess();
void generateGuessWordMap();
int getSum(vector<int> &v, int n);

string getResult(string const& guess, string const& word);
string makeGuess(unordered_set<string> &wordPool, unordered_set<string> &validWords);
void updateValidWords(unordered_set<string> &validWords, string const& guess, string const&result);
int countNewValidWords(unordered_set<string> &validWords, string const& guess, string const& result);
int solve(string const& word, bool verbose=false);


/* main() steps:
   1. Load words from wordlewords.txt
   2. Load the pre-calculated first guess from guess1.txt (or redo it)
   3. Play a game of wordle on every possible word
   4. Display the results (max guesses, average guesses, etc.)
*/
int main()
{
    // We failed to load the file
    if (loadWords() == 1)
        return 1;
    
    vector<int> results;
    generateGuessWordMap();
    
    if (loadGuess() == 1)
        return 1;
    
    if (SAMPLE)
    {
        vector<string> wordsVector(words.begin(), words.end());
        
        for (int i = 0; i < SAMPLE_SIZE; i++)
        {
            string word = wordsVector[rand() % wordsVector.size()];
            results.push_back(solve(word));
        }
    }
    
    else
    {
        // Exhaustive sample
        int count = 0;
        
        for (auto it = words.begin(); it != words.end(); it++)
        {
            results.push_back(solve(*it));
            
            count++;
            if (count == 100)
                cout << count << " Wordles Completed! " << endl;
        }
    }
        
    
    int n = int(results.size());
    int sum = getSum(results, n);
    
    cout.precision(3);
    sort(results.begin(), results.end());
    
    cout << "Here are the results! " << endl;
    cout << "Minimum # of Guesses: " << results[0] << endl;
    cout << "Median # of Guesses: " << results[n/2] << endl;
    cout << "Maximum # of Guesses: " << results[n-1] << endl;
    cout << "Average # of Guesses: " << sum / float(n) << endl;
}


/* Read in the word list from wordlewords.txt
   Return 0 if we successfully load the file, 1 if we fail
*/
int loadWords()
{
    string line;
    ifstream in("wordlewords.txt");
    
    if (!in)
    {
        cout << "File not found" << endl;
        return 1;
    }
    
    else
    {
        while (getline(in, line))
        {
            transform(line.begin(), line.end(), line.begin(), ::toupper);
            words.insert(line);
        }
        
        in.close();
        return 0;
    }
}


/* If we've already calculated the optimal first guess, use it
   If we haven't already made that calculation, do so now
   Return 0 if we successfully load the file, 1 if we fail
*/
int loadGuess()
{
    string line;
    ifstream in("guess1.txt");
    
    if (!in)
    {
        cout << "File not found" << endl;
        return 1;
    }
    
    else
    {
        getline(in, line);
        
        if (line != "")
        {
            firstGuess = line;
            in.close();
        }
        
        else
        {
            firstGuess = makeGuess(words, words);
            in.close();
            
            // Record the guess for future use
            ofstream out("guess1.txt");
            out << firstGuess;
            out.close();
        }
        
        return 0;
    }
}


// Generates a hashmap of guess+result : words
void generateGuessWordMap()
{
    for (string guess : words)
        for (string word : words)
            guessWordMap[guess+getResult(guess, word)].insert(word);
}


// Returns the sum of values in v, up to index n
int getSum(vector<int> &v, int n)
{
    int total = 0;
    
    for (int i = 0; i < n; i++)
        total += v[i];
    
    return total;
}


// Result format: 0 = grey, 1 = yellow, 2 = green
string getResult(string const& guess, string const& word)
{
    if (guessWordResult.find(guess+word) != guessWordResult.end())
        return guessWordResult[guess+word];
    
    string result = "00000";
    int wordCount[26] = {0};
    
    for (char letter : word)
        wordCount[letter-'A']++;
    
    // Yes, we unrolled the loop. We need maximum speed!
    // These first five if statements mark green squares
    char letter = guess[0];
    
    if (letter == word[0])
    {
        result[0] = '2';
        
        if (!LABEL_ALL_DUPES)
            wordCount[letter-'A']--;
    }
    
    letter = guess[1];
    
    if (letter == word[1])
    {
        result[1] = '2';
        
        if (!LABEL_ALL_DUPES)
            wordCount[letter-'A']--;
    }
    
    letter = guess[2];
    
    if (letter == word[2])
    {
        result[2] = '2';
        
        if (!LABEL_ALL_DUPES)
            wordCount[letter-'A']--;
    }
    
    letter = guess[3];
    
    if (letter == word[3])
    {
        result[3] = '2';
        
        if (!LABEL_ALL_DUPES)
            wordCount[letter-'A']--;
    }
    
    letter = guess[4];
    
    if (letter == word[4])
    {
        result[4] = '2';
        
        if (!LABEL_ALL_DUPES)
            wordCount[letter-'A']--;
    }
    
    // The next five if statements mark yellow squares
    if (result[0] != '2' && wordCount[letter-'A'])
    {
        result[0] = '1';
        
        if (!LABEL_ALL_DUPES)
            wordCount[guess[0]-'A']--;
    }
    
    letter = guess[1];
    
    if (result[1] != '2' && wordCount[letter-'A'])
    {
        result[1] = '1';
        
        if (!LABEL_ALL_DUPES)
            wordCount[guess[1]-'A']--;
    }
    
    letter = guess[2];
    
    if (result[2] != '2' && wordCount[letter-'A'])
    {
        result[2] = '1';
        
        if (!LABEL_ALL_DUPES)
            wordCount[guess[2]-'A']--;
    }
    
    letter = guess[3];
    
    if (result[3] != '2' && wordCount[letter-'A'])
    {
        result[3] = '1';
        
        if (!LABEL_ALL_DUPES)
            wordCount[guess[3]-'A']--;
    }
    
    letter = guess[4];
    
    if (result[4] != '2' && wordCount[letter-'A'])
    {
        result[4] = '1';
        
        if (!LABEL_ALL_DUPES)
            wordCount[guess[4]-'A']--;
    }
    
    guessWordResult[guess+word] = result;
    return result;
}


string makeGuess(unordered_set<string> &wordPool, unordered_set<string> &validWords)
{
    int n = int(validWords.size());
    int minPossibleWords = pow(n, 2);
    string bestGuess = *validWords.begin();
    
    for (auto itGuess = wordPool.begin(); itGuess != wordPool.end(); itGuess++)
    {
        int total = 0;
        
        for (auto itWord = validWords.begin(); itWord != validWords.end(); itWord++)
        {
            total += countNewValidWords(validWords, *itGuess, getResult(*itGuess, *itWord));
        }
        
        if (total < minPossibleWords)
        {
            bestGuess = *itGuess;
            minPossibleWords = total;
        }
    }
    
    return bestGuess;
}


void updateValidWords(unordered_set<string> &validWords, string const& guess, string const&result)
{
    unordered_set<string> newValidWords;
    unordered_set<string> possibleWords = guessWordMap[guess+result];
    
    for (auto it = possibleWords.begin(); it != possibleWords.end(); it++)
        if (validWords.find(*it) != validWords.end())
            newValidWords.insert(*it);
    
    validWords = newValidWords;
}


// We need a separate method for counting to minimize runtime from memory operations
int countNewValidWords(unordered_set<string> &validWords, string const& guess, string const& result)
{
    int count = 0;
    unordered_set<string> possibleWords = guessWordMap[guess+result];
    
    for (auto it = possibleWords.begin(); it != possibleWords.end(); it++)
        if (validWords.find(*it) != validWords.end())
            count++;
    
    return count;
}


// Returns the # of guesses the algorithm needs to solve the wordle
int solve(string const& word, bool verbose)
{
    if (verbose)
        cout << "The wordle is: " << word << endl;
    
    int numGuesses = 1;
    unordered_set<string> validWords(words.begin(), words.end());
    
    while (true)
    {
        string guess;
        
        if (numGuesses == 1)
            guess = firstGuess;
            
        else if (numGuesses != 5)
            guess = makeGuess(validWords, validWords);
        
        else
            guess = makeGuess(words, validWords);
        
        if (verbose)
            cout << "Guess #" << numGuesses << " is: " << guess << endl;
        
        if (guess == word)
        {
            if (verbose)
                cout << "We got the word in " << numGuesses << " guesse(s)!" << endl;
            
            return numGuesses;
        }
        
        else
        {
            string result = getResult(guess, word);
            updateValidWords(validWords, guess, result);
            numGuesses++;
        }
    }
    
    // Something has gone horribly wrong
    return -1;
}
