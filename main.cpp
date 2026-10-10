// CSC316 - Homework 2: Vigenere Cipher
// Charbel Chidiac
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) throw runtime_error("Missing input.");
    return line;
}

// An order such as 3 1 2 means: take the third, first, then second item.
vector<int> readOrder(const string& prompt) {
    istringstream input(readLine(prompt));
    vector<int> order;
    int value;
    while (input >> ws && !input.eof()) {
        if (!(input >> value))
            throw runtime_error("Enter integer positions only.");
        order.push_back(value);
    }
    if (order.empty())
        throw runtime_error("Enter a permutation of 1 through N.");
    vector<int> sorted = order;
    sort(sorted.begin(), sorted.end());
    for (size_t i = 0; i < sorted.size(); ++i)
        if (sorted[i] != static_cast<int>(i + 1))
            throw runtime_error("Order must contain 1 through N exactly once.");
    for (int& item : order) --item; // Use zero-based positions internally.
    return order;
}

// Shift English letters only; keep their case and leave other characters intact.
string vigenere(string text, const string& key, bool decrypt) {
    size_t keyIndex = 0;
    for (char& ch : text) {
        char base;
        if (ch >= 'A' && ch <= 'Z') base = 'A';
        else if (ch >= 'a' && ch <= 'z') base = 'a';
        else continue;
        int shift = key[keyIndex % key.size()] - 'A';
        if (decrypt) shift = -shift;
        ch = static_cast<char>(base + (ch - base + shift + 26) % 26);
        ++keyIndex;
    }
    return text;
}

// Fill each grid left to right, then read it in the selected row/column order.
// Missing cells in the final block are skipped, so no padding is necessary.
string permute(const string& text, const vector<int>& rowOrder,
               const vector<int>& colOrder, char round, bool decrypt) {
    string result = text;
    size_t rows = rowOrder.size(), cols = colOrder.size();
    size_t blockSize = rows * cols;
    for (size_t start = 0; start < text.size(); start += blockSize) {
        size_t count = min(blockSize, text.size() - start);
        size_t next = 0;
        for (size_t r = 0; r < rows; ++r) {
            for (size_t c = 0; c < cols; ++c) {
                size_t sourceRow = round == 'R' ? rowOrder[r] : r;
                size_t sourceCol = round == 'C' ? colOrder[c] : c;
                size_t source = sourceRow * cols + sourceCol;
                if (source >= count) continue;
                if (decrypt) result[start + source] = text[start + next];
                else result[start + next] = text[start + source];
                ++next;
            }
        }
    }
    return result;
}

int main() {
    try {
        cout << "Vigenere Cipher with Row/Column Permutations\n";
        string mode = readLine("Mode (E = Encrypt, D = Decrypt): ");
        if (mode != "E" && mode != "e" && mode != "D" && mode != "d")
            throw runtime_error("Choose E or D.");
        bool decrypt = mode == "D" || mode == "d";
        string text = readLine(decrypt ? "Ciphertext (include outer quotes): "
                                       : "Plaintext: ");
        if (decrypt) {
            // quoted() reverses the escaping printed by the encryption output.
            istringstream input(text);
            if (text.empty() || text.front() != '"' || !(input >> quoted(text)))
                throw runtime_error("Paste the complete quoted ciphertext.");
            input >> ws;
            if (!input.eof()) throw runtime_error("Unexpected text after ciphertext.");
        }
        string key = readLine("Secret keyword (letters only): ");
        if (key.empty()) throw runtime_error("Keyword cannot be empty.");
        for (char& ch : key) {
            if (ch >= 'a' && ch <= 'z') ch = static_cast<char>(ch - 'a' + 'A');
            if (ch < 'A' || ch > 'Z')
                throw runtime_error("Keyword must contain English letters only.");
        }
        vector<int> rowOrder = readOrder("Row order (example: 2 1): ");
        vector<int> colOrder = readOrder("Column order (example: 3 1 2): ");
        string rounds = readLine("Encryption round order (example: CR or CRC): ");
        if (rounds.empty()) throw runtime_error("Enter at least one round.");
        for (char& round : rounds) {
            if (round == 'c') round = 'C';
            if (round == 'r') round = 'R';
            if (round != 'C' && round != 'R')
                throw runtime_error("Use only C (columns) and R (rows).");
        }

        if (!decrypt) {
            text = vigenere(text, key, false);
            cout << "After Vigenere: " << quoted(text) << '\n';
            for (char round : rounds) {
                text = permute(text, rowOrder, colOrder, round, false);
                cout << "After " << round << " round: " << quoted(text) << '\n';
            }
            cout << "Final ciphertext: " << quoted(text) << '\n';
        } else {
            // Undo the last permutation first, then undo Vigenere substitution.
            for (auto it = rounds.rbegin(); it != rounds.rend(); ++it) {
                text = permute(text, rowOrder, colOrder, *it, true);
                cout << "After undoing " << *it << ": " << quoted(text) << '\n';
            }
            text = vigenere(text, key, true);
            cout << "Recovered plaintext: " << quoted(text) << '\n';
        }
    } catch (const exception& error) {
        cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
