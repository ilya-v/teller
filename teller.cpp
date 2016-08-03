#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <vector>
#include <algorithm>
#include <random>

const std::string pattern_chars { "0123456789?#$. " };
bool is_pattern_char(const char c) { return pattern_chars.find_first_of(c) != std::string::npos; }

std::string concat(const std::string & a, const std::string b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    const std::string space = (::isspace(a.back()) || ::isspace(b.front()))? "" : " ";
    return a + space + b;
}

struct string_view_t
{
    using rit = std::string::const_reverse_iterator;
    static constexpr size_t npos = std::string::npos;

    std::string       & str;
    size_t              pos_begin = 0;
    size_t              pos_end = std::string::npos;

    string_view_t(std::string &s, const size_t begin = 0, const size_t end = npos)
            : str(s), pos_begin(begin), pos_end(end == npos? s.size() : end) {}
    string_view_t(const string_view_t & sv) = default;

    rit         crbegin()       const { return str.crbegin() + (str.size() - pos_end); }
    rit         crend()         const { return str.crend() - pos_begin; }
    bool        right_aligned() const { return pos_end == str.size(); }
    bool        leftt_aligned() const { return pos_begin == 0; }
    size_t      size()          const { return (size_t)(pos_end - pos_begin); }
    operator    bool()          const { return size() > 0; }

    string_view_t & set_begin(const rit & i) { pos_begin = (size_t)std::distance(i, str.crend()); return *this; }
    void fill(const char c) const { std::fill(str.begin() + pos_begin, str.end() + pos_end, '?'); }
};


struct rule_t
{
    std::string     pattern;            // the pattern to match
    std::string     replacement;        // the words to replace the pattern

    rule_t(const std::string & rule_str) {
        for (const char c: rule_str)
            ( (replacement.empty() && is_pattern_char(c)) ? pattern : replacement).push_back(c);
    }

    string_view_t match(const string_view_t & numview) const {
        string_view_t empty_result {numview.str, numview.pos_end, numview.pos_end};
        if (   (is_terminal_rule() && !numview.right_aligned())
            || (is_start_rule()    && !numview.leftt_aligned())  )
            return empty_result;

        auto inumber = numview.crbegin();
        for (auto ipattern = pattern.crbegin(); ipattern != pattern.crend(); ++ipattern) {
            if (::isspace(*ipattern) || *ipattern == '.' || *ipattern == '$') continue;
            if (inumber == numview.crend() || (*ipattern == '#' && *inumber == '?'))  return empty_result;
            if (*ipattern == '#') break;
            if (*inumber != *ipattern) return empty_result;
            ++inumber;
        }
        if (is_start_rule() && inumber != numview.crend()) return empty_result;
        return string_view_t{empty_result}.set_begin(inumber);
    }

    size_t  n_hashes()          const { return (size_t) std::count(pattern.begin(), pattern.end(), '#'); }
    bool    is_terminal_rule()  const { return pattern.find_first_of('.') != std::string::npos; }
    bool    is_start_rule()     const { return pattern.find_first_of('$') != std::string::npos;}
};


std::string apply_rules(const std::vector<rule_t> & rules, const string_view_t &numview) {

    std::string result = "";

    for (std::string old_number; numview.str != old_number && numview.size(); ) {
        old_number = numview.str;
        for (const auto &rule : rules) {
            if (const string_view_t matched_view = rule.match(numview)) {
                matched_view.fill('?');
                result = concat(rule.replacement, result);
                if (const auto n_hashes = rule.n_hashes()) {
                    const size_t pos_begin = (matched_view.pos_begin > n_hashes)? matched_view.pos_begin - n_hashes : 0;
                    result = concat(apply_rules(rules, string_view_t{numview.str, pos_begin, matched_view.pos_begin}), result);
                }
                break;
            }
        }
    }
    return result;
}

std::vector<rule_t> read_rules(const std::string &rules_fname) {
    std::vector<rule_t> rules;
    std::ifstream rules_file{rules_fname.c_str()};
    for (std::string line; std::getline(rules_file, line); ) {
        const rule_t rule {line};
        if (!rule.pattern.empty())
            rules.push_back(rule);
    }
    return rules;
}

void tell(const std::vector<rule_t> & rules, std::string number) {
    std::cout << number << " --> ";
    std::string result = apply_rules(rules, string_view_t{number});
    number.erase( std::remove(number.begin(), number.end(), '?'), number.end());
    std::cout << number << (number.empty()?"":" ") << result << std::endl;
}

int main(const int argc, const char *argv[]) {

    const std::string rules_file = (argc >1)? argv[1] : "rules.txt";

    const std::vector<rule_t> rules = read_rules(rules_file);
    if (rules.empty()) {
        std::cerr << "No rules read freom " << rules_file << std::endl;
        return 1;
    }

    for (unsigned n = 0; n < 10000; n++)
        tell(rules, std::to_string(n));
}
