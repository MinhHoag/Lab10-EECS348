#include <bits/stdc++.h>
using namespace std;

//  Validation  //
// A valid “double” per the lab:
//   [ + | - ] DIGITS [ . DIGITS ]
// If '.' appears, there must be >=1 digit on both sides.
bool isValidDouble(const string& s) {
    if (s.empty()) return false;

    size_t i = 0;
    if (s[i] == '+' || s[i] == '-') {
        if (s.size() == 1) return false; // sign alone
        i++;
    }

    // at least one digit before optional '.'
    size_t startDigits = i;
    while (i < s.size() && isdigit(static_cast<unsigned char>(s[i]))) i++;
    size_t intDigits = i - startDigits;
    if (intDigits == 0) return false;   // require digits before '.'

    if (i == s.size()) {
        // No decimal point; e.g., "123", "+000"
        return true;
    }

    if (s[i] != '.') return false;      // invalid character
    i++;                                // skip '.'

    // require at least one digit after '.'
    size_t startFrac = i;
    while (i < s.size() && isdigit(static_cast<unsigned char>(s[i]))) i++;
    size_t fracDigits = i - startFrac;
    if (fracDigits == 0) return false;  // e.g., "5." is invalid

    return i == s.size();               // no extra junk at end
}

//  Representation  //
struct Num {
    int sign;         // +1 or -1
    string I;         // integer part, no sign, no leading zeros except "0"
    string F;         // fractional part, no trailing zeros after normalization (may be empty)
};

static string lstripZeros(const string& t) {
    size_t j = 0;
    while (j < t.size() && t[j] == '0') j++;
    return (j == t.size()) ? string("0") : t.substr(j);
}
static string rstripZeros(const string& t) {
    if (t.empty()) return t;
    size_t j = t.size();
    while (j > 0 && t[j-1] == '0') j--;
    return t.substr(0, j);
}

static void padLeft(string& a, size_t n) {
    if (a.size() < n) a.insert(a.begin(), n - a.size(), '0');
}
static void padRight(string& a, size_t n) {
    if (a.size() < n) a.append(n - a.size(), '0');
}

static Num parseToNum(const string& s) {
    // precondition: isValidDouble(s) == true
    size_t i = 0;
    int sign = +1;
    if (s[i] == '+') { sign = +1; i++; }
    else if (s[i] == '-') { sign = -1; i++; }

    // split at '.'
    size_t dot = s.find('.', i);
    string I, F;
    if (dot == string::npos) {
        I = s.substr(i);
        F = "";
    } else {
        I = s.substr(i, dot - i);
        F = s.substr(dot + 1);
    }

    // normalize integer part (remove leading zeros)
    I = lstripZeros(I);

    // fractional: keep as-is for now; arithmetic will pad; result will be trimmed
    return {sign, I, F};
}

// Comparisons of |a| vs |b| //
static int cmpAbs(const Num& a, const Num& b) {
    // Compare integer part lengths
    if (a.I.size() != b.I.size()) return (a.I.size() < b.I.size()) ? -1 : 1;

    // Compare integer lexicographically
    if (a.I != b.I) return (a.I < b.I) ? -1 : 1;

    // Compare fractional by padding to same length
    string af = a.F, bf = b.F;
    size_t L = max(af.size(), bf.size());
    padRight(af, L); padRight(bf, L);
    if (af != bf) return (af < bf) ? -1 : 1;

    return 0;
}

// |a| + |b| where both signs are same //
static Num addAbsSameSign(Num a, const Num& b) {
    // pad fractional to same length
    size_t Lf = max(a.F.size(), b.F.size());
    padRight(a.F, Lf);
    string F = a.F;
    string G = b.F;
    padRight(F, Lf); padRight(G, Lf);

    // add fractional right-to-left
    string frac(Lf, '0');
    int carry = 0;
    for (int k = static_cast<int>(Lf) - 1; k >= 0; --k) {
        int d = (F[k]-'0') + (G[k]-'0') + carry;
        frac[k] = char('0' + (d % 10));
        carry = d / 10;
    }

    // add integer right-to-left
    string A = a.I;
    string B = b.I;
    size_t Li = max(A.size(), B.size());
    padLeft(A, Li); padLeft(B, Li);

    string integ(Li, '0');
    for (int k = static_cast<int>(Li) - 1; k >= 0; --k) {
        int d = (A[k]-'0') + (B[k]-'0') + carry;
        integ[k] = char('0' + (d % 10));
        carry = d / 10;
    }
    if (carry) integ.insert(integ.begin(), char('0' + carry));

    // normalize output
    integ = lstripZeros(integ);
    frac = rstripZeros(frac);

    return {a.sign, integ, frac};
}

// |a| - |b| assuming |a| >= |b| //
static Num subAbsAssumingAGeB(Num A, const Num& B) {
    // Align fractional part
    size_t Lf = max(A.F.size(), B.F.size());
    padRight(A.F, Lf);
    string BF = B.F; padRight(BF, Lf);

    // Subtract fractional: A.F - BF
    string frac(Lf, '0');
    int borrow = 0;
    for (int k = static_cast<int>(Lf) - 1; k >= 0; --k) {
        int d = (A.F[k]-'0') - (BF[k]-'0') - borrow;
        if (d < 0) { d += 10; borrow = 1; } else { borrow = 0; }
        frac[k] = char('0' + d);
    }

    // Subtract integer with borrow
    string AI = A.I, BI = B.I;
    size_t Li = max(AI.size(), BI.size());
    padLeft(AI, Li); padLeft(BI, Li);

    string integ(Li, '0');
    for (int k = static_cast<int>(Li) - 1; k >= 0; --k) {
        int d = (AI[k]-'0') - (BI[k]-'0') - borrow;
        if (d < 0) { d += 10; borrow = 1; } else { borrow = 0; }
        integ[k] = char('0' + d);
    }

    // remove leading zeros and trailing frac zeros
    integ = lstripZeros(integ);
    frac = rstripZeros(frac);

    // If result is zero (both integ == "0" and frac empty), enforce + sign and "0"
    if (integ == "0" && frac.empty()) return {+1, string("0"), string()};

    return {+1, integ, frac}; // caller decides sign
}

// Addition with signs //
static string toString(const Num& n) {
    string out = n.I;
    if (!n.F.empty()) {
        out.push_back('.');
        out += n.F;
    }
    if (out == "0") return "0";
    if (n.sign < 0) return "-" + out;
    return out;
}

static string addStringsAsDoubles(const string& s1, const string& s2) {
    // Precondition: both valid per isValidDouble
    Num a = parseToNum(s1);
    Num b = parseToNum(s2);

    // If both zero after normalization, short-circuit
    if (a.I == "0" && a.F.empty()) a.sign = +1;
    if (b.I == "0" && b.F.empty()) b.sign = +1;

    if (a.sign == b.sign) {
        Num r = addAbsSameSign(a, b);
        r.sign = a.sign; // same sign
        return toString(r);
    } else {
        // Compare magnitudes
        int c = cmpAbs(a, b);
        if (c == 0) return "0";
        if (c > 0) {
            Num r = subAbsAssumingAGeB(a, b);
            r.sign = a.sign;
            return toString(r);
        } else {
            Num r = subAbsAssumingAGeB(b, a);
            r.sign = b.sign;
            return toString(r);
        }
    }
}

// I/O driver //
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "Enter path to test file: ";
    string path;
    if (!getline(cin, path) || path.empty()) {
        cerr << "No file provided.\n";
        return 1;
    }

    ifstream fin(path);
    if (!fin) {
        cerr << "Failed to open: " << path << "\n";
        return 1;
    }

    string line;
    int caseNo = 1;
    while (getline(fin, line)) {
        // skip empty or comment lines
        string trimmed = line;
        // trim spaces
        auto notspace = [](int ch){ return !isspace(ch); };
        trimmed.erase(trimmed.begin(), find_if(trimmed.begin(), trimmed.end(), notspace));
        while (!trimmed.empty() && isspace(trimmed.back())) trimmed.pop_back();
        if (trimmed.empty() || trimmed[0] == '#') continue;

        istringstream iss(line);
        string a, b;
        if (!(iss >> a >> b)) {
            cout << "Case " << caseNo++ << ": Invalid line format -> \"" << line << "\"\n";
            continue;
        }

        bool va = isValidDouble(a);
        bool vb = isValidDouble(b);

        if (!va || !vb) {
            cout << "Case " << caseNo++ << ": invalid number(s): "
                 << (va ? "" : ("[" + a + "] ")) << (vb ? "" : ("[" + b + "]"))
                 << "\n";
            continue;
        }

        string sum = addStringsAsDoubles(a, b);
        cout << "Case " << caseNo++ << ": " << a << " + " << b << " = " << sum << "\n";
    }

    return 0;
}
