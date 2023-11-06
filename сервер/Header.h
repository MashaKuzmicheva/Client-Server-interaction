#pragma once
#pragma warning(disable : 4996)
#define _CRT_SUCURE_NO_WARNINGS
#define _CRT_SUCURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <iostream>
#include <clocale>
#include<cstdlib>
#include<vector>
#include"Avia.h"
using namespace std;


void Generate(FILE* f, int quantity, string* city, string* aviacompany, int size_city, int size_aviacompany, Avia* A);

void Search(vector<int>& res, Avia* A, string x);


void Add(Avia* A, string s, int size);
void Delete(Avia* A, string s, int size);
int Buy(Avia* A, string s, int size);
