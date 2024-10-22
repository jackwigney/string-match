#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
using namespace std;

// Trie node structure
struct TrieNode {
    unordered_map<char, TrieNode*> children;
    TrieNode* fail; // Failure link
    vector<pair<string, int>> output; // Store substrings and their start index in str1

    TrieNode() : fail(nullptr) {}
};

// insert a pattern into the trie along with its index
void insertPattern(TrieNode* root, const string& pattern, int index) {
    TrieNode* node = root;
    for (char c : pattern) {
        if (node->children.find(c) == node->children.end()) {
            node->children[c] = new TrieNode();
        }
        node = node->children[c];
    }
    node->output.push_back({pattern, index});
}

// build the failure links for the Aho-Corasick automaton
void buildFailureLinks(TrieNode* root) {
    queue<TrieNode*> q;
    root->fail = root;
    for (auto& [ch, child] : root->children) {
        child->fail = root;
        q.push(child);
    }

    while (!q.empty()) {
        TrieNode* current = q.front();
        q.pop();
        for (auto& [ch, child] : current->children) {
            TrieNode* fail = current->fail;
            while (fail != root && fail->children.find(ch) == fail->children.end()) {
                fail = fail->fail;
            }
            if (fail->children.find(ch) != fail->children.end()) {
                child->fail = fail->children[ch];
            } else {
                child->fail = root;
            }
            child->output.insert(child->output.end(),
                                 child->fail->output.begin(),
                                 child->fail->output.end());
            q.push(child);
        }
    }
}

// generate all substrings of the given string up to length n, longest to shortest
vector<pair<string, int>> generateSubstrings(const string& str, int maxLength) {
    vector<pair<string, int>> substrings;
    int len = str.length();
    for (int length = maxLength; length > 0; --length) {
        for (int start = 0; start <= len - length; ++start) {
            substrings.push_back({str.substr(start, length), start});
        }
    }
    return substrings;
}

// search for the longest patterns in the text and remove them on match
bool searchAndRemoveLongestMatch(TrieNode* root, string& text, string& patternText) {
    TrieNode* node = root;
    int longestMatchLength = 0;
    size_t matchStartIndex = string::npos;
    string longestMatch;
    int str1MatchIndex = -1;

    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        while (node != root && node->children.find(c) == node->children.end()) {
            node = node->fail;
        }
        if (node->children.find(c) != node->children.end()) {
            node = node->children[c];
        }

        if (!node->output.empty()) {
            for (const auto& [match, index] : node->output) {
                size_t startIndex = i - match.length() + 1;
                if (match.length() > longestMatchLength) {
                    longestMatchLength = match.length();
                    matchStartIndex = startIndex;
                    longestMatch = match;
                    str1MatchIndex = index;
                }
            }
        }
    }

    // If a match is found, remove it from both strings
    if (matchStartIndex != string::npos) {
        size_t patternIndex = patternText.find(longestMatch);
        if (patternIndex != string::npos) {
            cout << "Pattern \"" << longestMatch << "\" found at index " 
                 << matchStartIndex << " in the second string, "
                 << "and at index " << str1MatchIndex << " in the first string." << endl;

            // Remove the matched substring from both strings
            text.erase(matchStartIndex, longestMatchLength);
            patternText.erase(patternIndex, longestMatchLength);

            return true;
        }
    }
    return false;
}

// delete the Trie and free memory
void deleteTrie(TrieNode* node) {
    for (auto& [ch, child] : node->children) {
        deleteTrie(child);
    }
    delete node;
}

int main() {
    string str1 = "ATCGTACGTA";
    string str2 = "CGTACGTGCG";
    int maxLength = 6;

    while (true) {
        // Generate all substrings of str1 up to maxLength along with their start indices
        vector<pair<string, int>> substrings = generateSubstrings(str1, maxLength);

        // Create the trie root
        TrieNode* root = new TrieNode();
        
        // Insert substrings into the trie
        for (const auto& [substring, index] : substrings) {
            insertPattern(root, substring, index);
        }

        // Build the failure links for the Aho-Corasick automaton
        buildFailureLinks(root);

        // Search for substrings in str2, removing matches as they are found
        if (!searchAndRemoveLongestMatch(root, str2, str1)) {
            deleteTrie(root);
            break; // No more matches found
        }

        // Free memory after each iteration
        deleteTrie(root);
    }

    return 0;
}
