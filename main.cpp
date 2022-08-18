#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <math.h>
#include <numeric>
#include <fstream>
#include <iostream>

using namespace std;


// Reads possible wordles from "filename"
vector<string> loadWords(string filename)
{
    string cur;
    vector<string> result;
    ifstream file(filename);
    
    while (getline(file, cur))
    {
        transform(cur.begin(), cur.end(), cur.begin(), ::toupper);
        result.push_back(cur);
    }
    
    return result;
}


// 0 = Gray (wrong place, wrong char)
// 1 = Yellow (wrong place, right char)
// 2 = Green (right place, right char)
string getResult(string &guess, string &word)
{
    string result;
    
    for (int i = 0; i < 5; i++)
    {
        if (guess[i] == word[i])
            result += '2';
        
        else if (word.find(guess[i]) != string::npos)
            result += '1';
        
        else
            result += '0';
    }
    
    return result;
}


// "words" = the entire, original word list
// "copy" = a list of words that could be the Wordle
string getGuess(vector<string> &words, vector<string> &copy)
{
    // Record the count of each response for every remaining possible answer
    // Ex: "about" returns "10200" for 3 strings, "20000" for 4 strings, etc.
    map<string, int> count;

    for (string guess : words)
        for (string answer : copy)
            count[guess + getResult(guess, answer)]++;

    string guess = words[0];
    int minWords = pow(10, 9);
    
    // Used to check if a guess could be the Wordle
    set<string> wordleSet(copy.begin(), copy.end());

    for (string curGuess : words)
    {
        int curWords = 0;

        for (string answer : copy)
            // How many answers will this guess not eliminate?
            curWords += count[curGuess + getResult(curGuess, answer)];

        // curWords == minWords... prioritizes guesses that could be the Wordle --> avoid infinite loop!
        if (curWords < minWords || (curWords == minWords && wordleSet.find(curGuess) != wordleSet.end()))
        {
            guess = curGuess;
            minWords = curWords;
        }
    }

    return guess;
}


// Returns the number of guesses used
// "words" = original list of possible words
// "word" = the current word we're trying to guess
// "verbose" = should the algorithm print its process?
// "firstGuess" = precomputed optimal first guess to save time
int play(vector<string> &words, string word, bool verbose=false, string firstGuess="RAISE")
{
    int numGuesses = 0;
    string lastGuess = "";
    
    // List of possible answers
    vector<string> copy = words;
    
    while (copy.size() > 1)
    {
        numGuesses++;
        string guess = numGuesses == 1 ? firstGuess : getGuess(words, copy);
        lastGuess = guess;
        
        if (verbose)
            cout << "Guess #" << numGuesses << ": " << guess << endl;
        
        string result = getResult(guess, word);
        
        // New possible answers
        vector<string> newCopy;
        
        for (string answer : copy)
            if (result == getResult(guess, answer))
                newCopy.push_back(answer);
        
        copy = newCopy;
    }
    
    if (lastGuess != word)
    {
        numGuesses++;
        
        if (verbose)
            cout << "Guess #" << numGuesses << ": " << word << endl;
    }
    
    if (verbose)
        cout << "The word was: " << copy[0] << ". We found it in " << numGuesses << " guesses!" << endl;
    
    return numGuesses;
}


int main()
{
    vector<string> words = loadWords("wordlewords.txt");
    
    if (words.size() == 0)
    {
        cout << "Couldn't read file!" << endl;
        return 0;
    }
    
    // # of Guesses
    vector<int> results;

    for (int i = 0; i < 2315; i++)
    {
        cout << "Wordle " << i+1 << ": " << endl;
        results.push_back(play(words, words[i], true));
        cout << endl;
    }

    int n = int(results.size());
    int sum = accumulate(results.begin(), results.end(), 0);

    cout.precision(3);
    sort(results.begin(), results.end());

    cout << "Here are the results! " << endl;
    cout << "Minimum # of Guesses: " << results[0] << endl;
    cout << "Median # of Guesses: "  << results[n/2] << endl;
    cout << "Maximum # of Guesses: " << results[n-1] << endl;
    cout << "Average # of Guesses: " << sum / float(n) << endl;
}
