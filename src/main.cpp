#include "lib.hpp"

int main() {
    double x;

    double l = 0;//intervalu apibrezimas (nuo l iki r)
    double r = 10;
    IDPmetodas(r, l);
    AuksoPjuvioAlgoritmas(r, l);
    NiutonoMetodas();
    cout<<"Ar norite pamatyti funkcijos grafika? (y/n): ";
    char choice;
    cin >> choice;
    if (choice == 'y' || choice == 'Y') {
        openWindow();
    }
    return 0;
}