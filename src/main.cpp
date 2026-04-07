#include <bits/stdc++.h>
using namespace std;

struct Var {
    enum Type { INT, STRING } type;
    long long ival = 0;
    string sval;
};

static inline void fast_io() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
}

// Tokenize a line: split by spaces, but keep quoted strings (double quotes) as one token, including quotes
static inline vector<string> tokenize(const string &s) {
    vector<string> tokens;
    tokens.reserve(4);
    const size_t n = s.size();
    size_t i = 0;
    while (i < n) {
        // skip spaces
        while (i < n && isspace(static_cast<unsigned char>(s[i]))) ++i;
        if (i >= n) break;
        if (s[i] == '"') {
            // quoted token
            size_t j = i + 1;
            while (j < n && s[j] != '"') ++j;
            // token from i..j inclusive if closing quote exists
            if (j >= n) {
                // No closing quote -> take till end (will be invalid later)
                tokens.emplace_back(s.substr(i));
                break;
            } else {
                tokens.emplace_back(s.substr(i, j - i + 1));
                i = j + 1;
            }
        } else {
            size_t j = i;
            while (j < n && !isspace(static_cast<unsigned char>(s[j]))) ++j;
            tokens.emplace_back(s.substr(i, j - i));
            i = j;
        }
    }
    return tokens;
}

struct ScopeMachine {
    vector<unordered_map<string, Var>> scopes;

    ScopeMachine() { scopes.emplace_back(); }

    void indent() { scopes.emplace_back(); }

    bool dedent() {
        if (scopes.size() <= 1) return false; // cannot pop global
        scopes.pop_back();
        return true;
    }

    // find variable from innermost to outermost; return pointer and index
    pair<Var*, int> findVar(const string &name) {
        for (int i = (int)scopes.size() - 1; i >= 0; --i) {
            auto it = scopes[i].find(name);
            if (it != scopes[i].end()) return { &it->second, i };
        }
        return { nullptr, -1 };
    }

    bool existsInCurrent(const string &name) {
        return scopes.back().find(name) != scopes.back().end();
    }

    static bool parseIntLiteral(const string &tok, long long &out) {
        if (tok.empty()) return false;
        size_t idx = 0;
        // allow leading + or -
        if (tok[idx] == '+' || tok[idx] == '-') {
            if (tok.size() == 1) return false;
            idx++;
        }
        for (size_t i = idx; i < tok.size(); ++i) if (!isdigit(static_cast<unsigned char>(tok[i]))) return false;
        try {
            out = stoll(tok);
        } catch (...) {
            return false;
        }
        return true;
    }

    static bool parseStringLiteral(const string &tok, string &out) {
        if (tok.size() < 2) return false;
        if (tok.front() != '"' || tok.back() != '"') return false;
        // no escapes per spec
        out = tok.substr(1, tok.size() - 2);
        return true;
    }

    bool declare(const string &typeStr, const string &name, const string &valTok) {
        Var v;
        if (typeStr == "int") {
            long long x;
            if (!parseIntLiteral(valTok, x)) return false;
            v.type = Var::INT; v.ival = x; v.sval.clear();
        } else if (typeStr == "string") {
            string s;
            if (!parseStringLiteral(valTok, s)) return false;
            v.type = Var::STRING; v.sval = std::move(s); v.ival = 0;
        } else {
            return false;
        }
        if (existsInCurrent(name)) return false; // cannot redeclare in same scope
        scopes.back().emplace(name, std::move(v));
        return true;
    }

    bool addOp(const string &resName, const string &v1Name, const string &v2Name) {
        auto [vres, ires] = findVar(resName);
        auto [v1, i1] = findVar(v1Name);
        auto [v2, i2] = findVar(v2Name);
        if (!vres || !v1 || !v2) return false;
        if (vres->type != v1->type || v1->type != v2->type) return false;
        if (vres->type == Var::INT) {
            vres->ival = v1->ival + v2->ival;
        } else {
            vres->sval = v1->sval + v2->sval;
        }
        return true;
    }

    bool selfAdd(const string &name, const string &valTok) {
        auto [v, idx] = findVar(name);
        if (!v) return false;
        if (v->type == Var::INT) {
            long long x;
            if (!parseIntLiteral(valTok, x)) return false;
            v->ival += x;
        } else {
            string s;
            if (!parseStringLiteral(valTok, s)) return false;
            v->sval += s;
        }
        return true;
    }

    bool printVar(const string &name) {
        auto [v, idx] = findVar(name);
        if (!v) return false;
        if (v->type == Var::INT) {
            cout << name << ":" << v->ival << '\n';
        } else {
            cout << name << ":" << v->sval << '\n';
        }
        return true;
    }
};

int main() {
    fast_io();
    int n;
    if (!(cin >> n)) return 0;
    string dummy;
    getline(cin, dummy); // consume rest of line

    ScopeMachine mach;
    string line;
    for (int i = 0; i < n; ++i) {
        if (!std::getline(cin, line)) break;
        auto tokens = tokenize(line);
        if (tokens.empty()) { cout << "Invalid operation\n"; continue; }
        const string &cmd = tokens[0];
        bool ok = true;
        if (cmd == "Indent") {
            if (tokens.size() != 1) ok = false; else mach.indent();
        } else if (cmd == "Dedent") {
            if (tokens.size() != 1) ok = mach.dedent(); else ok = mach.dedent();
        } else if (cmd == "Declare") {
            if (tokens.size() != 4) ok = false; else ok = mach.declare(tokens[1], tokens[2], tokens[3]);
        } else if (cmd == "Add") {
            if (tokens.size() != 4) ok = false; else ok = mach.addOp(tokens[1], tokens[2], tokens[3]);
        } else if (cmd == "SelfAdd") {
            if (tokens.size() != 3) ok = false; else ok = mach.selfAdd(tokens[1], tokens[2]);
        } else if (cmd == "Print") {
            if (tokens.size() != 2) ok = false; else ok = mach.printVar(tokens[1]);
        } else {
            ok = false;
        }
        if (!ok) cout << "Invalid operation\n";
    }
    return 0;
}
