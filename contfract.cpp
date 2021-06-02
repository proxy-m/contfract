#include <iostream>
#include <cmath>
#include <stdint.h>
using namespace std;

#define CONTFRACT_MODE_MIN 0
#define CONTFRACT_MODE_MAX 1

#define u8  unsigned char
#define s8  char
#define u16  unsigned short int
#define s16  short int
#define u32  unsigned int
#define s32  int
#define u64  uint64_t
#define s64  int64_t

/**
 * a / b
 */
struct NodeAB {
    u32 a;
    u32 b;
};

struct NodeD {
    u16 d;
};

struct SimpleFractAB {
#if (CONTFRACT_MODE_MAX >= 2)
    u8 isNegative; // 1 - negative, 0 - positive
#endif
    NodeAB nodeAB;
};

/**
 * a / b ~= d1 + 1 / (d2) ~= d1 + 1 / (e2 + 1 / (f3)) = [d1; e2, f3] ~= [a1; a2, a3 ... an]
 * See: https://en.wikipedia.org/wiki/Continued_fraction 
 */
#ifdef DEBUG
struct ContFractABC {
#if (CONTFRACT_MODE_MAX >= 2)
    u8 isNegative;
#endif
    NodeD nodeD;
    NodeD *nextCont;    
};
#endif

struct ContFract {
    u16 *a;
};

ContFract fract2contfract (s64 a, s64 b, double tol) {
    const int LEN = 255;
    tol = abs(tol);
    if (tol > 1.0) {
        tol = 1.0;
    }
    
    //cout << "a/b: " << a << " " << b << endl;
    
    u8 arr[LEN];
    for (int i=0; i<LEN; ++i) {
        arr[i] = 0;
    }
    arr[0] = 0; //

    u8 isNegative = (a < 0);
    if (isNegative) {
        isNegative = !(b < 0);
    } else {
        isNegative = (b < 0);
    }
    s64 an0 = abs(a);
    s64 bn0 = abs(b);
    
    int i = 0;
    int restCount = LEN - 1;
    while ((--restCount) > 0 && bn0 > tol) {
        s64 an1 = an0 / bn0;
        s64 bn1 = an0 % bn0;
        //cout << "an1 bn1: " << an1 << " " << bn1 << endl;
        if (an1 >= 256L*256L) {
            break;
        } else {
            arr[++i] = (u8)an1;
            //cout << (int)arr[i] << endl;
        }
        an0 = bn0;
        bn0 = bn1;
    }
    const int TOTAL_LEN = i+1;

    ContFract res = ContFract();
    res.a = new u16[TOTAL_LEN];
    for (int i=0; i<TOTAL_LEN; ++i) {
        res.a[i] = arr[i];
    }
    return res;
};

void printcontfract (ContFract cf) {
    if (cf.a[0] != 0) {
        cout << "Can not print for type: " << cf.a[0] << endl;
    } else {
        cout << "[";
        u8 isFirst = 2;
        int i = 1;
        while (cf.a[i] != 0 || i <= 2) {
            if (isFirst == 1) {
                cout << ";";
            } else if (isFirst == 0) {
                cout << ",";
            }
            cout << (int)cf.a[i];
            if (isFirst > 0) {
                --isFirst;
            }
            ++i;
        }
        cout << "]" << endl;
    }
};

void printsimplefract (SimpleFractAB f) {
    cout << f.nodeAB.a << "/" << f.nodeAB.b << endl;
};

ContFract double2contfract (double r, double tol) {
    const s64 f = 1000000000L;
    return fract2contfract((s64)std::floor(r*f), f, tol); // TODO simpily
};

double simplefract2double (SimpleFractAB simpleFractAB, double tol) {
    return 1.0*((double)simpleFractAB.nodeAB.a) / ((double)simpleFractAB.nodeAB.b); // TODO fix
};


s64 gcd05(s64 a, s64 b) { // NOD of 2 numbers
    a = abs(a);
    b = abs(b);
    while (a != b) {
        if (a > b) {
            s64 tmp = a;
            a = b;
            b = tmp;
        }
        b = b - a;
    }
    return a;
};

SimpleFractAB mult (SimpleFractAB x, SimpleFractAB y) {
    SimpleFractAB res = SimpleFractAB();
    res.nodeAB.a = x.nodeAB.a * y.nodeAB.a;
    res.nodeAB.b = x.nodeAB.b * y.nodeAB.b;
    s64 n = gcd05(res.nodeAB.a, res.nodeAB.b);
    res.nodeAB.a /= n;
    res.nodeAB.b /= n;
    return res;
};

SimpleFractAB summ (SimpleFractAB x, SimpleFractAB y) {
    SimpleFractAB res = SimpleFractAB();
    res.nodeAB.b = x.nodeAB.b * y.nodeAB.b;
    res.nodeAB.a = x.nodeAB.a * y.nodeAB.b + y.nodeAB.a * x.nodeAB.b;
    s64 n = gcd05(res.nodeAB.a, res.nodeAB.b);
    res.nodeAB.a /= n;
    res.nodeAB.b /= n;
    return res;
};

SimpleFractAB pow_1 (SimpleFractAB x) {
    SimpleFractAB res = SimpleFractAB();
    res.nodeAB.b = x.nodeAB.a;
    res.nodeAB.a = x.nodeAB.b;
    return res;
};

SimpleFractAB contfract2simplefract (ContFract cf) {
    SimpleFractAB simpleFractAB = SimpleFractAB();
    simpleFractAB.nodeAB.a = 0;
    simpleFractAB.nodeAB.b = 0;
    if (cf.a[0] != 0) {
        cout << "Can not simple for type: " << cf.a[0] << endl;
        return simpleFractAB; 
    } else {
        int i = 1;
        if (cf.a[i] == 0) {
           ++i;
        }
        while (i < 255 && cf.a[i] != 0) {
            ++i;
        }
        if (cf.a[i] != 0) {
            cout << "Broken type." << endl;
            return simpleFractAB;
        }
        const int LEN = i;
        --i;
        simpleFractAB.nodeAB.a = cf.a[i];
        simpleFractAB.nodeAB.b = 1;
        --i;
        while (i >= 1) {
            simpleFractAB = pow_1(simpleFractAB);

            SimpleFractAB addN = SimpleFractAB();
            addN.nodeAB.a = cf.a[i];
            addN.nodeAB.b = 1;

            simpleFractAB = summ(addN, simpleFractAB);           
            --i;
        }
        return simpleFractAB;
    }
};

int main (void) {
    ContFract cf;

    printcontfract(cf = double2contfract(4.0/9, 0.01));
    printsimplefract(contfract2simplefract(cf));

    printcontfract(cf = double2contfract(4.0/10, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(4.0/11, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(4.0/12, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(4.0/13, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(4.0/14, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(4.0/15, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(4.0/16, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(4.0/17, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(4.0/18, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(4.0/19, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(4.0/20, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(4.0/21, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(23.0/4, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(23.0/5, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(23.0/6, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(23.0/7, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(23.0/8, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(23.0/9, 0.01));
    printsimplefract(contfract2simplefract(cf));
    
    printcontfract(cf = double2contfract(233333333.0/500000000, 0.001));
    printsimplefract(contfract2simplefract(cf));
};
