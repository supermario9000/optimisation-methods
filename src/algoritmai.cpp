#include "lib.hpp"

void IDPmetodas(double r, double l)
{
    double L=r-l;//pradiniai duom
    double xm=(l+r)/2;
    double x1, x2;
    int ciklai=0;
    int funkcijuSkaic=0;
    double xmin;
    while(L>eps)
    {
        ciklai++;
        x1=l+L/4; //kairys viduriukas
        x2=r-L/4; //desinys viduriukas
        if (f(x1)<f(xm)){
            r=xm;
            xm=x1;
            funkcijuSkaic+=2;//KLAUSIMAS kaip skaiciuoti funkciju call skaiciu.
        }
        else if (f(x2)<f(xm)){
            l=xm;
            xm=x2;
            funkcijuSkaic+=2;
        }
        else {
            l=x1;
            r=x2;
            funkcijuSkaic+=2;
        }
        //cout<<fixed<<setw(6)<<setprecision(5)<<x1<<" "<<f(x1)<<" "<<xm<<" "<<f(xm)<<" "<<x2<<" "<<f(x2)<<" "<<min(min(f(x1),f(x2)),f(xm))<<endl;
        L=r-l;
    }
    cout<<"IDP metodas: "<<endl;
    cout<<"Ciklu skaicius: "<<ciklai<<endl;
    cout<<"Funkcijos apskaiciavimo skaicius: "<<funkcijuSkaic<<endl;
    if(f(x1)<f(x2)&&f(x1)<f(xm)) xmin=x1;
    else if(f(x2)<f(x1)&&f(x2)<f(xm)) xmin=x2;
    else xmin=xm;
    cout<<"Maziausio tasko sprendinys (x): "<<xmin<<endl;
    cout<<"Funkcijos reiksme sprendinyje: "<<setprecision(10)<<f(xmin)<<endl;
    cout<<"------------------------------"<<endl;
}

void AuksoPjuvioAlgoritmas(double r, double l)
{
    double fi=(sqrt(5)-1)/2;//KLAUSIMAS ar reikia fi kad butu kaip paveikslely? (fi ir 1-fi yra r-l)
    double L=r-l;
    double x1, x2;
    int ciklai=0;
    int funkcijuSkaic=0;
    double xmin;

    x1=r - fi*L;
    x2=l + fi*L;
    while(L>eps)
    {
        ciklai++;
        if (f(x2)<f(x1)){
            l=x1;
            x1=x2;
            L=r-l;
            x2=l + fi*L;
            funkcijuSkaic+=2;
        }
        else {
            r=x2;
            x2=x1;
            L=r-l;
            x1=r - fi*L;
            funkcijuSkaic+=2;
        }
        //cout<<fixed<<setw(11)<<setprecision(10)<<x1<<" "<<f(x1)<<" "<<x2<<" "<<f(x2)<<" "<<min(f(x1),f(x2))<<endl;
    }
    cout<<"Aukso pjuvio algoritmas: "<<endl;
    cout<<"Ciklu skaicius: "<<ciklai<<endl;
    cout<<"Funkcijos apskaiciavimo skaicius: "<<funkcijuSkaic<<endl;
    if(f(x1)<f(x2)) xmin=x1;
    else xmin=x2;
    cout<<"Maziausio tasko sprendinys (x): "<<xmin<<endl;
    cout<<"Funkcijos reiksme sprendinyje: "<<setprecision(10)<<f(xmin)<<endl;
    cout<<"------------------------------"<<endl;
}

void NiutonoMetodas()
{
    //finkcijas apsirasiau lib.hpp
    //f(x)=(x^2-a)^2/b-1;
    //f'(x)=4x(x^2-a)/b;
    //f''(x)=4(3x^2-a)/b;

    vector<double> x;
    x.push_back(5);//x[0]
    double xi;
    while(true)
    {
        xi = x.back() - df(x.back())/ddf(x.back());
        if(abs(xi-x.back())<eps) break;
        x.push_back(xi);
        //cout<<fixed<<setw(11)<<setprecision(10)<<x.back()<<" "<<f(x.back())<<" "<<df(x.back())<<" "<<ddf(x.back())<<endl;
    }

    cout<<"Niutono metodas: "<<endl;
    cout<<"Ciklu skaicius: "<<x.size()-1<<endl;
    cout<<"Funkcijos apskaiciavimo skaicius: "<<2*(x.size()-1)<<endl;
    cout<<"Maziausias rastas taskas (sprendinys): "<<x.back()<<endl;
    cout<<"Funkcijos reiksme sprendinyje: "<<setprecision(10)<<f(x.back())<<endl;
    cout<<"------------------------------"<<endl;
}