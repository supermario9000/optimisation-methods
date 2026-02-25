#include "lib.hpp"

void IDPmetodas(double r, double l)
{
    double L=r-l;//pradiniai duom
    double xm=(l+r)/2;
    double x1, x2;
    int ciklai=0;
    int funkcijuSkaic=0;
    double xmin;
    double ym, y1, y2;
    while(L>eps)
    {
        ciklai++;
        x1=l+L/4; //kairys viduriukas
        x2=r-L/4; //desinys viduriukas
        y1=f(x1);
        y2=f(x2);

        if(ciklai==1) 
        {
            ym=f(xm);
            funkcijuSkaic++;
        }
        funkcijuSkaic+=2;

        if (y1<ym){
            r=xm;
            xm=x1;
            ym=y1;
        }
        else if (y2<ym){
            l=xm;
            xm=x2;
            ym=y2;
        }
        else {
            l=x1;
            r=x2;
        }
        //cout<<fixed<<setw(6)<<setprecision(5)<<x1<<" "<<f(x1)<<" "<<xm<<" "<<f(xm)<<" "<<x2<<" "<<f(x2)<<" "<<min(min(f(x1),f(x2)),f(xm))<<endl;
        L=r-l;
    }
    cout<<"IDP metodas: "<<endl;
    cout<<"Ciklu skaicius: "<<ciklai<<endl;
    cout<<"Funkcijos apskaiciavimo skaicius: "<<funkcijuSkaic<<endl;
    if(y1<y2&&y1<ym) xmin=x1;
    else if(y2<y1&&y2<ym) xmin=x2;
    else xmin=xm;
    cout<<"Maziausio tasko sprendinys (x): "<<xmin<<endl;
    cout<<"Funkcijos reiksme sprendinyje: "<<setprecision(10)<<f(xmin)<<endl;
    cout<<"------------------------------"<<endl;
}

void AuksoPjuvioAlgoritmas(double r, double l)
{
    double fi=(sqrt(5)-1)/2;
    double L=r-l;
    double x1, x2, y1, y2;
    int ciklai=0;
    int funkcijuSkaic=0;
    double xmin;

    x1=r - fi*L;
    x2=l + fi*L;
    y1=f(x1);
    y2=f(x2);
    funkcijuSkaic+=2;
    while(L>eps)
    {
        ciklai++;
        if (y2<y1){
            l=x1;
            x1=x2;
            y1=y2;
            L=r-l;
            x2=l + fi*L;
            y2=f(x2);
            funkcijuSkaic++;
        }
        else {
            r=x2;
            x2=x1;
            y2=y1;
            L=r-l;
            x1=r - fi*L;
            y1=f(x1);
            funkcijuSkaic++;
        }
        //cout<<fixed<<setw(11)<<setprecision(10)<<x1<<" "<<f(x1)<<" "<<x2<<" "<<f(x2)<<" "<<min(f(x1),f(x2))<<endl;
    }
    cout<<"Aukso pjuvio algoritmas: "<<endl;
    cout<<"Ciklu skaicius: "<<ciklai<<endl;
    cout<<"Funkcijos apskaiciavimo skaicius: "<<funkcijuSkaic<<endl;
    if(y1<y2) xmin=x1;
    else xmin=x2;
    cout<<"Maziausio tasko sprendinys (x): "<<xmin<<endl;
    cout<<"Funkcijos reiksme sprendinyje: "<<setprecision(10)<<f(xmin)<<endl;
    cout<<"------------------------------"<<endl;
}

void NiutonoMetodas()
{
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