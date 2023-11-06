#pragma once
#include <new>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <iostream>
#include <clocale>
#include <vector>

using namespace std;


class Avia
{
public:

    string number;

    string str_city_ar;
    string str_city_dep;
    string str_date_ar;
    string str_date_dep;
    int str_free_site;
    int str_occupied_site;
    int str_price;
    string str_company;
    string status;

    Avia() { number = "-"; str_city_ar = "-"; str_city_dep = "-"; str_date_ar = "-"; str_date_dep = "-"; str_free_site = -1; str_occupied_site = -1;  str_price = -1; str_company = "-"; status = "Active"; }
    Avia(string number1, string str_city_ar1, string str_city_dep1, string str_date_ar1, string str_date_dep1, int str_free_site1, int str_occupied_site1, int str_price1, string str_company1, string str_status1) {
        number = number1;
        str_city_ar = str_city_ar1;
        str_city_dep = str_city_dep1;
        str_date_ar = str_date_ar1;
        str_date_dep = str_date_dep1;
        str_free_site = str_free_site1;
        str_occupied_site = str_occupied_site1;
        str_price = str_price1;
        str_company = str_company1;
        status = str_status1;
    }

    friend ostream& operator<<(ostream& os, const Avia& x) {
        return os << x.number << " " << x.str_city_ar << " " << x.str_city_dep << " " << x.str_date_ar << " " << x.str_date_dep << " " << x.str_free_site << " " << x.str_occupied_site << " " << x.str_price << " " << x.str_company << " " << x.status << endl;
    }

    const Avia& operator = (const Avia& x) {
        number = x.number;
        str_city_ar = x.str_city_ar;
        str_city_dep = x.str_city_dep;
        str_date_ar = x.str_date_ar;
        str_date_dep = x.str_date_dep;
        str_free_site = x.str_free_site;
        str_occupied_site = x.str_occupied_site;
        str_price = x.str_price;
        str_company = x.str_company;
        status = x.status;
        return *this;
    }


};
