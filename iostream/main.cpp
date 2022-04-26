#include <iostream>
#include <cstdio>
#include <iomanip>
 
class my_set_precision {
public:
    my_set_precision(size_t precision) : precision_(precision) {
    }
 
    size_t precision_;
};
 
class my_endl {
public:
    my_endl() = default;
};
 
class my_ostream{
public:
    /* settings */
 
    friend my_ostream& operator << (my_ostream& out, const my_set_precision& x) {
        out.precision = x.precision_;
        return out;
    }
 
    friend my_ostream& operator << (my_ostream& out, const int& x) {
        printf("%d", x);
        return out;
    }
 
    friend my_ostream& operator << (my_ostream& out, const long long& x) {
        printf("%lld", x);
        return out;
    }
 
    friend my_ostream& operator << (my_ostream& out, const float & x) {
        std::string format = "%." + std::to_string(out.precision) + "f";
        printf(format.c_str(), x);
        return out;
    }
 
    friend my_ostream& operator << (my_ostream& out, const double & x) {
        std::string format = "%." + std::to_string(out.precision) + "f";
        printf(format.c_str(), x);
        return out;
    }
 
    friend my_ostream& operator << (my_ostream& out, const std::string& x) {
        printf("%s", x.c_str());
        return out;
    }
 
    friend my_ostream& operator << (my_ostream& out, const my_endl& x) {
        out << "\n";
        return out;
    }
 
private:
    size_t precision;
};
 
class my_istream{
public:
 
    friend my_istream& operator >> (my_istream& in, int& x) {
        scanf("%d", &x);
        return in;
    }
 
    friend my_istream& operator >> (my_istream& in, long long& x) {
        scanf("%lld", &x);
        return in;
    }
 
    friend my_istream& operator >> (my_istream& in, float & x) {
 
        scanf("%f", &x);
        return in;
    }
 
    friend my_istream& operator >> (my_istream& in, double & x) {
        scanf("%lf", &x);
        return in;
    }
 
    friend my_istream& operator >> (my_istream& in, std::string& x) {
        char ch[255];
        scanf("%s", ch);
        x = ch;
        return in;
    }
 
};
 
my_ostream my_out;
my_istream my_in;
 
my_endl endl;
 
class X {
public:
    X(int x, int y) : x(x), y(y) {}
private:
    int x;
    int y;
    friend int sum_x_y(X& x);
    friend std::ostream&  operator << (std::ostream& out, const X& el);
    friend my_ostream&  operator << (my_ostream& out, const X& el);
    int GetN() const {
        return x;
    }
};
 
int sum_x_y(X& x) {
    return x.x + x.y;
}
 
std::ostream&  operator << (std::ostream& out, const X& el) {
    out << el.x << " " << el.y;
    return out;
}
 
 
int operator << (X& x, int y) {
    return sum_x_y(x) + y;
}
 
 
my_ostream& operator << (my_ostream& out, const X& el) {
    return out;
}
 
int main() {
 
    //std::cout << std::fixed << std::setprecision(3) << 3.3 << std::hex << 378375;
   // my_out << my_set_precision(3) << 3.88888 << 5 << 4 << 4 << "\n" << 78;
    my_out << my_set_precision(3) << 4.4;
    std::string s;
    my_in >> s;
    my_out << endl;
    my_out << s;
    my_out << endl;
/* ДЗ
    my_ifstream f("/home/vvv/jocjf.txt");
    f >> x;
    my_ofstream f("/home/vvv/jocjf.txt");
    f << 100; */
 
    my_out << s;
    return 0;
}
