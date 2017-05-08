#include "minimizer.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <algorithm>
#include <bitset>
#include <math.h>

minimizer::minimizer(QObject *parent) : QObject(parent)
{
    echo = false;
    inName = "input.txt";
    dcName = "input2.txt";
    outName = "output.txt";
}

void minimizer::setEcho()
{
    echo = true;
}

void minimizer::setInName(QString name)
{
    inName = name.toStdString();
}

void minimizer::setInName(std::string name)
{
    inName = name;
}

void minimizer::setDcName(QString name)
{
    dcName = name.toStdString();
}

void minimizer::setDcName(std::string name)
{
    dcName = name;
}

void minimizer::setOutName(QString name)
{
    outName = name.toStdString();
}

void minimizer::setOutName(std::string name)
{
    outName = name;
}

bool minimizer::setInFile()
{
    inFile.open(inName, std::ios::in);
    return inFile.is_open();
}

bool minimizer::setDcFile()
{
    dcFile.open(dcName, std::ios::in);
    return dcFile.is_open();
}

bool minimizer::setOutFile()
{
    outFile.open(outName, std::ios::out | std::ios::trunc);
    return outFile.is_open();
}

void minimizer::read()
{
    int m;

    //input file
    inFile >> n; inFile.ignore();
    max = pow(2,n) - 1;
    if(n < 2 || n > 4) {
        outFile << "number of inputs out of range" << std::endl;
        if(echo) std::cout << "number of inputs out of range" << std::endl;
        emit quit();
    }

    std::string ri;
    std::getline(inFile, ri);
    std::stringstream sri(ri);
    while (std::getline(sri, ri, ',')) {
        m = std::atoi(ri.c_str());
        if (m >= 0 && m <= max) {
            minterms.push_back(m);
        }
        else {
            outFile << "Invalid minterm" << std::endl;
            if(echo) std::cout << "Invalid minterm" << std::endl;
            emit quit();
            return;
        }
    }

    //dont care file
    if(dcFile.is_open()) {
        dcFile >> m; dcFile.ignore();
        if(m != (signed)n) {
            outFile << "number of inputs don't match" << std::endl;
            if(echo) std::cout << "number of inputs don't match" << std::endl;
            emit quit();
            return;
        }

        std::getline(dcFile, ri);
        std::stringstream sri2(ri);
        while (std::getline(sri2, ri, ',')) {
            m = std::atoi(ri.c_str());
            if (m >= 0 && m <= max && !isMinterm(m)) {
                dcterms.push_back(m);
            }
            else {
                outFile << "Invalid don't care term" << std::endl;
                if(echo) std::cout << "Invalid don't care term" << std::endl;
                emit quit();
                return;
            }
        }
    }

    if(echo) std::cout << F[n-2] << " = ";
    for (unsigned int i = 0; i < minterms.size(); i++) {
        binMinterms.push_back(binterms[n-2][minterms[i]]);

        if (i != minterms.size() - 1) {
            if(echo) std::cout << terms[n-2][minterms[i]] << " + ";
        }
        else {
            if(echo) std::cout << terms[n-2][minterms[i]] << std::endl;
        }
    }
    if(echo) std::cout << D[n-2] << " = ";
    for (unsigned int i = 0; i < dcterms.size(); i++) {
        binDcterms.push_back(binterms[n-2][dcterms[i]]);

        if (i != dcterms.size() - 1) {
            if(echo) std::cout << terms[n-2][dcterms[i]] << " + ";
        }
        else {
            if(echo) std::cout << terms[n-2][dcterms[i]] << std::endl;
        }
    }

    inFile.close();
    dcFile.close();

    minimize();
}

bool minimizer::isMinterm(int t)
{
    for(unsigned int i = 0; i < minterms.size(); i++)
        if(minterms[i] == t)    return true;

    return false;
}

bool minimizer::isEssential(std::string t)
{
    for(unsigned int i = 0; i < essential.size(); i++)
        if(essential[i] == t)    return true;

    return false;
}

bool minimizer::isDcterm(int t)
{
    for(unsigned int i = 0; i < dcterms.size(); i++)
        if(dcterms[i] == t)    return true;

    return false;
}

void minimizer::minimize()
{
    std::cout << "working.." << std::endl;
    //group minterms and don't care terms by number of ones
    group();
    //printRound(0);

    //combine size 0 implicants
    int cnt = combine(implicants[0]);
    int i = 1;
    //printRound(i);

    //while posible, combine implicants
    while(cnt > 0)
    {
        cnt = combine(implicants[i]);
        i++;
        //printRound(i);
    }

    //form the prime implicant chart and print
    std::vector<std::vector<std::string> > chart = primeChart();
    printChart(chart);

    //reduce the chart
    std::vector<std::vector<std::string> > reduced = reduce(chart);
    //printChart(reduced);

    //find required prime implicants from the reduced chart
    if(reduced.size() > 2)
        findRequired(reduced);
    //form final equation
    formFinal();

    //print results
    if(echo) std::cout << final << std::endl;
    outFile << final << std::endl;

    std::cout << "done!" << std::endl;
    emit quit();
    return;
}

//separate minterms and don't care terms into groups by number of ones
void minimizer::group()
{
    std::vector<std::vector<std::string> > size0(5);
    for(unsigned int i = 0; i < binMinterms.size(); i++)
    {
        int cnt = countOnes(binMinterms[i]);
        size0[cnt].push_back(binMinterms[i]);
    }

    for(unsigned int i = 0; i < binDcterms.size(); i++)
    {
        int cnt = countOnes(binDcterms[i]);
        size0[cnt].push_back(binDcterms[i]);
    }

    implicants.push_back(size0);
}

//separate terms in v by number of ones, and return vector containing all groups
std::vector<std::vector<std::string> > minimizer::group(std::vector<std::string> &v)
{
    std::vector<std::vector<std::string> > sizen(5);
    for(unsigned int i = 0; i < v.size(); i++)
    {
        int cnt = countOnes(v[i]);
        sizen[cnt].push_back(v[i]);
    }
    return sizen;
}

//count and return number of ones in a srting binary number
int minimizer::countOnes(std::string num)
{
    int c = 0;
    for(unsigned int i = 0; i < num.length(); i++) {
        if(num[i] == '1') c++;
    }

    return c;
}

//combine implicants and return number of combinations done
int minimizer::combine(std::vector<std::vector<std::string> > &sizen)
{
    unsigned int i, j, k;
    std::vector<std::vector<std::string> > size2n;
    std::vector<std::string> combined;
    int cnt = 0, oldc = 0;

    for(i = 0; i < sizen.size() - 1; i++) {
        std::vector<std::string> nbit = sizen[i];       //n ones group
        std::vector<std::string> np1bit = sizen[i+1];   //n+1 ones group
        if(nbit.empty())
            continue;
        for(j = 0; j < nbit.size(); j++) {
            std::string imp = nbit[j];
            if(np1bit.empty()){
                continue;
            }
            for(k = 0; k < np1bit.size(); k++) {
                std::string imp2 = np1bit[k];
                if(imp2[imp2.length() - 1] == '*')
                    continue;

                int idx = 0;
                int dif = compare(imp, imp2, idx);
                if(dif == 1) {
                    std::string str = imp;
                    if(str.back() == 'u') str.pop_back();
                    str[idx] = '-';
                    combined.push_back(str);
                    cnt++;
                    if(sizen[i+1][k].back() != 'u')   sizen[i+1][k] += "u"; //mark as used
                }
            }
            if(cnt == oldc && sizen[i][j].back() != 'u')
                sizen[i][j] += "*";

            oldc = cnt;
        }
    }

    //delete the used marks
    for(i = 0; i < sizen.size(); i++) {
        for(j = 0; j < sizen[i].size(); j++) {
            if(sizen[i][j].back() == 'u')
                sizen[i][j].pop_back();
        }
    }

    if(cnt == 0) {
        for(i = 0; i < sizen.size() - 1; i++) {
            if(sizen[i+1].empty()) {
                for(j = 0; j < sizen[i].size(); j++) {
                        sizen[i][j] += "*";
                }
            }
        }
    }

    //check for duplicates and delete them if found
    for(i = 0; i < combined.size(); i++) {
        for(j = i + 1; j < combined.size(); j++) {
            if(combined[i] == combined[j]) {
                combined.erase(combined.begin() + j);
                j--;
            }
        }
    }
    if(cnt > 0) {
        size2n = group(combined);
        implicants.push_back(size2n);
    }

    return cnt;
}

//compare two strings and return the number of character that are different, idx represent the index of the last difference found
int minimizer::compare(std::string n1, std::string n2, int &idx)
{
    if(n1.back() == 'u')    n1.pop_back();
    if(n2.back() == 'u')    n2.pop_back();

    if(n1.length() != n2.length())
        return 0;

    int cnt = 0;
    for(unsigned int i = 0; i < n1.length(); i++) {
        if(n1[i] != n2[i]) {
            cnt++;
            idx = i;
        }
    }
    return cnt;
}

std::vector<std::vector<std::string> > minimizer::primeChart()
{
    unsigned i, j, k;

    std::vector<std::vector<std::string> > chart;
    std::vector<std::vector<std::string> > imp;
    std::vector<std::string> tmp;

    //look for prime implicants
    for(i = 1; i < implicants.size(); i++) {
        for(j = 0; j < implicants[i].size(); j++) {
            for(k = 0; k < implicants[i][j].size(); k++) {
                std::string s = implicants[i][j][k];
                if(s.back() == '*') {
                    s.pop_back();
                    tmp.push_back(s);
                }
            }
        }
    }
    imp = group(tmp);
    tmp.clear();

    //form chart header
    tmp.push_back(" ");
    for(i = 0; i < minterms.size(); i++) {
        std::string str = std::to_string(minterms[i]);
        if(str.length() == 1)
            str += " ";
        tmp.push_back(str);
    }
    chart.push_back(tmp);
    tmp.clear();

    //form rest of the chart
    for(i = 0; i < imp.size(); i++) {
        for(j = 0; j < imp[i].size(); j++) {
            //only add the implicant if it contains a minterm (it is not a combination of don't cares
            bool added = false;
            for(k = 0; k < minterms.size(); k++)
                if(contains(imp[i][j], binMinterms[k])) {
                    tmp.push_back(imp[i][j]);
                    added = true;
                    break;
                }
            if(added){
                for(k = 0; k < minterms.size(); k++) {
                    if(contains(imp[i][j], binMinterms[k]))
                        tmp.push_back("X ");
                    else
                        tmp.push_back("  ");
                }
                chart.push_back(tmp);
            }
            tmp.clear();
        }
    }

    //find essential implicants
    for(i = 1; i < chart[0].size(); i++) {
        int idx, cnt = 0;
        for(j = 1; j < chart.size(); j++) {
            if(chart[j][i] == "X ") {
                cnt++;
                idx = j;
            }
        }
        if(cnt == 1) {
            if(!isEssential(chart[idx][0]))
                essential.push_back(chart[idx][0]);
        }
    }

    return chart;
}

std::vector<std::vector<std::string> > minimizer::reduce(std::vector<std::vector<std::string> > &chart)
{
    unsigned int i, j;
    std::vector<std::vector<std::string> > reduced;
    std::vector<std::string> row;
    std::vector<int> col(minterms.size());
    for(i = 0; i < col.size(); i++) col[i] = 0;

    //for eah essential find what minterms it contains and increment the corresponding column value
    for(i = 0; i < essential.size(); i++) {
        for(j = 0; j < binMinterms.size(); j++) {
            if(contains(essential[i], binMinterms[j]))
                    col[j]++;
        }
    }

    //store non essential rows in reduced chart
    for(i = 0; i < chart.size(); i++) {
        if(!isEssential(chart[i][0])) {
            row = chart[i];
            reduced.push_back(row);
        }
    }

    //delete column of minterms contained by an essential
    for(i = 0; i < col.size(); i++) {
        if(col[i] != 0) {
            for(j = 0; j < reduced.size(); j++) {
                int idx = i + 1;
                reduced[j].erase(reduced[j].begin() + idx);
            }
        }
    }

    //printChart(reduced);

    return reduced;
}

//return true if an implicant contains a minterm
bool minimizer::contains(std::string imp, std::string mint)
{
    unsigned i;

    if(imp.length() != mint.length())
        return false;

    for(i = 0; i < imp.length(); i++) {
        if(imp[i] == '-')
            continue;
        if(imp[i] != mint[i])
            return false;
    }
    return true;
}

bool binComp(std::string s1, std::string s2) {
    long long m1 = atoi(s1.c_str());
    long long m2 = atoi(s2.c_str());

    return m1 < m2;
}

void minimizer::findRequired(std::vector<std::vector<std::string> > &chart)
{
    std::vector<std::vector<std::string> > candidates;
    std::vector<std::string> t;
    std::vector<std::string> umin;
    std::vector<std::vector<std::string> > usedmin(chart.size() - 1);
    std::vector<std::vector<std::string> > missmin(chart.size() - 1);
    unsigned int i, j, k, min = 9999;

    //look for minterms in chart
    for(i = 1; i < chart[0].size(); i++) {
        std::string s = chart[0][i];
        if(s[1] == ' ')
            s.pop_back();

        s = std::bitset<4>(atoi(s.c_str())).to_string();
        umin.push_back(s);
    }

    //fill the used and missing minterms table
    for(i = 1; i < chart.size(); i++) {
        std::vector<std::string> u;
        std::vector<std::string> m;
        for(j = 0; j < binMinterms.size(); j++) {
            if(contains(chart[i][0], binMinterms[j]))
                u.push_back(binMinterms[j]);
            else
                m.push_back(binMinterms[j]);
        }
        usedmin[i - 1] = u;
        missmin[i - 1] = m;
    }

    //find candidates
    for(i = 1; i < chart.size(); i++) {
        std::vector<std::string> u;
        t.push_back(chart[i][0]);
        u = usedmin[i - 1];

        for(j = 1; j < chart.size(); j++) {
            bool add = true;

            if(i == j) continue;    //same term, ignore

            for(k = 0; k < u.size(); k++) {
                if(contains(chart[j][0], u[k])) {
                   add = false;
                }
            }
            if(add) {
                t.push_back(chart[j][0]);
                for(k = 0; k < usedmin[j - 1].size(); k++) {
                    u.push_back(usedmin[j - 1][k]);
                }
            }
        }
        std::sort(u.begin(), u.end(), binComp);
        //for(j = 0; j < u.size(); j++) std::cout << "u[" << j << "] = " << u[j] << std::endl;
        if(u == umin) {
            candidates.push_back(t);
        }
        t.clear();
    }

    //choose the shortest
    for(i = 0, j = 0; i < candidates.size(); i++) {
        if(candidates[i].size() < min) {
            min = candidates[i].size();
            j = i;
        }
    }

    if(candidates.size() != 0) required = candidates[j];
}

void minimizer::formFinal()
{
    unsigned int i;
    final = F[n-2] + " = ";

    for(i = 0; i < essential.size(); i++) {
        final += toA(essential[i]);

        if(i == essential.size() - 1) {
            if(required.size() != 0) {
                final += " + ";
            }
        }
        else
            final += " + ";
    }

    for(i = 0; i < required.size(); i++) {
        final += toA(required[i]);
        (i != required.size() - 1)? final += " + ": final += "";
    }
}

std::string minimizer::toA(std::string imp)
{
    unsigned int i;
    std::string str = "";
    std::string letters = "ABCD";

    for(i = 0; i < imp.length(); i++) {
        if(imp[i] != '-') {
            str += letters[i];
            if(imp[i] == '0')
                str += '\'';
        }
    }

    return str;
}

void minimizer::printRound(int r)
{
    unsigned int i;
    std::vector<std::string> lines;
    std::string srt;

    switch(r+1) {
    case 1:
        lines.push_back("First Round");
        break;
    case 2:
        lines.push_back("Second Round");
        break;
    case 3:
        lines.push_back("Third Round");
        break;
    case 4:
        lines.push_back("Fourth Round");
        break;
    case 5:
        lines.push_back("Fiveth Round");
        break;
    case 6:
        lines.push_back("Sixth Round");
        break;
    case 7:
        lines.push_back("Sevent Round");
        break;
    case 8:
        lines.push_back("Eighth Round");
        break;
    case 9:
        lines.push_back("Nineth Round");
        break;
    case 10:
        lines.push_back("Tenth Round");
        break;
    }

    lines.push_back("_____________________");
    lines.push_back("_____________________");

    /*
     * TODO
     */

    for(i = 0; i < lines.size(); i++) {
        outFile << lines[i] << std::endl;
        if(echo) std::cout << lines[i] << std::endl;
    }
}

void minimizer::printChart(std::vector<std::vector<std::string> > chart)
{
    unsigned int i, j;
    std::vector<std::string> lines;
    //header
    lines.push_back("Result");
    lines.push_back("____________________________________");
    lines.push_back("____________________________________");

    for(i = 0; i < chart.size(); i++) {
        std::string str = "";
        for(j = 0; j < chart[i].size(); j++) {
            if(i > 0 && j == 0)
                str += toA(chart[i][j]);
            else
                str += chart[i][j];

            if(j == 0) {
                if(str.length() < 8) {
                    str += '\t';
                }
                str += "|";
            }
            else
                str += " ";
        }

        lines.push_back(str);

        if(i == 0 || i == chart.size() - 1) {
            lines.push_back("--------|---------------------------");
        }
    }

    for(i = 0; i < lines.size(); i++) {
        outFile << lines[i] << std::endl;
        if(echo) std::cout << lines[i] << std::endl;
    }
}

std::vector<std::vector<std::string> > const minimizer::terms =   { { "A'B'", "A'B", "AB'", "AB" },
                                                                    { "A'B'C'", "A'B'C", "A'BC'", "A'BC",
                                                                      "AB'C'", "AB'C", "ABC'", "ABC" },
                                                                    { "A'B'C'D'", "A'B'C'D", "A'B'CD'", "A'B'CD",
                                                                      "A'BC'D'", "A'BC'D", "A'BCD'", "A'BCD",
                                                                      "AB'C'D'", "AB'C'D", "AB'CD'", "AB'CD",
                                                                      "ABC'D'", "ABC'D", "ABCD'", "ABCD" } };
std::vector<std::vector<std::string> > const minimizer::binterms ={ { "00", "01", "10", "11" },
                                                                    { "000", "001", "010", "011",
                                                                      "100", "101", "110", "111" },
                                                                    { "0000", "0001", "0010", "0011",
                                                                      "0100", "0101", "0110", "0111",
                                                                      "1000", "1001", "1010", "1011",
                                                                      "1100", "1101", "1110", "1111" } };
std::vector<std::string> const minimizer::F = {"F(A,B)","F(A,B,C)","F(A,B,C,D)"};
std::vector<std::string> const minimizer::D = {"D(A,B)","D(A,B,C)","D(A,B,C,D)"};
