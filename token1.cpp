// type for set
#include <set>
// type for file stream and streams
#include <fstream>
// type for locale
#include <locale>
// type for codecvt_uft8
#include <codecvt>
// type for vector
#include <vector>
// type for string
#include <string>
// type for map
#include <map>
// 
#include <string.h>
//
#include <ctype.h>
// standard input output
#include <iostream>
// random numbers 
#include <random>

using namespace std;

typedef int uid;
typedef long long buf_size;

namespace {
    
// create random device
random_device random_device_variable;
// seed the engine
mt19937_64 mt19937_64_engine(random_device_variable());
// rande of number to generate
uniform_int_distribution<uid> uniform_int_distribution_variable{125, 32500*64};
// 
#define gen mt19937_64_engine
#define dist uniform_int_distribution_variable
#define rnd(X) uid X = dist(gen);


// define token types
rnd(WORD)
rnd(SPACE)
rnd(INTEGER)
rnd(FLAG)
rnd(KEYWORD)
rnd(BOOL)
rnd(PRIMARY)
rnd(DOUBLE)
rnd(FLOAT)
rnd(LONG)
rnd(SPECIAL)
rnd(MATH)
rnd(COMMENT)
rnd(STRING)
rnd(BR_open)
rnd(BR_close)
rnd(CHARACTER)
//
#undef rnd
#define rnd(X) uid X = -dist(gen);
//
rnd(NOT_EXPECTED)
rnd(MISSING_TERMINATING_ELEMENT)
rnd(INVALID_ESCAPE_ELEMENT)
rnd(EMPTY_CHAR_LITERAL)
rnd(UNCLOSING_CHAR_LITERAL)

// buffer structure
template <class st = istream>
class buffer
{
    public:
    // an stream
    st * fin;
    // an name of the buffer
    char * name;
    // an map with line length;
    vector<buf_size> map_line;
    // line, begin's with 1
    buf_size line = 1;
    // position in line, begin's with 0
    buf_size column = 0;
    // initialize buffer
    void open(st * f, char * name){
        this->fin = f;
        this->name = name;
    }
    buffer(st * f, char * name){
        open(f, name);
    }
    // get symbol
    int get(){
        // get symbol from fin
        int i = fin->get();
        // if i is equals to '\n', then add line
        if (i == L'\n')
        {
            // add line length to line position
            this->map_line.push_back(this->column + 1);
            // set position of new line equals to 0
            this->column = 0;
            this->line ++;
        }
        // if i greater that -1, then displace position
        else if (i > -1)
        {
            // displace position
            this->column++;
        }
        return i;
    }
    // jump to other position
    buf_size back(buf_size i){
        // seek
        this->fin->seekg((long long)this->fin->tellg() - i);
        // repeat until i > position
        while (i > this->column)
        {
            // decrease i pointer
            i -= this->column;
            i --;
            // decrease line position
            this->line --;
            // if line pointer is equals to 0, then reset
            if (this->line == 0)
            {
                this->column = 0;
                this->line = 1;
                return 0;
            }
            // set position equals to last element in vector 
            this->column = this->map_line.back();
            this->map_line.pop_back();
        }
        this->column -= i;
        
        return this->fin->tellg();
    }
};

typedef buffer<wifstream> wbuffer;
// token info
struct TokenInfo{
    buf_size line;
    buf_size column;
    char * source;
 //   TokenInfo(int line, int column, char * source){
 //       this->line = line;
 //       this->column = column;
 //       this->source = source;
 //   }
};
// token structure
struct Token{
    void * value;
    uid type;
    TokenInfo info;
    void set(void * value, uid type, TokenInfo info){
        this->value = value;
        this->type = type;
        this->info = info;
    }
    Token(void * value, uid type, TokenInfo info){
        set(value, type, info);
    }
    
    Token(wstring str, uid type, TokenInfo info){
        // get size
        size_t s = str.size();
        // allocate memory
        wchar_t * value = new wchar_t[s+1];
        // copy memory
        memcpy(value, str.c_str(), str.size()*(sizeof(wchar_t)));
        value[s] = 0;
        // set pointer
        set(value, type, info);
    }
    Token(long long number, uid type, TokenInfo info){
        long long * value = new long long(number);
        set(value, type, info);
    }
    Token(wchar_t number, uid type, TokenInfo info){
        wchar_t * value = new wchar_t(number);
        set(value, type, info);
    }
    Token(long double number, uid type, TokenInfo info){
        long double * value = new long double(number);
        set(value, type, info);
    }
    Token(uid special, uid type, TokenInfo info){
        uid * value = new uid[1];
        *value = special;
        /// 
        set(value, type, info);
    }
};

//
map<uid, wstring> uid_string_map;

// pass the _ symbols
int get_u(wbuffer & f){
    int i;
    do {
        i = f.get();
    } while (i == '_');
    return i;
}

// get decimal number
int get_dec_num(wbuffer&f,long long&big){
    //
    int u = get_u(f);
    // 
    while (isdigit(u)){
        //
        big *= 10; big += u - '0';
        //
        u = get_u(f);
    }
    return u;
}

// get binary number
int get_bin_num(wbuffer&f,long long&big){
    //
    int u = get_u(f);
    // 
    back:
        big *= 2;
        switch (u){
            case '1':
               big++;
            case '0':
               u = get_u(f);
               goto back;
            default:
               break;
        };
        //
    return u;
}

// get decimal number 
int get_dec_num(wbuffer&f,long long&big,long long&del){
    //
    int u = get_u(f);
    // 
    while (isdigit(u)){
        //
        big *= 10; big += u - '0'; del *= 10;
        //
        u = get_u(f);
    }
    return u;
}

// get hexadecimal number
int get_hex_num(wbuffer&f,long long&big,long long&del){
    //
    int u = get_u(f);
    // 
    while (isxdigit(u)){
        //
        u = tolower(u);
        //
        big *= 16; del *= 16;
        //
        big += (iswalnum(u)) ? '0' - u : 'a' - u + 10;
        //
        u = get_u(f);
    }
    return u;
}

// get hexadecimal number
int get_hex_num(wbuffer&f,long long&big){
    //
    int u = get_u(f);
    // 
    while (isxdigit(u)){
        //
        u = tolower(u);
        //
        big *= 16; 
        //
        big += (iswalnum(u)) ? '0' - u : 'a' - u + 10;
        //
        u = get_u(f);
    }
    return u;
}

// radix structure
struct Radix{
    map<uid, Radix*> tree;
};

#undef rnd
#define rnd dist(gen)

// this method will create an radix structure and fill data 
Radix * create_radix(vector<vector<wstring>> ch2, 
      vector<uid> st, map<wstring, uid*> &ch_tree){
    // 
    Radix * rad = new Radix;
    Radix * temp;
    Radix * temp2;
    //
     
     map<uid, Radix*>::iterator it;
     //
     int t = 0;
     uid td;
     uid * d;
     //
    for (vector<wstring> & ch: ch2){
     uid type = st[t];
     t++;
     for (const wstring & ws: ch){
         //
         temp = rad;
         //
          d = new uid[2];
          d[0] = td = rnd;
          d[1] = type;
          //
          uid_string_map[td] = ws;
         //
         ch_tree[ws] = d;
         //
         for (const wchar_t &t: ws){
             //
             it = temp->tree.find(t);
             //
             if (it != temp->tree.end()){
                 temp = it->second;
             }
             //
             else{
                 //
                 temp2 = new Radix();
                 //
                 temp->tree[t] = temp2;
             }
             //
         }
     }
    }
     //
     return rad;
};
// 
map<wstring, uid*> keyword_tree;
//

inline uid uid_string_map_add(wstring str, uid type){
    //
    uid *ret = new uid[2];
    //
    ret[1] = type;
    type = rnd;
    ret[0] = type;
    //
    uid_string_map[type] = str;
    //
    keyword_tree[str] = ret;
    //
    return type;
}

#define stv(X, Y, Z) uid _##X = uid_string_map_add(Y, Z);
//
namespace keyword{
    stv(abstract, L"abstract", KEYWORD)
    stv(continue, L"continue", KEYWORD)
    stv(for, L"for", KEYWORD)
    stv(new, L"new", KEYWORD)
    stv(switch, L"switch", KEYWORD)
    stv(assert, L"assert", KEYWORD)
    stv(default, L"default", KEYWORD)
    stv(goto, L"goto", KEYWORD)
    stv(package, L"package", KEYWORD)
    stv(synchronized, L"synchronized", KEYWORD)
    stv(do, L"do", KEYWORD)
    stv(if, L"if", KEYWORD)
    stv(private, L"private", KEYWORD)
    stv(this, L"this", KEYWORD)
    stv(break, L"break", KEYWORD)
    stv(implements, L"implements", KEYWORD)
    stv(protected, L"protected", KEYWORD)
    stv(throw, L"throw", KEYWORD)
    stv(else, L"else", KEYWORD)
    stv(import, L"import", KEYWORD)
    stv(public, L"public", KEYWORD)
    stv(throws, L"throws", KEYWORD)
    stv(case, L"case", KEYWORD)
    stv(enum, L"enum", KEYWORD)
    stv(instanceof, L"instanceof", KEYWORD)
    stv(return, L"return", KEYWORD)
    stv(transient, L"transient", KEYWORD)
    stv(catch, L"catch", KEYWORD)
    stv(extends, L"extends", KEYWORD)
    stv(try, L"try", KEYWORD)
    stv(finally, L"finally", KEYWORD)
    stv(interface, L"interface", KEYWORD)
    stv(static, L"static", KEYWORD)
    stv(var, L"var", KEYWORD)
    stv(class, L"class", KEYWORD)
    stv(final, L"final", KEYWORD)
    stv(strictfp, L"strictfp", KEYWORD)
    stv(void, L"void", PRIMARY)
    stv(const, L"const", KEYWORD)
    stv(native, L"native", KEYWORD)
    stv(super, L"super", KEYWORD)
    stv(volatile, L"volatile", KEYWORD)
    stv(while, L"while", KEYWORD)
    
    stv(boolean, L"boolean", PRIMARY)
    stv(float, L"float", PRIMARY)
    stv(int, L"int", PRIMARY)
    stv(short, L"short", PRIMARY)
    stv(long, L"long", PRIMARY)
    stv(byte, L"byte",PRIMARY)
    stv(double, L"double", PRIMARY)
    stv(char, L"char", PRIMARY)
    
    stv(true, L"true", BOOL)
    stv(false, L"false", BOOL)
    
}
//
map<wstring, uid*> sym_tree;
//
Radix * sym_radix = create_radix(
vector<vector<wstring>>{
{L"||", L"&&", L"|", 
L"~", L"!", L";",
L"^", L"--", L"++",
L"&", L"==", L"!=",
L"<", L"<=", L">", 
L">=", L"<<", L">>", 
L">>>", L"+", L"-", 
L"%", L"*", L"/", 
L"%=",L"&=",L"*=",
L"+=",L"-=",L"/=", 
L"<<=",L"=",L">>=",
L">>>=",L"^=",L"|="}, 
{L"?", L":", L"->", 
L".", L"::", L"...", 
L"@", L",", L";"}, 
{L"{", L"[", L"("},
{L"}", L"]", L")"},
{L"//", L"/*"},
{L"\"", L"\"\"", L"\"\"\""},
{L"'"}}, 

vector<uid>{MATH, SPECIAL, BR_open, BR_close, 
    COMMENT, STRING, CHARACTER}, sym_tree);
//

namespace sym{
    uid multiline_comment = sym_tree[L"//"][0];
    uid oneline_comment = sym_tree[L"/*"][0];
    uid block_string = sym_tree[L"\"\"\""][0];
    uid empty_string = sym_tree[L"\"\""][0];
    uid line_string = sym_tree[L"\""][0];
    uid character = sym_tree[L"'"][0];
}

map<int, int> char_escape_map = {
    {L'n', L'\n'},
    {L'r', L'\r'},
    {L'f', L'\f'},
    {L'\\', L'\\'},
    {L'\'', L'\''},
    {L'"', L'"'},
    {L't', L'\t'},
    {L'b', L'\b'},
};

template<typename a, typename b>
inline b get_or_default(map<a, b> m, a key, b def_){
    auto c = m.find(key);
    if (c == m.end()){
        return def_;
    }
    else{
        return c->second;
    }
}

int get_char_escape(wbuffer & f){
    //
    int u = f.get();
    //
    if (u == 'u'){
        //
        u = 0;
        //
        int k;
        //
        for (char i = 0; i < 4; i++){
            //
            k = f.get();
            //
            if (isxdigit(k)){
                //
                k = tolower(k);
                //
                u += (k > 'a') ? k-'a'+10 : k-'0';
            }
            else{
                return -1;
            }
        }
        return u;
    }
    else{
        //
        u = get_or_default(char_escape_map, u, u); 
        return u;
    }
}

inline Token * get_missing_error(wbuffer&f,wstring st,TokenInfo&info){
    //
    info.line = f.line;
    info.column = f.column;
    //
    return new Token(st, MISSING_TERMINATING_ELEMENT, info);
}

inline Token * get_escape_error(wbuffer&f,TokenInfo&info){
    //
    info.line = f.line;
    info.column = f.column;
    //
    return new Token((uid)-1, INVALID_ESCAPE_ELEMENT, info);
}

Token * get_string (uid s, wbuffer & f, TokenInfo & info){
    // 
    wstring t(L"");
    // 
    int u;
    //
    if (s == sym::empty_string){
        return new Token(t, STRING, info);
    }
    else if (s == sym::block_string){
        // 
        // get element
        u = f.get();
        // count of " elements 
        char y = 0;
        // repeat
        loop:{
            // check element
            while (u == '"'){
                y ++;
                // check if y is 3
                if (y == 3) return new Token(t, STRING, info);
                //
                u = f.get();
            }
            //
            while (y > 0){
                t += '"';
                y--;
            }
            // if got literal
            if (u == '\\'){
                //
                u = get_char_escape(f);
                //
                if (u == -1){
                    return get_escape_error(f, info);
                }
            }
            //
            if (u == -1){
                return get_missing_error(f, L"\"\"\"", info);
            }
            //
            t += (wchar_t) u;
            //
            u = f.get();
            //
            goto loop;
        }
    }
    else if (s == sym::line_string){
        //
        u = f.get();
        //
        while (u != '"'){
            if (u == '\n' || u == -1){
                //
                return get_missing_error(f, L"\"", info);
            }
            //
            if (u == '\\'){
                //
                u = get_char_escape(f);
                //
                if (u == -1){
                    return get_escape_error(f, info);
                }
            }
            // add element 
            t += (wchar_t) u;
            // get next element
            u = f.get();
        }
        //
        return new Token(t, STRING, info);
    }
    else{
        return NULL;
    }
}

Token * get_character (uid s, wbuffer &f, TokenInfo &info){
    //
    int u = f.get();
    int term;
    //
    if (u == '\''){
        //
        info.line = f.line; info.column = f.column;
        //
        return new Token((uid)-1, EMPTY_CHAR_LITERAL, info);
    }
    //
    if (u == '\\'){
        //
        u = get_char_escape(f);
        //
        if (u == -1){
            return get_escape_error(f, info);
        }
    }
    // get terminating element
    term = f.get();
    //
    if (term != '\''){
        if (term != -1) f.back(1);
        return get_missing_error(f, L"'", info);
    }
    //
    return new Token((wchar_t)u, CHARACTER, info);
}

Token * get_comment (uid s, wbuffer & f, TokenInfo & info){
    //
    wstring t(L"");
    //
    int i;
    //
    if (s == sym::multiline_comment){
        // get multiline comment
        do{
            i = f.get();
            //
            if (i == L'*'){
                i = f.get();
                //
                if (i == '/'){
                    return new Token(t, COMMENT, info);
                }
                t += L'*';
            }
            t += (wchar_t) i;
            //
        } while (i != -1);
        //
        info.line = f.line;
        info.column = f.column;
        //
        return new Token(L"*/", MISSING_TERMINATING_ELEMENT, info);
    }
    else if (s == sym::oneline_comment){
        //
        do{
            i = f.get();
            //
            if (i == '\n'){
                return new Token(t, COMMENT, info);
            }
            t += (wchar_t) i;
            //
        } while (i != -1);
        //
        return new Token(t, COMMENT, info);
    }
    else {
        return NULL;
    }
}

//
map<uid, Token * (*)(uid, wbuffer&, TokenInfo&)> type_map = {
    {COMMENT, get_comment},
    {STRING, get_string},
    {CHARACTER, get_character},
    {MATH, [](uid a, wbuffer&b, TokenInfo&c)
        {return new Token(a, MATH, c);}},
    {SPECIAL, [](uid a, wbuffer&b, TokenInfo&c)
        {return new Token(a, SPECIAL, c);}},
    {BR_open, [](uid a, wbuffer&b, TokenInfo&c)
        {return new Token(a, BR_open, c);}},
    {BR_close, [](uid a, wbuffer&b, TokenInfo&c)
        {return new Token(a, BR_close, c);}},
};

//set <wchar_t> BR_open_ar {L'{', L'[', L'('};
//set <wchar_t> BR_close_ar {L'}', L']', L')'};

//# get token ( an keyword )
Token * get_token(wbuffer & f){//wistream & f){
  //  # read 1 byte from stream
    TokenInfo info= {f.line, f.column, f.name};
  //
    int u = f.get();
  //  # if character is null
    if (u == -1) return new Token(L"", 0, info);
 //   # if is space
    else if (iswspace(u)){
        // getting space
        int t = 0;
        do{
            // repeat while space here
            t++;
            u = f.get();
        }
        while (iswspace(u));
        // go back if not end here
        if (u != -1) f.back(1);
        return new Token((long long)t, SPACE, info);
    }
 // if is identifier
    else if (iswalpha(u)||(u == L'_')||(u == L'$')){
        wstring t(L"");
        do{
            // repeat until space or symbol here
            t+= (wchar_t)u;
            u = f.get();
        }
        while (iswalpha(u)|| isdigit(u)||(u == L'_')||(u == L'$'));
        // if got symbol, then go to back
        if (u != -1) f.back(1);
        // if identifier is an keyword, then return keyword token
        auto r = keyword_tree.find(t);
        //
        if (r != keyword_tree.end()){
            uid * o = r->second;
            return new Token(o[0], o[1], info);
        }
        //
        return new Token(t, WORD, info);
    }
    
    else if (isdigit(u)){
        long long big = 0;
        long long small = 0;
        long long del = 1;
        bool dot_met = false;
        //type
        // 1 - integer
        // 2 - long
        // 3 - float 
        // 4 - double
        char type = 'i';
        // check symbol
        switch (u){
            case '0':
            u = get_u(f);
            //
            if (isdigit(u)) goto l2;
            //
            switch(u){
                case '.': goto l1;
                case 'x':
                //
                    u = get_u(f);
                  //
                    if (u == '.'){
                        //
                        type = 'd';
                        //
                        u = get_hex_num(f, big);
                        // if got P number, then check for - 
                        if (u == 'P'){
                            u = get_u(f);
                            //
                            if (u != '-'){
                                if (isxdigit(u)){
                                    small*=16; del*=16;
                                    u = tolower(u);
                                    small+=(isdigit(u))
                                        ? u - '0'
                                        : u - 'a' + 10;
                                }
                                else{
                                    goto ret_num;
                                }
                            }
                            u = get_hex_num(f, small, del);
                        }
                    }
                    else{
                        //
                        u = get_hex_num(f, big);
                        //
                        if (u == '.') goto error;
                    }
                    goto ret_num;
                case 'b':
                //
                    u = get_bin_num(f, big);
                    goto ret_num;
                //
            }
            //
            goto ret_num;
            default:
            big += u - '0';
            l2:
            // process with simple decimal number
            u = get_dec_num(f, big);
            // check if meet an dot
            if (u == '.'){
                l1:
                // set default type to double
                type = 'd';
                u = get_dec_num(f, small, del);
            }
        }
        
        ret_num:
        
        if (u != -1){
            if (iswalpha(u)){
                type = (char) tolower(u);
            }
            else if (isdigit(u)){
                goto error;
            }
            else if (u != -1){
                f.back(1);
            }
        }
        //
        switch (type){
            case 'i':
                if (dot_met) break;
                return new Token(big, INTEGER, info);
            case 'l':
                if (dot_met) break;
                return new Token(big, LONG, info);
            case 'f':
                return new Token((small/del)+big, FLOAT, info);
            case 'd':
                return new Token((small/del)+big, DOUBLE, info);
        }
        // here will be processed an error
        error:
        return new Token((wchar_t)u, NOT_EXPECTED, info);
    }
    else 
    {
        //
        // trying to find symbol from radix
        auto it=sym_radix->tree.find(u);
        //
    if (it != sym_radix->tree.end()){
        //
        Radix * map_t;
        //
        wstring t(L"");
        //
        do{
            t += (wchar_t)u;
            //
            u = f.get();
            //
            map_t = it->second;
            //
            if (map_t == NULL) break;
            //
            it = map_t->tree.find(u);
            //
        } while (it != map_t->tree.end());
        //
        if (u != -1) f.back(1);
        
        // 
        uid * d = sym_tree[t];
        //
        auto it2 = type_map[d[1]];
        //
        return it2(d[0], f, info);
        
        }
    else {
        return new Token((wchar_t)u, NOT_EXPECTED, info);
    }
    
    }
 // 
}

//
codecvt_utf8<wchar_t> * codecvt_utf8_1 = new codecvt_utf8<wchar_t>;
// wchar type buffer
wbuffer wchar_buffer(char * path){
   // create wifstream
   wifstream * fs = new wifstream();
   // imbue code utf-8
   fs->imbue(std::locale(locale(), codecvt_utf8_1));
   // open path
   fs->open(path);
   // return buffer
   return wbuffer(fs, path);
}
// 
namespace mark{
    uid vertical_bar2 = sym_tree[L"||"][0];
    uid ampersand2 = sym_tree[L"&&"][0];
    uid vertical_bar = sym_tree[L"|"][0];
    uid caret = sym_tree[L"^"][0];
    uid ampersand = sym_tree[L"&"][0];
    uid assign2 = sym_tree[L"=="][0];
    uid exclamation = sym_tree[L"!"][0];
    uid exclamation_assign = sym_tree[L"!="][0];
    uid less = sym_tree[L"<"][0];
    uid less_assign = sym_tree[L"<="][0];
    uid greater = sym_tree[L">"][0];
    uid greater_assign = sym_tree[L">="][0];
    uid less2 = sym_tree[L"<<"][0];
    uid greater2 = sym_tree[L">>"][0];
    uid greater3 = sym_tree[L">>>"][0];
    uid plus = sym_tree[L"+"][0];
    uid plus2 = sym_tree[L"++"][0];
    uid minus = sym_tree[L"-"][0];
    uid minus2 = sym_tree[L"--"][0];
    uid percent = sym_tree[L"%"][0];
    uid asterisk = sym_tree[L"*"][0];
    uid slash = sym_tree[L"/"][0];
    uid percent_assign = sym_tree[L"%="][0];
    uid ampersand_assign = sym_tree[L"&="][0];
    uid asterisk_assign = sym_tree[L"*="][0];
    uid plus_assign = sym_tree[L"+="][0];
    uid minus_assign = sym_tree[L"-="][0];
    uid slash_assign = sym_tree[L"/="][0];
    uid less2_assign = sym_tree[L"<<="][0];
    uid assign = sym_tree[L"="][0];
    uid semicolon = sym_tree[L";"][0];
    uid greater2_assign = sym_tree[L">>="][0];
    uid greater3_assign = sym_tree[L">>>="][0];
    uid caret_assign = sym_tree[L"^="][0];
    uid vertical_bar_assign = sym_tree[L"|="][0];
    uid question = sym_tree[L"?"][0];
    uid colon = sym_tree[L":"][0];
    uid arrow = sym_tree[L"->"][0];
    uid dot = sym_tree[L"."][0];
    uid tilda = sym_tree[L"~"][0];
    uid colon2 = sym_tree[L"::"][0];
    uid dot3 = sym_tree[L"..."][0];
    uid at = sym_tree[L"@"][0];
    uid comma = sym_tree[L","][0];
    uid brace_open = sym_tree[L"{"][0];
    uid bracket_open = sym_tree[L"["][0];
    uid parenthes_open = sym_tree[L"("][0];
    uid brace_close = sym_tree[L"}"][0];
    uid bracket_close = sym_tree[L"]"][0];
    uid parenthes_close = sym_tree[L")"][0];
}

//#undef rnd
#undef stv
//#undef dist 
//#undef gen
};

//#include "token.cpp"

namespace {
//
struct Node{
    uid type;
};

namespace prc_map{
    
map<uid, char> postfix = {
    //
    {mark::plus2, 13},
    {mark::minus2, 13}
    //
};

map<uid, char> prefix = {
    //
    {mark::plus2, 12},
    {mark::minus2, 12}
    //
};

map<uid, char> unary = {
    //
    {mark::plus, 12},
    {mark::minus, 12},
    {mark::tilda, 12},
    {mark::exclamation, 12}
    //
};

map<uid, char> binary = {
    //
    {mark::asterisk, 11},
    {mark::slash, 11},
    {mark::percent, 11},
    //
    {mark::plus, 10},
    {mark::minus, 10},
    //
    {mark::greater2, 9},
    {mark::greater3, 9},
    {mark::less2, 9},
    //
    {mark::less, 8},
    {mark::greater, 8},
    {mark::less_assign, 8},
    {mark::greater_assign, 8},
    //
    {mark::assign2, 7},
    {mark::exclamation_assign, 7},
    //
    {mark::ampersand, 6},
    //
    {mark::caret, 5},
    //
    {mark::vertical_bar, 4},
    //
    {mark::ampersand2, 3},
    //
    {mark::vertical_bar2, 2}
    //
};

map<uid, char> assign = {
    //
    {mark::assign, 0},
    {mark::plus_assign, 0},
    {mark::minus_assign, 0},
    {mark::asterisk_assign, 0},
    {mark::slash_assign, 0},
    {mark::percent_assign, 0},
    {mark::less2_assign, 0},
    {mark::greater2_assign, 0},
    {mark::greater3_assign, 0},
    {mark::ampersand_assign, 0},
    {mark::caret_assign, 0},
    {mark::vertical_bar_assign, 0}
    //
};
  
}

/*
template <typename k>
class iterator{
    public:
    virtual k next() = 0;
    virtual bool hasNext() = 0;
    virtual bool hasBack() = 0;
    virtual k peek() = 0;
    virtual k back() = 0;
};*/

class TokenIterator{//: iterator<Token*>{
    public:
    // container 
    Token** begin;
    // pointer within container
    Token** pointer;
    // end of container
    Token** end;
    
    TokenIterator(Token** vec, size_t len, size_t pointer = 0){
        this->begin = vec;
        this->pointer = vec + pointer;
        this->end = vec + len;
    }
    
    virtual Token* peek(){
        if (this->hasNext()){
            return *pointer;
        }
        else{
            return NULL;
        }
    }
    
    virtual Token* next(){
        if (this->hasNext()){
            return *pointer++;
        } 
        else{
            return NULL;
        }
    }
    
    virtual Token* back(){
        if (this->hasBack()){
            return *--pointer;
        }
        else{
            return NULL;
        }
    }
    virtual bool hasBack(){
        return this->pointer > this->begin;
    }
    
    virtual bool hasNext(){
        return this->pointer < this->end;
    }
};
wostream &operator<<(wostream &os, Token const &m) {
    uid u = m.type;
    if (u == STRING)return os<<L"\""<<(wchar_t*)m.value<<L"\"";
    else
    if (u == WORD)return os<<(wchar_t*)m.value;
    else
    if (u == COMMENT)return os<<L"/*"<<(wchar_t*)m.value<<L"*/";
    else
    if (u == CHARACTER)return os<<L"\'"<<*(wchar_t*)m.value<<L"\'";
    else
    if (u == DOUBLE|| u == FLOAT)return os<<*(long double*)m.value;
    else
    if (u == INTEGER|| u == LONG) return os<<*(long long*)m.value;
    else
    if (u == SPACE) return os<<"#space("<<*(long long*)m.value<<L")#";
    else
    if (u < -1) return os<<"!!ERROR!!";
    else
    return os<<uid_string_map[*(uid*)m.value];
}

wostream &operator<<(wostream &os, Token* m) {
    Token k = *m ;
    return os << k;
}

wostream& operator<<(wostream &os, Node* m);

wostream &operator<<(wostream& os, TokenIterator const &m) { 
    Token** begin = m.begin;
    Token** end = m.end;
    //
    if (begin == end) return os << L"[]";
    end --;
    //
    os <<L"[";
    //
    while (begin < end){
        os << (**begin);
        os << (L", ");
        begin++;
    };
    os << **end;
    os << L"]";
    return os;
}

/*
wostream &operator<<(wostream &os, TokenIterator const &m) { 
    return os << &m;
}
*/
struct syntax_error : public std::exception
{
    public :
    syntax_error(){
    }
	const char * what () const throw ()
    {
    	return "";
    }
};

struct TokenNode: Node{
    Token * value;
};

struct ListNode: Node{
    vector<Node*> * args;
};

struct InsnNode: ListNode{
    Token * value;
};

struct TypeNode: ListNode{
    uint dim;
};

namespace NodeTypes{
    uid constant_node = rnd;
    uid word_node = rnd;
    uid func_node = rnd;
    uid assign_op_node = rnd;
    uid postfix_op_node = rnd;
    uid prefix_op_node = rnd;
    uid binary_op_node = rnd;
    uid unary_op_node = rnd;
    uid ternary_node = rnd;
    uid dot_node = rnd;
    uid item_node = rnd;
    uid cast_node = rnd;
    uid new_node = rnd;
    uid type_node = rnd;
    uid ins_node = rnd;
    
    uid anon_node = rnd;
    uid alloc_node = rnd;
    uid array_node = rnd;
    
    uid list_node = rnd;
    
    uid primary_node = rnd;
}


void token_node_cast(Node* node, wostream&os){
    os<<*(((TokenNode*)(void*)(node))->value);
    return;
}

void insn_node_cast(Node*node, wostream&os){
    //
    os<<*(((InsnNode*)(void*)(node))->value);
    //
    vector<Node*> * vec = (((InsnNode*)(void*)(node))->args);
    //
    os<<L"(";
    for (Node*node :*vec){
        os<<node<<L", ";
    }
    os<<L")";
    //
    return ;
}

void list_node_cast(Node*node, wostream&os, wchar_t * T){
    //
    os<<T;
    vector<Node*> * vec = (((ListNode*)(void*)(node))->args);
    //
    os<<L"(";
    for (Node*node :*vec){
        os<<node<<L", ";
    }
    os<<L")";
    //
    return;
}

void list_node_cast_ternary(Node*node, wostream&os){
    list_node_cast(node, os, L"?");
}

void list_node_cast_dot(Node*node, wostream&os){
    list_node_cast(node, os, L".");
}

void list_node_cast_item(Node*node, wostream&os){
    list_node_cast(node, os, L"[]");
}

void list_node_cast_cast(Node*node, wostream&os){
    list_node_cast(node, os, L"cast");
}

void list_node_cast_new(Node*node, wostream&os){
    list_node_cast(node, os, L"new");
}

void list_node_cast_type(Node*node2, wostream&os){
   //
   TypeNode * node = (TypeNode*)(void*)node2;
   // list_node_cast(node, os, L"class");
   os<<L"class ";
   //
   auto ptr = node->args->begin();
   auto end = node->args->end();
   //
   os<<*ptr++;
   //
   while (ptr < end){
       os << L"."<<*ptr++;
   };
   uint j = node->dim;
   // iterate until j > 0
   while (j > 0){
       os<<L"[]";
       j --;
   }
}

void list_node_cast_ins(Node*node, wostream&os){
    list_node_cast(node, os, L"instanceof");
}

void list_node_cast_anon(Node*node, wostream&os){
    list_node_cast(node, os, L"anonumous");
}

void list_node_cast_array(Node*node, wostream&os){
    list_node_cast(node, os, L"array");
}

void list_node_cast_alloc(Node*node, wostream&os){
    list_node_cast(node, os, L"alloc");
}

void list_node_cast_list(Node*node, wostream&os){
    list_node_cast(node, os, L"list");
}

map<uid, void(*)(Node*, wostream&)> node_types_map = {
    {NodeTypes::constant_node, token_node_cast},
    {NodeTypes::primary_node, token_node_cast},
    {NodeTypes::word_node, token_node_cast},
    {NodeTypes::func_node, insn_node_cast},
    {NodeTypes::assign_op_node, insn_node_cast},
    {NodeTypes::postfix_op_node, insn_node_cast},
    {NodeTypes::prefix_op_node, insn_node_cast},
    {NodeTypes::binary_op_node, insn_node_cast},
    {NodeTypes::unary_op_node, insn_node_cast},
    {NodeTypes::ternary_node, list_node_cast_ternary},
    {NodeTypes::dot_node, list_node_cast_dot},
    {NodeTypes::item_node, list_node_cast_item},
    {NodeTypes::cast_node, list_node_cast_cast},
    {NodeTypes::type_node, list_node_cast_type},
    {NodeTypes::ins_node, list_node_cast_ins},
    {NodeTypes::anon_node, list_node_cast_anon},
    {NodeTypes::array_node, list_node_cast_array},
    {NodeTypes::alloc_node, list_node_cast_alloc},
    {NodeTypes::list_node, list_node_cast_list},
    {NodeTypes::new_node, list_node_cast_new}
};

wostream& operator<<(wostream &os, Node* m){
    // get type
    if (m == NULL){
        return os<<"#NULL";
    }
    //
    uid type = m->type;
    //
    map<uid, void(*)(Node*, wostream&)>::iterator it =
       node_types_map.find(type);
    //
    it->second(m, os);
    return os;
} 

Node * getTokenNode(Token * token, uid type){
    // create new constant node
    TokenNode * node = new TokenNode;
    // set constant node fields
    node->value = token;
    //
    node->type = type;
    return node;
}

Node * getConstantNode(Token * t){
    return getTokenNode(t, NodeTypes::constant_node);
}

Node * getWordNode(Token * t){
    return getTokenNode(t, NodeTypes::word_node);    
}

Node * getInsnNode(Token * t, vector<Node*>* args, uid type){
    // create new insn node
    InsnNode * node = new InsnNode;
    // set node fields
    node->args = args;
    node->value = t;
    node->type = type;
    // return node;
    return node;
};

Node * getListNode(vector<Node*>* args, uid type){
    // create new insn node
    ListNode * node = new ListNode;
    // set node fields
    node->args = args;
    //
    node->type = type;
    // return node;
    return node;
};

Node * getTypeNode(vector<Node*>*args, int dim = 0){
    // create new type node
    TypeNode * node = new TypeNode;
    // set node fields
    node->args = args;
    //
    node->type = NodeTypes::type_node;
    node->dim = dim;
    //
    return node;
}

#define contains(X, Y, Z) (X = Y.find(Z)) != Y.end()

namespace Expr{
    //
    // this set is for closing values 
    static set<uid> token_closing = {mark::colon, mark::semicolon, mark::comma};
    
    // struct, that consist data about syntax error
    struct Flaw{
        // type of the error
        uid type;
        // symbol, that consist information about where error occured
        Token * token;
    };
    
    // struct consist expected an symbol error
    struct FlawExpected: Flaw{
        uid symbol;
    };
    
    // struct consist error required one, buf found other
    struct RecFound: Flaw{
        uid rec;
        uid found;
    };
    
    // there are flaw types
    namespace FlawTypes{
        uid flaw_expected = rnd;
        uid flaw_illegal_start = rnd;
        uid flaw_unexpected = rnd;
        uid variable = rnd;
        uid value = rnd;
    }
    
    // an structure that consist some data about errors
    struct Log{
        uid end_mark = mark::semicolon;
        vector<Flaw*> *flaws;
    };
    
    void throw_syntax_error(){
        throw syntax_error();
    }
    
    // this function will be called at the assign of the non-variable
    void throw_syntax_error_variable(Log*c){
        //
        auto i = new RecFound;
        //
        i->type = FlawTypes::flaw_unexpected;
        i->rec = FlawTypes::variable;
        i->found = FlawTypes::value;
        //
        c->flaws->push_back(i);
    }
    
    // this function will be called at the end of the expression
    void throw_syntax_error_end(Log*c){
        //
        auto i = new FlawExpected;
        //
        i->type == FlawTypes::flaw_expected;
        i->symbol = c->end_mark;
        c->flaws->push_back((Flaw*)i);
        //
    }
    // this function will be called if expression is null
    void throw_illegal_start(Log*c){
        auto i = new Flaw;
        i->type == FlawTypes::flaw_illegal_start;
        c->flaws->push_back((Flaw*)i);
    }
    
    //
    
    // this function checks if Node is assignable
    bool can_assign(Node * node){
        if (node->type == NodeTypes::word_node){
            return true;
        }
        return false;
    }
    
    // this function checks if Token is closing
    bool is_closing(Token * token){
        uid u = token->type;
        if (token->type == BR_close) return true;
        if (token->type == SPECIAL){
            u = *((uid*)token->value);
            return token_closing.find(u) != token_closing.end();
        }
        return false;
    }

    
    Node * get_expr(TokenIterator*, Node *, char, Log*);
    //
    Node * get_constant(TokenIterator*, Node*, Token*, char, Log*);
    Node * get_math(TokenIterator*, Node*, Token*, char, Log*);
    Node * get_spc(TokenIterator*, Node*, Token*, char, Log*);
    Node * get_brc(TokenIterator*, Node*, Token*, char, Log*);
    Node * get_word(TokenIterator*, Node*, Token*, char, Log*);
    Node * get_key(TokenIterator*, Node*, Token*, char, Log*);
    Node * get_new(TokenIterator*, Node*, Token*, char, Log*);
    Node * get_ins(TokenIterator*, Node*, Token*, char, Log*);
    
    Node * get_class_node(TokenIterator *, char, Log*);
    Node * get_class_node(TokenIterator *it, Log*log){
        return get_class_node(it,0b10000000, log);
    }
    //
    // this set is for operations with tokens
    map<uid, Node*(*)(TokenIterator*, Node*, Token*, char, Log*)> type_expr = {
         {STRING, get_constant},
         {INTEGER, get_constant},
         {WORD, get_word},
         {FLOAT, get_constant},
         {DOUBLE, get_constant},
         {LONG, get_constant},
         {CHARACTER, get_constant}, 
         {BOOL, get_constant},
         {MATH, get_math},
         {SPECIAL, get_spc},
         {KEYWORD, get_key},
         {BR_open, get_brc}/*,
         {KEYWORD, get_key}*/};
    //
    bool return_true(Token*t){
        return true;
    }
    bool return_false(Token*t){
        return false;
    }
    bool return_is_round_bracket(Token *t){
        return *(uid*) t->value == mark::parenthes_open;
    }
    bool return_right_keyword(Token*t){
        return *(uid*) t->value == keyword::_new;
    }
    // this set is an another set for get_round_brc function
    map<uid, bool(*)(Token*)> round_brc_expr = {
         {STRING, return_true},
         {INTEGER, return_true},
         {WORD, return_true},
         {FLOAT, return_true},
         {DOUBLE, return_true},
         {LONG, return_true},
         {CHARACTER, return_true}, 
         {BOOL, return_true},
         {MATH, return_false},
         {SPECIAL, return_false},
         {PRIMARY, return_false},
         {KEYWORD, return_right_keyword},
         {BR_open, return_is_round_bracket},
         {BR_close, return_false}};
    //
    // this set is an another set for get_key function
    map<uid, Node*(*)(TokenIterator*,Node*,Token*,char,Log*)>key_map={
        {keyword::_instanceof, get_ins},
        {keyword::_new, get_new}
    };
    //
    
    Node * get_key(TokenIterator*it,Node*prev,Token*t,
                    char prior,Log*log){
        //
        auto tt = key_map.find(*(uid*)t->value);
        //
        if (tt != key_map.end()){
            // then return key operation
            return tt->second(it, prev, t, prior, log);
        }
        else{
            return NULL;
         //   throw_illegal_start();
        }
    }
    //
    vector<Node*> * get_node_list(TokenIterator*it, uid m, 
                           vector<Node*> * vec, Log*);
    
    vector<Node*> * get_node_list(TokenIterator*it, uid m, Log*log){
        return get_node_list(it, m, new vector<Node*>, log);
    } 
    
    vector<Node*> * get_node_list(TokenIterator*it, uid m, 
                        vector<Node*> * vec, Log*log){
        //
        Token * t;
        Node * temp;
        //
        t = it->next();
        // if got ), then return list
        if (t == NULL){
            goto err;
        }
        else{
            //
            if (t->type==BR_close&&*(uid*)t->value==m){
                //
                return vec;
            }
            else{
                it->back();
            }
        }
        // 
        loop1:{
            // get expression and add
            temp = get_expr(it, NULL, 0, log);
            // 
            if (temp != NULL){
                vec->push_back(temp);
            }
            // get next token and check 
            t = it->next();
            // 
            if (t == NULL) {
                goto err;
            }
            else if (t->type==SPECIAL&&*(uid*)t->value==mark::comma){
                goto loop1;
            }
            else if (t->type==BR_close&&*(uid*)t->value==m){
                return vec;
            }
            else {
                goto err;
            }
        }
        
        err:
        throw_syntax_error();
    }
    //
    Node * get_array_expr(TokenIterator*it, Log*log){
        return getListNode(get_node_list(it, mark::brace_close, log), 
            NodeTypes::list_node);
    }
    //
    
    //
    Node*get_alloc_expr(TokenIterator*it,Node*prev,Token*t,
                         char prior, Log * log){
        //
        //
        uint a; vector<Node*> * vec;
        uid tt ;
        uid node_type;
        using namespace mark;
        //
        // check if next symbol is ] 
        // if yes, then return defined array
        // else return allocated array
        t = it->next();
        //
        if (t == NULL){
            throw_syntax_error();
        }
        else if (t->type==BR_close&&*(uid*)t->value==bracket_close){
            //
            node_type = NodeTypes::array_node;
            //
            a = 1; 
            back1:
            t = it->next();
            // check for token
            if (t == NULL){
                throw_syntax_error();
            }
            else if (t->type==BR_open){
                tt = *(uid*)t->value;
                //
                if (tt==mark::bracket_open){
                    t = it->next();
                    // if got not ] symbol, then raise error
                    if (t==NULL||t->type!=BR_close||
                        *(uid*)t->value!=mark::bracket_close){
                        throw_syntax_error();
                    }
                    // go to back
                    a ++;
                    goto back1;
                }
                else if (tt==mark::brace_open){
                    // create vector 
                    vec = get_node_list(it,mark::brace_close, 
                        new vector<Node*>({prev}), log);
                    //
                    ret:
                    // set dimension
                    ((TypeNode*)prev)->dim = a;
                    //
                    return getListNode(vec, node_type);
                }
            }
            else{
                throw_syntax_error();
            }
        }
        else{
            node_type = NodeTypes::alloc_node;
            a = 0;
            // there will be returned allocation node type
            it->back();
            // create new vector
            vec = new vector<Node*>({prev});
            Node * exp;
            // 
            back2:
            //
            exp = get_expr(it, NULL, 0, log);
            // check if next token is ]
            t = it->next();
            //
            if (t==NULL||t->type!=BR_close||
                *(uid*)t->value!=mark::bracket_close){
                throw_syntax_error();
            }
            else{
                a++;
            }
            // if got null, then go to back3;
            if (exp == NULL){
                goto back3;
            }
            else{
                //
                vec->push_back(exp);
                //
                // check for next token
                t = it->next();
                // 
                if (t==NULL||t->type!=BR_open||
                    *(uid*)t->value!=mark::bracket_open){
                    goto ret;
                }
                else{
                    goto back2;
                }
            }
            //
            back3:
            //
            // check for next token
            t = it->next();
            // 
            if (t==NULL||t->type!=BR_open||
                *(uid*)t->value!=mark::bracket_open){
                goto ret;
            }
            else{
                a ++;
            }
            //
            t = it->next();
            if (t==NULL||t->type!=BR_close||
                *(uid*)t->value!=mark::bracket_close){
                throw_syntax_error();
            }
            else{
                goto back3;
            }
            //
        }
        //
    };
    // 
    map<uid, Node*(*)(TokenIterator*,Node*,Token*,char,Log*)> new_map={
        // this function suitable for ( symbol
        {mark::parenthes_open,
            [](TokenIterator*it,Node*prev,Token*t,char prior, Log*log)
            -> Node *{
                //
                vector<Node*> * node;
                //
                if (prev->type!=NodeTypes::type_node){
                    throw_syntax_error();
                }
                //
                get_node_list(it,mark::parenthes_close,
                         node = new vector<Node*>({prev}), log);
                //
                return getListNode(node,NodeTypes::new_node);
                //
            }},
        // this function suitable for { symbol
        {mark::brace_open, 
            [](TokenIterator*it,Node*prev,Token*t,char prior, Log*log)
            -> Node * {
                
                throw_syntax_error();
            }},
        // this function suitable for [ symbol
        {mark::bracket_open, get_alloc_expr}
    };
    
    Node * get_array_expr(TokenIterator*it,Node*prev,Token*t,
                           char prior, Log*log){
        return get_array_expr(it, log);
    }
    //
    Node * get_new(TokenIterator*it,Node*prev,Token*t,char prior,
                                   Log * log){
        //
        //
        if (prev != NULL){
            throw_syntax_error();
        }
        //
        //
        prev = get_class_node(it, log);
        // get {, or [, or ( bracket
        t = it->next();
        // 
        if (t!=NULL&&t->type==BR_open){
            return new_map[*(uid*)t->value](it,prev,t,prior,log);
        }
        else{
            throw_syntax_error();
        }
        //
    }
    //
    Node * get_square_brc(TokenIterator *it, Node*prev, 
                        Token*t, char pr, Log*log){
        // check if previous node here
        if (prev == NULL){
            throw_syntax_error();
        }
        else{
            // get expression
            Node * arg = get_expr(it, NULL, 0, log);
            // return new node
            return getListNode(new vector<Node*>({prev, arg}), NodeTypes::item_node);
        }
    }
    ///
    Node * get_round_brc(TokenIterator *it, Node*prev, 
                        Token*t, char pr, Log*log){
        // check if previous node here
        if (prev == NULL){
            // check for next ) and following symbol
            Token ** pointer = it->pointer;
            //
            unsigned int tt = 0;
            // iterate until ) meet
            // if met ), then assume if 
            // next type is a constant
            loop1: {
                // get next token
                t = it->next();
                // check if token is null
                if (t == NULL) goto err;
                //
                // check if token is BR_open 
                if (t->type == BR_open){
                    //
                    if (mark::parenthes_open==*(uid*)t->value){
                        //
                        tt++;
                    }
                }
                // 
                else if (t->type == BR_close){
                    //
                    if (mark::parenthes_close==*(uid*)t->value){
                        //
                        if (tt == 0){
                            goto loop3;
                        }
                        tt--;
                    }
                }
                goto loop1;
            }
            loop2:
            // 
            // 
            it->pointer = pointer;
            // get expression
            prev = get_expr(it, NULL, 0, log);
            // get round bracket
            t = it->next();
            // 
            if ((t==NULL)||
                (t->type!=BR_close)||
                (*(uid*)t->value!=mark::parenthes_close)){
                    err:
                    throw_syntax_error();
            }
            return prev;
            
            loop3:
            // get next token
            t = it->next();
            // if got null
            if (t == NULL){
                goto loop2;
            }
            // else, then check 
            // if token must be cast
            if (round_brc_expr[t->type](t)){
                //
                it->pointer = pointer;
                //
                Node * prev = get_class_node(it, log);
                //
                it->next();
                //
                // return
                return getListNode(new vector<Node*>({prev, 
                get_expr(it, NULL, 12, log)}), NodeTypes::cast_node);
            }
            else{
                goto loop2;
            }
        }
        // if yes, then raise error
        else{
            throw_syntax_error();
        }
    }
    //
    Node*get_ins(TokenIterator*it,Node*prev,Token*t,
                     char prior, Log * log){
        if (prev == NULL){
            throw_syntax_error();
        }
        else if (prior > 8){
            //
            it->back();
            //
            return prev;
        }
        else{
            return getListNode(new vector<Node*>({prev,
                get_class_node(it, log)}),NodeTypes::ins_node);
        }
    }
    //
    Node*get_class_node(TokenIterator * it, char flags, Log*log){
        //  get word
        Token * t;
        vector<Node*> * vec = new vector<Node*>;
        //
        loop1:
        // get next token
        t = it->next();
        // if got null, then raise error
        if (t == NULL){
            throw_syntax_error();
        }
        // if got next token, then check if got word
        if (t->type == WORD){
            // 
            vec->push_back(getConstantNode(t));
            // check if next token is dot
            t = it->next();
            // 
            if (t!=NULL){
                if (t->type==SPECIAL&&
                   *(uid*)t->value==mark::dot){
                    goto loop1;
                }
                else{
                    it->back();
                }
            }
        }
        else if ((flags&0b10000000)>0&&t->type==PRIMARY&&
                   vec->size()==0){
            return getTokenNode(t, NodeTypes::primary_node);
        }
        else{
            throw_syntax_error();
        }
        //
        return getTypeNode(vec);
    }
    //
    map<uid, Node*(*)(TokenIterator*, Node*, 
                       Token*, char, Log*)> brc_expr = {
         {mark::parenthes_open, get_round_brc},
         {mark::brace_open, get_array_expr},
         {mark::bracket_open, get_square_brc}
 };
    //
    //
    Node * get_brc(TokenIterator* it, 
                 Node*prev, Token*t, char ch, Log * log){
        return brc_expr[*(uid*) t->value](it, prev, t, ch, log);
    }
    ///
    Node * get_word(TokenIterator*it, Token*word, Log * log){
        // check if next token is round bracket
        Token * token = it->next();
        //
        if (token!=NULL){
            if ((token->type==BR_open)&&
                ((*(uid*)token->value)==mark::parenthes_open)){
                //
                return getInsnNode(word, 
                    get_node_list(it, mark::parenthes_close, log),
                    NodeTypes::func_node);
            }
            else{
                it->back();
            }
        };
        return getWordNode(word);
    }
    //
    Node * get_word(TokenIterator*it,Node*prev,Token*t,
                  char prior, Log * log){
        //
        if (prev == NULL){
            //
            return get_word(it, t, log);
        }
        else{
            //
            throw_syntax_error();
        }
    }
    //
    Node * get_constant(TokenIterator*it,
                Node*prev,Token*t,char prior, Log * log){
    
        if (prev == NULL){
            return getConstantNode(t);
        }
        else{
            throw_syntax_error();
        }
    }
    //
    //
    Node * get_ternary(TokenIterator*, Node*, Token*, char, Log *);
    Node * get_dot(TokenIterator*, Node*, Token*, char, Log*);
    // this map is for special types
    map<uid,Node*(*)(TokenIterator*,Node*,Token*,char, Log*)>spc_expr={
        {mark::question, get_ternary},
        {mark::dot, get_dot}
    };
    //
    Node*get_spc(TokenIterator*it,Node*node,Token*token,
                           char prior, Log * log){
        // get spc uid 
        uid u = *(uid*) token->value;
        // get iterator from map
        auto tt = spc_expr.find(u);
        //
        return tt->second(it, node, token, prior, log);
    }
    // this map is for get_attr function
    map<uid, Node*(*)(TokenIterator*, Token*, Log*log)> get_attr_map = {
        {WORD, get_word}
    };
    // this function will return field after dot
    Node * get_attr(TokenIterator*it, Log*log){
        // get token
        Token * token = it->next();
        // get iterator
        auto it2 = 
            get_attr_map.find(token->type);
        //
        if (it2 == get_attr_map.end()){
            throw_syntax_error();
        }
        else{
            return it2->second(it, token, log);
        }
    }
    // 
    // 
    Node * get_dot(TokenIterator*it,Node*prev,Token*t,
                   char prior, Log*log){
        using mark::dot;
        using NodeTypes::dot_node;
        //
        auto vec = new vector<Node*>({prev});
        //
        back:
        // add node
        vec->push_back(get_attr(it, log));
        // check for next token
        t = it->next();
        // if got null
        if (t == NULL) goto ret;
        //
        if (t->type==SPECIAL&&*(uid*)(t->value)==mark::dot){
            goto back;
        }
        else{
            // go to back
            it->back();
            //
            ret:
            // return new node
            return getListNode(vec, dot_node);
        }
    }
    
    // ternary expression
    Node * get_ternary(TokenIterator* it,Node*prev,Token*t,
                        char prior, Log*log){
        using mark::colon;
        using NodeTypes::ternary_node;
        // check previous token
        if (prev == NULL){
            throw_syntax_error();
        }
        else{
            //
            Node * a = get_expr(it, NULL, 0, log);
            // 
            Token * d = it->next();
            // if is not colon met, then raise error
            if (d==NULL||d->type!=SPECIAL||*(uid*)d->value!=colon){
                throw_syntax_error();
            }
            // 
            Node * b = get_expr(it, NULL, 0, log);
            // 
            vector<Node*> *t = new vector<Node*>({prev, a, b});
            // return node
            ListNode * node = (ListNode*)(void*) getListNode(t, ternary_node);
            //
            return node;
        }
    }
    
    // math expression
    Node * get_math(TokenIterator*it,Node*prev,Token*t,
                     char prior, Log*log){
        using namespace NodeTypes;
        using namespace prc_map;
        
        // get an uid
        uid m = *(uid*) t->value;
        // get an iterator 
        map<uid, char>::iterator it2;// = prc_map::postfix.find(m);
        //
        if (prev == NULL){
            //
            if ((it2=prefix.find(m))!=prefix.end()){
                //
                prev = get_expr(it, NULL, it2->second, log);
                //
                if (!can_assign(prev)){
                    throw_syntax_error_variable(log);
                }
                
                return getInsnNode(t, new vector<Node*>
                        ({prev}), prefix_op_node);
            }
            else 
            if ((it2=unary.find(m))!=unary.end()){
                //
                prev = get_expr(it, NULL, it2->second, log);
                //
                return getInsnNode(t, new vector<Node*>
                        ({prev}), unary_op_node);
            }
            else{
                throw_syntax_error();
            }
        }
        else{
            if ((it2=postfix.find(m))!=postfix.end()){
                //
                if (can_assign(prev)){
                    //
                    return getInsnNode(t, new vector<Node*>
                        ({prev}), postfix_op_node);
                }
                else{
                    throw_syntax_error();
                }
            }
            else 
            if ((it2=assign.find(m))!=assign.end()){
                //
                if (it2->second < prior){
                    //
                    it->back();
                    return prev;
                }
                else{
                    //
                    return getInsnNode(t, new vector<Node*>({prev,
                        get_expr(it,NULL,0, log)}), assign_op_node);
                }
            }
            else
            if ((it2=binary.find(m))!=binary.end()){
                //
                //
                if (it2->second < prior){
                    //
                    it->back();
                    return prev;
                }
                else{
                    //
                    return getInsnNode(t, new vector<Node*>({prev,
            get_expr(it,NULL,it2->second+1, log)}), binary_op_node);
                }
            }
            else{
                throw_syntax_error();
            }
        }
    }
    // get expression
    Node * get_expr(TokenIterator*it,Node*prev,char prior,Log*log){
        get_expr:
        // this is node
        Node * node;
        // get next token
        Token * t = it->next();
        // 
        // if got null, then return prev
        if (t == NULL){
            return prev;
        }
        // check if got closing token
        if (is_closing(t)){
            //
            it->back();
            // then return previous element
            return prev;
        }
        // get next node 
        if (prev==(node=type_expr[t->type](it,prev,t,prior,log))){
            //
            return node;
        }
        else{
            prev = node;
            goto get_expr;
        }
        //
        return NULL;
    }
    //
    
    
}

//this function will convert an file to the token iterator
TokenIterator * get_token_iterator(char * path){
    // get wchar buffer
    wbuffer buf = wchar_buffer(path);
    // create vector of token operators
    vector<Token*> vec;
    //
    Token * token = get_token(buf);
    //
    uid type = token->type;
    // repeat until null token here
    while (token->type != NULL){
        // add token
        if (type!=COMMENT&&type!=SPACE){
            // push back
            vec.push_back(token);
        }
        // get new toke
        token = get_token(buf);
        //
        type = token->type;
    }
    if (vec.size() == 0){
        return new TokenIterator(NULL, NULL, 0);
    }
    else{
        // get size
        size_t size = vec.size();
        // create array
        Token ** array = new Token * [size];
        Token ** begin = array;
         // fill array
        for (auto i: vec){
            *begin++ = i;
        }
        // 
        vec.clear();
         // return
        return new TokenIterator(array, size, 0);
    }
}



}


int main(){
    using namespace Expr;
    //
    TokenIterator * it = get_token_iterator("task.java");
   
    wcout<<*it<<endl;
    
    Log*log = new Log;
    
    Node * node = get_expr(it, NULL, 0, log);

    wcout<<node<<endl;
  
    return 0;
}



